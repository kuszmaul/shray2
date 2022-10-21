
/* Distribution: 1d it is a block distribution on the bytes, so
 * phi_s(k) = k + s * roundUp(n, p), in the higher dimensional case,
 * we distribute blockwise along the first dimension. */

#include "../include/shray2/shray.h"
#include <assert.h>

/*****************************************************
 * Global variable declarations.
 *****************************************************/

static Cache cache;
static Allocation *heap;

static unsigned int Shray_rank;
static unsigned int Shray_size;
static size_t segfaultCounter;
static size_t barrierCounter;
static size_t ShrayPagesz;
static size_t cacheLineSize;

/*****************************************************
 * Helper functions
 *****************************************************/

static inline size_t max(size_t x, size_t y)
{
    return x > y ? x : y;
}

static inline size_t min(size_t x, size_t y)
{
    return x < y ? x : y;
}

static void gasnetBarrier(void)
{
    gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0, GASNET_BARRIERFLAG_ANONYMOUS);
}

/* Returns ceil(a / b) */
static inline size_t roundUp(size_t a, size_t b)
{
    return (a + b - 1) / b;
}

int inRange(void *address, Allocation* alloc)
{
    return ((uintptr_t)alloc->location <= (uintptr_t)address) &&
        ((uintptr_t)address < (uintptr_t)alloc->location + alloc->size);
}

Allocation *findOwner(void *segfault)
{
    /* We advance through the allocations, until we find the one containing our
     * segfault. */
    Allocation *alloc = heap;

    while (alloc != NULL && !inRange(segfault, alloc)) {
        alloc = alloc->next;
    }

    if (alloc == NULL) {
            printf("[node %d]: Segfault (%p) outside of DSM area\n", Shray_rank, segfault);
            gasnet_exit(1);
    }

    return alloc;
}

void SegvHandler(int sig, siginfo_t *si, void *unused)
{
    SEGFAULTCOUNT
    void *address = si->si_addr;
    void *roundedAddress = (void *)((uintptr_t)address / ShrayPagesz * ShrayPagesz);
    GRAPH_SEGV((uintptr_t)address, segfaultCounter)

    DBUG_PRINT("Segfault at %p.", address)

    Allocation *alloc = findOwner(roundedAddress);
    uintptr_t difference = (uintptr_t)roundedAddress - (uintptr_t)(alloc->location);
    unsigned int owner = difference / alloc->bytesPerBlock;

    DBUG_PRINT("Segfault is owned by node %d.", owner);

    /* Fetch the remote page and store it in the cache line we are going to evict. */
    gasnet_get(cache.addresses[cache.firstIn], owner, roundedAddress, ShrayPagesz);
    /* Set protections to read in case that was undone by cacheReset. */
    MPROTECT_SAFE(cache.addresses[cache.firstIn], ShrayPagesz, PROT_READ | PROT_WRITE);

    /* Remap the line to the proper position. */
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

void resetCache()
{
    for (size_t i = 0; i < cache.numberOfLines; i++) {
        MPROTECT_SAFE(cache.addresses[i], ShrayPagesz, PROT_WRITE);
    }
}

Allocation *createAllocation(void)
{
    Allocation *result;
    MALLOC_SAFE(result, sizeof(Allocation));

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

bool ShrayOutput;

void ShrayInit(int *argc, char ***argv)
{
    GASNET_SAFE(gasnet_init(argc, argv));
    /* Must be built with GASNET_SEGMENT_EVERYTHING, so these arguments are ignored. */
    GASNET_SAFE(gasnet_attach(NULL, 0, 4096, 0));

    Shray_size = gasnet_nodes();
    Shray_rank = gasnet_mynode();

    ShrayOutput = (Shray_rank == 0);

    segfaultCounter = 0;
    barrierCounter = 0;

    char *cacheLineEnv = getenv("SHRAY_CACHELINE");
    if (cacheLineEnv == NULL) {
        fprintf(stderr, "Please set the cacheline environment variable SHRAY_CACHELINE\n");
        gasnet_exit(1);
    } else {
        cacheLineSize = atol(cacheLineEnv);
    }

    int pagesz = sysconf(_SC_PAGE_SIZE);
    if (pagesz == -1) {
        perror("Querying system page size failed.");
    }

    ShrayPagesz = (size_t)pagesz * cacheLineSize;

    heap = createAllocation();

    char *cacheSizeEnv = getenv("SHRAY_CACHESIZE");
    if (cacheSizeEnv == NULL) {
        fprintf(stderr, "Please set the cache size environment variable SHRAY_CACHESIZE\n");
        gasnet_exit(1);
    } else {
        cache.numberOfLines = atol(cacheSizeEnv) / ShrayPagesz;
    }

    if (cache.numberOfLines < cacheLineSize) {
        fprintf(stderr, "You set the cacheline size (SHRAY_CACHELINE) larger than the "
                        "cache size (SHRAY_CACHESIZE)\n");
        gasnet_exit(1);
    }

    cache.firstIn = 0;
    MALLOC_SAFE(cache.addresses, cache.numberOfLines * sizeof(void *));

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

    /* For the segfault handler, we need the start of each allocation to be 
     * ShrayPagesz-aligned. We cheat a little by making it possible for this to be multiple 
     * system-pages. So we mmap an extra page at the start and end, and then move the 
     * pointer up. */
    if (Shray_rank == 0) {
        void *mmapAddress;
        MMAP_SAFE(mmapAddress, mmap(NULL, alloc->size + 2 * ShrayPagesz, 
                    PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));
        alloc->location = (void *)(roundUp((uintptr_t)mmapAddress, ShrayPagesz) * ShrayPagesz);
        DBUG_PRINT("mmapAddress = %p, allocation start = %p", mmapAddress, alloc->location);
    }

    if (alloc->size / ShrayPagesz < Shray_size) {
        fprintf(stderr, "Your allocation is less than a page per node. This makes "
                "no sense, allocate it redundantly on each node.\n");
        gasnet_exit(1);
    }

    /* Broadcast alloc->location to the other nodes. */
    gasnet_coll_broadcast(gasnete_coll_team_all, &(alloc->location),
            0, &(alloc->location), sizeof(void *), GASNET_COLL_DST_IN_SEGMENT);

    if (Shray_rank != 0) {
        MMAP_SAFE(alloc->location, mmap(alloc->location, alloc->size + ShrayPagesz, PROT_NONE,
                MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0));
    }

    /* We distribute blockwise over the first dimension. */
    size_t bytesPerLatterDimensions = alloc->size / firstDimension;
    alloc->bytesPerBlock = roundUp(firstDimension, Shray_size) * bytesPerLatterDimensions;

    uintptr_t firstByte = ((uintptr_t)alloc->location + Shray_rank * alloc->bytesPerBlock);
    uintptr_t lastByte = (Shray_rank == Shray_size - 1) ? 
        (uintptr_t)alloc->location + alloc->size - 1 : 
        (uintptr_t)alloc->location + (Shray_rank + 1) * alloc->bytesPerBlock - 1;

    /* First byte, last byte rounded down to page number. */
    uintptr_t firstPage = firstByte / ShrayPagesz * ShrayPagesz;
    uintptr_t lastPage = lastByte / ShrayPagesz * ShrayPagesz;

    size_t segmentSize = lastPage - firstPage + ShrayPagesz;

    DBUG_PRINT("Made a DSM allocation [%p, %p[, of which we own [%p, %p[.",
            alloc->location, (void *)((uintptr_t)alloc->location + alloc->size),
            (void *)firstPage, (void *)(firstPage + segmentSize));

    MPROTECT_SAFE((void *)firstPage, segmentSize, PROT_READ | PROT_WRITE);

    /* Insert a new allocation */
    heap = insertAtHead(heap, alloc);

    return alloc->location;
}

size_t ShrayStart(size_t firstDimension)
{
    return Shray_rank * roundUp(firstDimension, Shray_size);
}

size_t ShrayStartOffset(size_t firstDimension, size_t offset)
{
    return max(ShrayStart(firstDimension), offset);
}

size_t ShrayEnd(size_t firstDimension)
{
    return (Shray_rank == Shray_size - 1) ? firstDimension :
        (Shray_rank + 1) * roundUp(firstDimension, Shray_size);
}

size_t ShrayEndOffset(size_t firstDimension, size_t offset)
{
    return min(ShrayEnd(firstDimension), offset);
}

void ShrayRealloc(void *array)
{
    /* Make sure every node has finished reading. */
    gasnetBarrier();
    BARRIERCOUNT;

    resetCache();
}

void ShraySync(void *array)
{
    /* So the other processors have finished writing before we get their parts of the pages. */
    gasnetBarrier();
    BARRIERCOUNT

    Allocation *alloc = findOwner(array);

    /* Synchronise in case the first or last page is co-owned with someone else. */
    size_t firstByte = Shray_rank * alloc->bytesPerBlock;
    size_t lastByte = (Shray_rank + 1) * alloc->bytesPerBlock - 1;

    /* Get the last page of the previous rank, and copy its contents to our copy of that
     * page. */
    if ((firstByte % ShrayPagesz != 0) && (Shray_rank != 0)) {

        void *pageBoundary = (void *)((uintptr_t)alloc->location +
                (uintptr_t)firstByte / ShrayPagesz * ShrayPagesz);

        size_t count = (uintptr_t)alloc->location + firstByte - (uintptr_t)pageBoundary;

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

        void *dest = (void *)((uintptr_t)alloc->location + lastByte + 1);

        size_t count = roundUp((uintptr_t)lastByte, ShrayPagesz) * ShrayPagesz -
            (uintptr_t)lastByte - 1;

        DBUG_PRINT("We are going to get [%p, %p[ from node %d\n", dest,
                (void *)((uintptr_t)dest + count), Shray_rank + 1);

        gasnet_get_bulk(dest, Shray_rank + 1, dest, count);

        DBUG_PRINT("We got [%p, %p[ from node %d\n", dest,
                (void *)((uintptr_t)dest + count), Shray_rank + 1);
    }

    resetCache(alloc);

    /* So no one reads from use before the communications are completed. */
    gasnetBarrier();
    BARRIERCOUNT
}

void ShrayFree(void *address)
{
    /* So everyone has finished reading before we free the array. */
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
}

void ShrayReport(void)
{
    fprintf(stderr,
            "Shray report P(%d): %zu segfaults, %zu barriers, "
            "%zu bytes communicated.\n", Shray_rank, segfaultCounter, barrierCounter, 
            segfaultCounter * ShrayPagesz);
}

void ShrayFinalize(int exit_code)
{
    gasnet_exit(exit_code);
}
