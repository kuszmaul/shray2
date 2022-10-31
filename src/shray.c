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

    while (!inRange(segfault, alloc)) {
        alloc = alloc->next;
#ifdef DEBUG
        if (alloc == NULL) {
            printf("[node %d]: Segfault (%p) outside of DSM area\n", Shray_rank, segfault);
            gasnet_exit(1);
        }
#endif
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

    cache.firstIn++;
    if (cache.firstIn == cache.numberOfLines) {
        cache.allUsed = true;
        cache.firstIn = 0;
    }
}

void registerHandler(void)
{
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset (&sa.sa_mask);
    sa.sa_sigaction = SegvHandler;

    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("Registering SIGSEGV handler failed.\n");
        gasnet_exit(1);
    }
}

/* Helper function for ShraySync, updates page with the calculated contents from 
 * other pages asynchronously. */
void UpdatePage(uintptr_t page, Allocation *alloc)
{
    DBUG_PRINT("Update page %p of allocation %p", (void *)page, alloc->location);

    /* We have communication with a rank when allocStart <= page + ShrayPagesz - 1 and 
     * allocEnd - 1 >= page. Solving the inequalities yields the following loop. */
    uintptr_t location = (uintptr_t)alloc->location;
    uintptr_t bytesPerBlock = alloc->bytesPerBlock;

    unsigned int lastRank = (page + ShrayPagesz - location - 1) / bytesPerBlock;
    unsigned int firstRank = roundUp(page - location - bytesPerBlock + 1, bytesPerBlock);

    DBUG_PRINT("UpdatePage: we communicate with nodes %d, ..., %d.", firstRank, lastRank);

    /* Extra condition because the last part of the last page may be write-owned by no-one. */
    for (unsigned int rank = firstRank; rank <= lastRank && rank < Shray_size; rank++) {

        if (rank == Shray_rank) continue;

        uintptr_t allocStart = location + rank * bytesPerBlock;
        uintptr_t allocEnd = location + (rank + 1) * bytesPerBlock;
        
        uintptr_t start = (allocStart < page) ? page : allocStart;
        uintptr_t end = (allocEnd > page + ShrayPagesz) ? page + ShrayPagesz : allocEnd;

        DBUG_PRINT("UpdatePage: we get [%p, %p[ from %d", (void *)start, (void *)end, rank);
        gasnet_get_nbi_bulk((void *)start, rank, (void *)start, end - start);
    }
}

void resetCache(void)
{
    size_t end = (cache.allUsed) ? cache.numberOfLines : cache.firstIn;
    for (size_t i = 0; i < end; i++) {
        MPROTECT_SAFE(cache.addresses[i], ShrayPagesz, PROT_WRITE);
    }
}

Allocation *createAllocation(void)
{
    Allocation *result;
    MALLOC_SAFE(result, sizeof(Allocation));
    result->next = NULL;

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

    heap = NULL;

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
    cache.allUsed = false;
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
            (void *)firstByte, (void *)(lastByte + 1));

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
    uintptr_t firstPage = (uintptr_t)alloc->location + 
        (uintptr_t)firstByte / ShrayPagesz * ShrayPagesz;
    uintptr_t lastPage = (uintptr_t)alloc->location + 
        (uintptr_t)lastByte / ShrayPagesz * ShrayPagesz;

    UpdatePage(firstPage, alloc);
    if (firstPage != lastPage) UpdatePage(lastPage, alloc);
    gasnet_wait_syncnbi_gets();
    DBUG_PRINT("We are done updating pages for %p", array);

    resetCache();

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
#ifdef DEBUG
        if (*indirect == NULL) {
            fprintf(stderr, "Illegal dsm free\n");
            gasnet_exit(1);
        }
#endif
        indirect = &(*indirect)->next;
    }

    munmap(address, (*indirect)->size);
    Allocation *deleteThis = *indirect;
    *indirect = (*indirect)->next;
    DBUG_PRINT("We free %p (should be %p)", deleteThis->location, address);
    free(deleteThis);
}

void ShrayReport(void)
{
    fprintf(stderr,
            "Shray report P(%d): %zu segfaults, %zu barriers, "
            "%zu bytes communicated.\n", Shray_rank, segfaultCounter, barrierCounter, 
            segfaultCounter * ShrayPagesz);
}

size_t ShrayRank(void)
{
    return Shray_rank;
}

size_t ShraySize(void)
{
    return Shray_size;
}

void ShrayFinalize(int exit_code)
{
    gasnet_exit(exit_code);
}
