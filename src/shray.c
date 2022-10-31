/* Distribution: 1d it is a block distribution on the bytes, so
 * phi_s(k) = k + s * roundUp(n, p), in the higher dimensional case,
 * we distribute blockwise along the first dimension. */

#include "../include/shray2/shray.h"
#include <assert.h>

/*****************************************************
 * Global variable declarations.
 *****************************************************/

static Cache cache;
static Heap heap;

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

static int inRange(void *address, int index)
{
    return ((uintptr_t)heap.allocs[index].location <= (uintptr_t)address) &&
        ((uintptr_t)address < (uintptr_t)heap.allocs[index].location + heap.allocs[index].size);
}

/* Simple binary search, assumes heap.allocs is sorted. Returns the index of the 
 * allocation segfault belongs to. */
static int findOwner(void *segfault)
{
    int low = 0;
    int high = heap.numberOfAllocs - 1;
    int middle = -1;

    while (low <= high) {
        middle = (low + high) / 2;
        uintptr_t location = (uintptr_t)heap.allocs[middle].location;
        if (location + heap.allocs[middle].size <= (uintptr_t)segfault) low = middle + 1;
        else if ((uintptr_t)segfault < location) high = middle - 1;
        else break;
    }

#ifdef DEBUG
    if (!inRange(segfault, middle)) {
        fprintf(stderr, "%p is not in an allocation.\n", segfault); 
        ShrayFinalize(1);
    }
#endif

    return middle;
}

static void SegvHandler(int sig, siginfo_t *si, void *unused)
{
    SEGFAULTCOUNT
    void *address = si->si_addr;
    void *roundedAddress = (void *)((uintptr_t)address / ShrayPagesz * ShrayPagesz);
    GRAPH_SEGV((uintptr_t)address, segfaultCounter)

    DBUG_PRINT("Segfault at %p.", address)

    int index = findOwner(roundedAddress);
    uintptr_t difference = (uintptr_t)roundedAddress - (uintptr_t)(heap.allocs[index].location);
    unsigned int owner = difference / heap.allocs[index].bytesPerBlock;

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

static void registerHandler(void)
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
static void UpdatePage(uintptr_t page, int index)
{
    DBUG_PRINT("Update page %p of allocation %p", (void *)page, heap.allocs[index].location);

    /* We have communication with a rank when allocStart <= page + ShrayPagesz - 1 and 
     * allocEnd - 1 >= page. Solving the inequalities yields the following loop. */
    uintptr_t location = (uintptr_t)heap.allocs[index].location;
    uintptr_t bytesPerBlock = heap.allocs[index].bytesPerBlock;

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

static void resetCache(void)
{
    size_t end = (cache.allUsed) ? cache.numberOfLines : cache.firstIn;
    for (size_t i = 0; i < end; i++) {
        MPROTECT_SAFE(cache.addresses[i], ShrayPagesz, PROT_WRITE);
    }
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

    heap.size = sizeof(Allocation);
    heap.numberOfAllocs = 0;
    MALLOC_SAFE(heap.allocs, sizeof(Allocation));

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
    void *location;

    /* For the segfault handler, we need the start of each allocation to be
     * ShrayPagesz-aligned. We cheat a little by making it possible for this to be multiple
     * system-pages. So we mmap an extra page at the start and end, and then move the
     * pointer up. */
    if (Shray_rank == 0) {
        void *mmapAddress;
        MMAP_SAFE(mmapAddress, mmap(NULL, totalSize + 2 * ShrayPagesz, 
                    PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));
        location = (void *)(roundUp((uintptr_t)mmapAddress, ShrayPagesz) * ShrayPagesz);
        DBUG_PRINT("mmapAddress = %p, allocation start = %p", mmapAddress, location);
    }

    /* Broadcast location to the other nodes. */
    gasnet_coll_broadcast(gasnete_coll_team_all, &location, 0, &location, 
            sizeof(void *), GASNET_COLL_DST_IN_SEGMENT);

    if (Shray_rank != 0) {
        MMAP_SAFE(location, mmap(location, totalSize + ShrayPagesz, PROT_NONE,
                MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0));
    }

    /* Insert allocation into the heap, making sure allocs stays sorted. */
    heap.numberOfAllocs++;
    if (heap.numberOfAllocs > heap.size / sizeof(Allocation)) {
        REALLOC_SAFE(heap.allocs, 2 * heap.size);
        heap.size *= 2;
    }
    int index = heap.numberOfAllocs;
    while ((uintptr_t)heap.allocs[index - 1].location > (uintptr_t)location && index > 0) {
        heap.allocs[index] = heap.allocs[index - 1];
        index--;
    }

    /* We distribute blockwise over the first dimension. */
    size_t bytesPerLatterDimensions = totalSize / firstDimension;
    size_t bytesPerBlock = roundUp(firstDimension, Shray_size) * bytesPerLatterDimensions;

    uintptr_t firstByte = ((uintptr_t)location + Shray_rank * bytesPerBlock);
    uintptr_t lastByte = (Shray_rank == Shray_size - 1) ? 
        (uintptr_t)location + totalSize - 1 : 
        (uintptr_t)location + (Shray_rank + 1) * bytesPerBlock - 1;

    /* First byte, last byte rounded down to page number. */
    uintptr_t firstPage = firstByte / ShrayPagesz * ShrayPagesz;
    uintptr_t lastPage = lastByte / ShrayPagesz * ShrayPagesz;

    size_t segmentSize = lastPage - firstPage + ShrayPagesz;

    DBUG_PRINT("Made a DSM allocation [%p, %p[, of which we own [%p, %p[.",
            location, (void *)((uintptr_t)location + totalSize),
            (void *)firstByte, (void *)(lastByte + 1));

    MPROTECT_SAFE((void *)firstPage, segmentSize, PROT_READ | PROT_WRITE);

    heap.allocs[index].location = location;
    heap.allocs[index].size = totalSize;
    heap.allocs[index].bytesPerBlock = bytesPerBlock;

    return location;
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

    int index = findOwner(array);
    size_t bytesPerBlock = heap.allocs[index].bytesPerBlock;
    uintptr_t location = (uintptr_t)heap.allocs[index].location;

    /* Synchronise in case the first or last page is co-owned with someone else. */
    uintptr_t firstByte = Shray_rank * bytesPerBlock;
    uintptr_t lastByte = (Shray_rank + 1) * bytesPerBlock - 1;
    uintptr_t firstPage = location + firstByte / ShrayPagesz * ShrayPagesz;
    uintptr_t lastPage = location + lastByte / ShrayPagesz * ShrayPagesz;

    UpdatePage(firstPage, index);
    if (firstPage != lastPage) UpdatePage(lastPage, index);
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

    int index = findOwner(address);
    heap.numberOfAllocs--;
    while ((unsigned)index < heap.numberOfAllocs) {
        heap.allocs[index] = heap.allocs[index + 1];
        index++;
    }
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
