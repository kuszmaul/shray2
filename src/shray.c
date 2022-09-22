/* Distribution: 1d it is a block distribution on the bytes, so 
 * phi_s(k) = k + s * roundUp(n, p), in the higher dimensional case, 
 * we distribute blockwise along the first dimension. */

#include "../include/shray.h"
#include <assert.h>

/*****************************************************
 * Global variable declarations. 
 *****************************************************/

static Cache cache;
static Allocation *heap;

static int Shray_rank;
static int Shray_size;
static size_t segfaultCounter;
static size_t barrierCounter;
static size_t ShrayPagesz;

/*****************************************************
 * Helper functions
 *****************************************************/

static void gasnetBarrier(void)
{
    gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0, GASNET_BARRIERFLAG_ANONYMOUS);
}

/* Returns ceil(a / b) */
inline size_t roundUp(size_t a, size_t b)
{
    return (a + b - 1) / b;
}

int inRange(void *address, Allocation* alloc)
{
    return ((uintptr_t)alloc->location <= (uintptr_t)address) && 
        ((uintptr_t)address < (uintptr_t)alloc->location + alloc->size);
}

typedef struct {
    /* Owner of this page */
    int thisOwner;
    /* Owner of the next page, or -1 if it is not sensible to prefetch. */
    int nextOwner;
} Owner;

/* segfault is rounded to page boundary */
Owner findOwner(void *segfault)
{
    Owner owner;

    /* We advance through the allocations, until we find the one containing our 
     * segfault. */
    Allocation *current = heap;

    while (current != NULL && !inRange(segfault, current)) {
        current = current->next;
    }

    if (current == NULL) {
            printf("Segfault (%p) outside of DSM area\n", segfault);
            gasnet_exit(1);
    }

    uintptr_t difference = (uintptr_t)segfault - (uintptr_t)(current->location);

    owner.thisOwner = difference / current->bytesPerBlock;
    void *nextPage = (void *)((uintptr_t)segfault + ShrayPagesz);
    if (inRange(nextPage, current)) {
        owner.nextOwner = (difference + ShrayPagesz) / current->bytesPerBlock;
    } else {
        owner.nextOwner = -1;
    }

    return owner;
}

void SegvHandler(int sig, siginfo_t *si, void *unused)
{
    SEGFAULTCOUNT
    void *address = si->si_addr;
    void *roundedAddress = (void *)((uintptr_t)address / ShrayPagesz * ShrayPagesz);
    DBUG_PRINT("Segfault at %p.", address)

    gasnet_wait_syncnbi_gets();

    Owner owner = findOwner(roundedAddress);
    DBUG_PRINT("Segfault is owned by node %d. Next page is owned by node %d.",
            owner.thisOwner, owner.nextOwner);

    /* The prefetch of the previous segfault. */
    void *prefetchOld = cache.prefetch;

    /* Prefetch to the second cache line admitted. Protect the line afterwards as its 
     * contents is undefined until the asynchronous fetch is completed. */
    cache.prefetch = (void *)((uintptr_t)roundedAddress + ShrayPagesz);
    if (owner.nextOwner != -1) {
        DBUG_PRINT("We prefetch %p, storing it in %p.", cache.prefetch, 
                cache.addresses[cache.firstIn + 1]);
        gasnet_get_nbi(cache.addresses[cache.firstIn + 1], owner.nextOwner, 
                cache.prefetch, ShrayPagesz);
    } else {
        DBUG_PRINT("We do not prefetch %p as it falls outside of a DSM allocation.", 
                cache.prefetch);
    }
    MPROTECT_SAFE(mprotect(cache.addresses[cache.firstIn + 1], ShrayPagesz, PROT_NONE));

    if (prefetchOld != roundedAddress) {
        /* We prefetched the wrong page. Do a blocking fetch on the page we need, storing
         * the result in the line we evict. */
        DBUG_PRINT("We prefetched %p which we do not need, so now we fetch %p, "
                "storing it in %p.", prefetchOld, roundedAddress, 
                cache.addresses[cache.firstIn]);
        gasnet_get(cache.addresses[cache.firstIn], owner.thisOwner, roundedAddress, ShrayPagesz);
    }

    /* Remap the evicted cache line to the proper position. This also protects the 
     * previous position of the evicted cache line. FIXME right?*/
    DBUG_PRINT("We remap %p to %p", cache.addresses[cache.firstIn], roundedAddress);
    MREMAP_SAFE(cache.addresses[cache.firstIn], mremap(cache.addresses[cache.firstIn], 
            ShrayPagesz, ShrayPagesz, MREMAP_MAYMOVE | MREMAP_FIXED, 
            roundedAddress));

    cache.firstIn = (cache.firstIn + 1) % cache.numberOfLines;
}

void registerHandler(void)
{
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset (&sa.sa_mask);
    sa.sa_sigaction = SegvHandler;

    if (sigaction (SIGSEGV, &sa, NULL) == -1) {
        perror("Registering SIGSEGV handler failed.\n");
        gasnet_exit(1);
    }
}

Allocation *createAllocation(void)
{
    Allocation *result = malloc(sizeof(Allocation));

    return result;
}

Allocation *insertAtHead(Allocation *head, Allocation *newHead)
{
    newHead->next = head;
    return newHead;
}

/*****************************************************
 * Shray functionality
 *****************************************************/

void ShrayInit(int *argc, char ***argv, size_t cacheSize)
{
    GASNET_SAFE(gasnet_init(argc, argv));
    /* Must be built with GASNET_SEGMENT_EVERYTHING, so these arguments are ignored. */
    GASNET_SAFE(gasnet_attach(NULL, 0, 4096, 0));

    Shray_size = gasnet_nodes();
    Shray_rank = gasnet_mynode();

    segfaultCounter = 0;
    barrierCounter = 0;

    int pagesz = sysconf(_SC_PAGE_SIZE);
    if (pagesz == -1) {
        perror("Querying system page size failed.");
    }
 
    ShrayPagesz = (size_t)pagesz;

    heap = createAllocation();

    cache.firstIn = 0;
    cache.numberOfLines = cacheSize / ShrayPagesz;
    cache.addresses = malloc(cache.numberOfLines * sizeof(void *));
    cache.prefetch = NULL;

    if (cache.addresses == NULL) {
        perror("Allocating cache addresses has failed\n");
        gasnet_exit(1);
    }

    MMAP_SAFE(cache.addresses[0], mmap(NULL, cache.numberOfLines * ShrayPagesz, 
                PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));

    for (size_t i = 1; i < cache.numberOfLines; i++) {
        cache.addresses[i] = (void *)((uintptr_t)cache.addresses[i - 1] + ShrayPagesz);
    }

    DBUG_PRINT("We allocated %zu pages of cache starting at %p.", cache.numberOfLines,
            cache.addresses[0]);

    registerHandler();
}

void *ShrayMalloc(size_t firstDimension, size_t totalSize)
{
    Allocation *alloc = createAllocation();

    alloc->size = totalSize;

    if (Shray_rank == 0) {
        MMAP_SAFE(alloc->location, mmap(NULL, alloc->size, PROT_NONE, MAP_ANONYMOUS |
                MAP_PRIVATE, -1, 0));
    }

    /* Broadcast alloc->location to the other nodes. I don't know whether this is 
     * correct. Especially the 0 was a pure guess. Appears to work for now though. */
    gasnet_coll_broadcast(gasnete_coll_team_all, &(alloc->location),
            0, &(alloc->location), sizeof(void *), GASNET_COLL_DST_IN_SEGMENT);

    if (Shray_rank != 0) {
        MMAP_SAFE(alloc->location, mmap(alloc->location, alloc->size, PROT_NONE, 
                MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));
    }

    /* We distribute blockwise over the first dimension. */
    size_t bytesPerLatterDimensions = alloc->size / firstDimension;
    alloc->bytesPerBlock = roundUp(firstDimension, Shray_size) * bytesPerLatterDimensions;

    /* Protect all pages that we do not own, and register what we do. */
    size_t firstPage = Shray_rank * alloc->bytesPerBlock / ShrayPagesz;
    size_t lastPage = (Shray_rank == Shray_size - 1) ? alloc->size / ShrayPagesz :
        (Shray_rank + 1) * alloc->bytesPerBlock / ShrayPagesz;
    size_t segmentSize = (lastPage - firstPage + 1) * ShrayPagesz;
    if (segmentSize == 0) {
        fprintf(stderr, "A processor has no data\n");
        gasnet_exit(1);
    }

    void *start = (void *)((uintptr_t)alloc->location + firstPage * ShrayPagesz);
    DBUG_PRINT("ShrayMalloc makes %zu bytes from %p available for read/write.", 
            segmentSize, start);
    MPROTECT_SAFE(mprotect(start, segmentSize, PROT_READ | PROT_WRITE));

    /* Insert a new allocation */
    heap = insertAtHead(heap, alloc);

    gasnetBarrier();

    DBUG_PRINT("Made a DSM allocation [%p, %p[, of which we own [%p, %p[.", 
            alloc->location, (void *)((uintptr_t)alloc->location + alloc->size),
            start, (void *)((uintptr_t)start + segmentSize));

    BARRIERCOUNT

    return alloc->location;
}

size_t ShrayStart(size_t firstDimension)
{
    return Shray_rank * roundUp(firstDimension, Shray_size);
}

size_t ShrayEnd(size_t firstDimension)
{
    return (Shray_rank == Shray_size - 1) ? firstDimension :
        (Shray_rank + 1) * roundUp(firstDimension, Shray_size);
}

void ShraySync(void *array)
{    
    gasnetBarrier();
    BARRIERCOUNT

    Allocation *current = heap;

    while (current != NULL && current->location != array) {
        current = current->next;
    }

    if (current == NULL) {
        perror("address is not the start of a DSM allocation.");
    }

    /* Synchronise in case the first or last page is co-owned with someone else. */
    size_t firstByte = Shray_rank * current->bytesPerBlock;
    size_t lastByte = (Shray_rank + 1) * current->bytesPerBlock - 1;

    /* Get the last page of the previous rank, and copy its contents to our copy of that 
     * page. */
    if ((firstByte % ShrayPagesz != 0) && (Shray_rank != 0)) {

        void *pageBoundary = (void *)((uintptr_t)current->location +
                (uintptr_t)firstByte / ShrayPagesz * ShrayPagesz);

        size_t count = (uintptr_t)current->location + firstByte - (uintptr_t)pageBoundary;

        DBUG_PRINT("We are going to get [%p, %p[ from node %d\n", pageBoundary, 
                (void *)((uintptr_t)pageBoundary + count), Shray_rank - 1);

        gasnet_get_bulk(pageBoundary, Shray_rank - 1, pageBoundary, count);

        DBUG_PRINT("We got [%p, %p[ from node %d\n", pageBoundary, 
                (void *)((uintptr_t)pageBoundary + count), Shray_rank - 1);
    }

    /* Get the first page of the next rank, and copy its contents to our copy of that 
     * page. */
    if ((lastByte % ShrayPagesz != ShrayPagesz - 1) && 
            (Shray_rank != Shray_size - 1)) {

        void *dest = (void *)((uintptr_t)current->location + lastByte + 1);

        size_t count = roundUp((uintptr_t)lastByte, ShrayPagesz) * ShrayPagesz - 
            (uintptr_t)lastByte - 1;

        DBUG_PRINT("We are going to get [%p, %p[ from node %d\n", dest, 
                (void *)((uintptr_t)dest + count), Shray_rank + 1);

        gasnet_get_bulk(dest, Shray_rank + 1, dest, count);

        DBUG_PRINT("We got [%p, %p[ from node %d\n", dest, 
                (void *)((uintptr_t)dest + count), Shray_rank + 1);
    }

    BARRIERCOUNT
}

void ShrayFree(void *address)
{
    gasnetBarrier();
    BARRIERCOUNT

    /* indirect iterates through the links of the nodes, e.g. the pointers
     * to the next allocation. Double pointer is necessary because there is no 
     * link to heap (the head of the list). */
    Allocation **indirect = &heap; 

    while ((*indirect)->location != address) {
        indirect = &(*indirect)->next;
    }

    if (*indirect == NULL) {
        fprintf(stderr, "Illegal dsm free\n");
        gasnet_exit(1);
    } else {
        munmap(address, (*indirect)->size);
        Allocation *deleteThis = *indirect;
        *indirect = (*indirect)->next;
        free(deleteThis);
    }

    gasnetBarrier();
    BARRIERCOUNT
}

void ShrayReport(void)
{
    fprintf(stderr, 
            "Shray report P(%d): %zu segfaults, %zu barriers, %zu bytes communicated.\n",
            Shray_rank, segfaultCounter, barrierCounter, segfaultCounter * ShrayPagesz);
}

void ShrayFinalize(void)
{
    gasnet_exit(0);
}
