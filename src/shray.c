/* Distribution: 1d it is a block distribution on the bytes, so
 * phi_s(k) = k + s * roundUp(n, p), in the higher dimensional case,
 * we distribute blockwise along the first dimension. */

#include "shray.h"
#include "../include/shray2/shray.h"
#include "bitmap.h"
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

/* Returns ceil(a / b) */
static inline uintptr_t roundUp(uintptr_t a, uintptr_t b)
{
    return (a + b - 1) / b;
}

static void gasnetBarrier(void)
{
    gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0, GASNET_BARRIERFLAG_ANONYMOUS);
}

static int inRange(void *address, int index)
{
    return ((uintptr_t)heap.allocs[index].location <= (uintptr_t)address) &&
        ((uintptr_t)address < (uintptr_t)heap.allocs[index].location + heap.allocs[index].size);
}

/* Simple binary search, assumes heap.allocs is sorted. Returns the index of the 
 * allocation segfault belongs to. */
static int findAlloc(void *segfault)
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

    if (!inRange(segfault, middle)) {
        DBUG_PRINT("%p is not in an allocation.\n", segfault); 
        ShrayFinalize(1);
    }

    return middle;
}

/* Evicts at least size bytes from the index'th allocation.
 * FIXME What would be a suitable algorithm here? */

// FIFO
//
// - When eviction is needed: go through the list starting at the front and delete
// until you have enough space: O(n) where n is the size of the cache
// - Insertion O(1)
// - Deletion O(1)
//
// problem: evict is called in the signal handler, can not do memory allocations
// solution: use a ringbuffer
//
// problem: need to update bookkeeping but cache contains no such information
// solution: each cache entry contains some additional data: pointer to
// allocation entry
//
// ----
// LFU:
// For LFU the issue is that we don't know how often a cache entry is used.
// memory accesses are implicit, we only notice segmentation faults.
//
// This is also the reason why any frequency-based algorithm will not work
// without some to way actually track access. We could allow users to provide
// this information but that (1) increases the burden on the user and (2) is
// hard to predict and get right.
//
// ----
// LRU:
// same issue as with LFU, we can't track proper accesses since no page fault
// occurs and it is implicitly handled by the system
static void evict(size_t size)
{
    return;
}

static void SegvHandler(int sig, siginfo_t *si, void *unused)
{
    SEGFAULTCOUNT
    void *address = si->si_addr;
    uintptr_t roundedAddress = (uintptr_t)address / ShrayPagesz * ShrayPagesz;

    Allocation *alloc = heap.allocs + findAlloc((void *)roundedAddress);

    size_t pageNumber = (roundedAddress - (uintptr_t)alloc->location) / ShrayPagesz;

    if (BitmapCheck(alloc->prefetched, pageNumber)) {
        Range range = BitmapSurrounding(alloc->prefetched, pageNumber);

        DBUG_PRINT("%p is currently being prefetched, along with [%p, %p[",
               address, (void *)((uintptr_t)alloc->location + range.start * ShrayPagesz), 
               (void *)((uintptr_t)alloc->location + range.end * ShrayPagesz));

        gasnet_wait_syncnbi_gets();

        MPROTECT_SAFE((void *)((uintptr_t)alloc->location + range.start * ShrayPagesz),
                    (range.end - range.start) * ShrayPagesz, PROT_READ);

        DBUG_PRINT("We set numbers [%zu, %zu[ to locally available, and to not waiting", 
                range.start, range.end);

        for (unsigned int i = 0; i < heap.numberOfAllocs; i++) {
            BitmapSetOnes(heap.allocs[i].local, range.start, range.end);
            BitmapSetZeroes(heap.allocs[i].prefetched, range.start, range.end);
        }
    } else {
        DBUG_PRINT("%p is not being prefetched, perform blocking fetch.", address);
        if (cache.usedMemory + ShrayPagesz > cache.maximumMemory) {
            DBUG_PRINT("We free up %zu bytes of cache memory", cache.maximumMemory / 10);
            evict(cache.maximumMemory / 10);
        }

        cache.usedMemory += ShrayPagesz;
        alloc->usedMemory += ShrayPagesz;

        uintptr_t difference = roundedAddress - (uintptr_t)alloc->location;
        unsigned int owner = difference / alloc->bytesPerBlock;

        DBUG_PRINT("Segfault is owned by node %d.", owner);

        MPROTECT_SAFE((void *)roundedAddress, ShrayPagesz, PROT_WRITE);

        gasnet_get((void *)roundedAddress, owner, (void *)roundedAddress, ShrayPagesz);

        MPROTECT_SAFE((void *)roundedAddress, ShrayPagesz, PROT_READ);

        DBUG_PRINT("We set pages [%zu, %zu[ to locally available.", 
                pageNumber, pageNumber + 1);

        BitmapSetOnes(alloc->local, pageNumber, pageNumber + 1);
    }

    return;
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

    /* We max it because the last part of the last page may not be owned by anyone. */
    unsigned int lastRank = min((page + ShrayPagesz - location - 1) / bytesPerBlock, 
            Shray_size - 1);
    unsigned int firstRank = roundUp(page - location - bytesPerBlock + 1, bytesPerBlock);

    DBUG_PRINT("UpdatePage: we communicate with nodes %d, ..., %d.", firstRank, lastRank);

    for (unsigned int rank = firstRank; rank <= lastRank; rank++) {

        if (rank == Shray_rank) continue;

        uintptr_t allocStart = location + rank * bytesPerBlock;
        uintptr_t allocEnd = location + (rank + 1) * bytesPerBlock;
        
        uintptr_t start = max(page, allocStart);
        uintptr_t end = min(page + ShrayPagesz, allocEnd);

        DBUG_PRINT("UpdatePage: we get [%p, %p[ from %d", (void *)start, (void *)end, rank);
        gasnet_get_nbi_bulk((void *)start, rank, (void *)start, end - start);
    }
}

/* Gets [start, end[ asynchronously and sets the protection to PROT_WRITE. Assumes 
 * start, end are page-aligned and not owned by our rank. */
static inline void helpPrefetch(uintptr_t start, uintptr_t end, Allocation *alloc)
{
    if (end <= start) return;

    uintptr_t location = (uintptr_t)alloc->location;
    size_t size = alloc->size;
    uintptr_t bytesPerBlock = alloc->bytesPerBlock;

    MPROTECT_SAFE((void *)start, end - start, PROT_WRITE);
    unsigned int firstOwner = (start - location) / bytesPerBlock;
    /* We take the min with Shay_size because the last part of the last page is not owned by 
     * anyone. */
    unsigned int lastOwner = min((end - 1 - location) / bytesPerBlock, Shray_size - 1);

    DBUG_PRINT("We set page numbers [%zu, %zu[ to prefetched.", 
            (start - location) / ShrayPagesz, (end - location) / ShrayPagesz);

    BitmapSetOnes(alloc->prefetched, 
            (start - location) / ShrayPagesz, (end - location) / ShrayPagesz);

    DBUG_PRINT("Prefetch [%p, %p[ from nodes %d, ..., %d", 
            (void *)start, (void *)end, firstOwner, lastOwner);
    assert(firstOwner > Shray_rank || lastOwner < Shray_rank);

    for (unsigned int rank = firstOwner; rank <= lastOwner; rank++) {
        uintptr_t theirStart = (location + rank * bytesPerBlock) / 
            ShrayPagesz * ShrayPagesz;
        uintptr_t theirEnd = (rank == Shray_size - 1) ? 
            roundUp(location + size, ShrayPagesz) * ShrayPagesz :
            roundUp(location + (rank + 1) * bytesPerBlock, ShrayPagesz) * ShrayPagesz;
        uintptr_t dest = max(start, theirStart);
        size_t nbytes = min(end, theirEnd) - max(start, theirStart);

        DBUG_PRINT("We prefetch [%p, %p[ from node %d", 
                (void *)dest, (void *)(dest + nbytes), rank);

        gasnet_get_nbi_bulk((void *)dest, rank, (void *)dest, nbytes);
    }
}

/* Resetting the protections is done by ShrayRealloc and ShraySync */
static void resetCache(Allocation *alloc)
{
    cache.usedMemory -= alloc->usedMemory;  
    alloc->usedMemory = 0;
    BitmapSetZeroes(alloc->local, 0, alloc->local->size);
    BitmapSetZeroes(alloc->prefetched, 0, alloc->prefetched->size);
    gasnet_wait_syncnbi_gets();
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
        cache.maximumMemory = atol(cacheSizeEnv);
    }

    cache.usedMemory = 0;

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
    int index = heap.numberOfAllocs - 1;
    while (index > 0 && (uintptr_t)heap.allocs[index - 1].location > (uintptr_t)location) {
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

    size_t totalPages = (roundUp((uintptr_t)location + totalSize, ShrayPagesz) * ShrayPagesz -
        (uintptr_t)location) / ShrayPagesz;

    heap.allocs[index].location = location;
    heap.allocs[index].size = totalSize;
    heap.allocs[index].bytesPerBlock = bytesPerBlock;
    heap.allocs[index].usedMemory = 0;
    heap.allocs[index].local = BitmapCreate(totalPages);
    heap.allocs[index].prefetched = BitmapCreate(totalPages);

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

    resetCache(heap.allocs + findAlloc(array));
}

void ShraySync(void *array)
{
    /* So the other processors have finished writing before we get their parts of the pages. */
    gasnetBarrier();
    BARRIERCOUNT

    int index = findAlloc(array);
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

    resetCache(heap.allocs + findAlloc(array));

    /* So no one reads from use before the communications are completed. */
    gasnetBarrier();
    BARRIERCOUNT
}

void ShrayFree(void *address)
{
    /* So everyone has finished reading before we free the array. */
    gasnetBarrier();
    BARRIERCOUNT

    int index = findAlloc(address);
    /* We leave potentially two pages mapped due to the alignment in ShrayMalloc, but 
     * who cares. */
    MUNMAP_SAFE(heap.allocs[index].location, heap.allocs[index].size);
    BitmapFree(heap.allocs[index].local);
    BitmapFree(heap.allocs[index].prefetched);
    heap.numberOfAllocs--;
    while ((unsigned)index < heap.numberOfAllocs) {
        heap.allocs[index] = heap.allocs[index + 1];
        index++;
    }
}

/* FIXME Not thread-safe. If we want to go that route, this should at the very least
 * only be called by the memory-thread. */
void ShrayPrefetch(void *address, size_t size)
{
    /* We make the maximal subarray [start, end[ of [address, address + size[ available,
     * such that start, end are rounded to page boundaries. 
     *
     * We only get pages we do not partially own to avoid needless communication, and to 
     * not mess up the protections of owned pages. 
     * 
     * If our node owns [ourStart, ourEnd[ (rounded to pages!), then we need to fetch 
     * [start, end[ \cap [ourStart, ourEnd[^c = 
     * [start, end[ \cap (]-\infty, ourStart[ \cup [ourEnd, \infty[) = 
     * ([start, end[ \cap ]-\infty, ourStart[) \cup ([start, end[ \cap [ourEnd, \infty[) = 
     * [start, min(end, ourStart)[ \cup [max(start, ourEnd), end[. 
     *
     * This union is disjoint as min(end, ourStart) <= ourStart < ourEnd <= max(start, ourEnd).
     */
    uintptr_t start = roundUp((uintptr_t)address, ShrayPagesz) * ShrayPagesz;
    uintptr_t end = ((uintptr_t)address + size) / ShrayPagesz * ShrayPagesz;

    Allocation *alloc = heap.allocs + findAlloc((void *)start);
    if (findAlloc((void *)start) != findAlloc((void *)(end - 1))) {
        DBUG_PRINT("ShrayGet [%p, %p[ is not within a single allocation.", (void *)start, 
                (void *)end);
    }

    uintptr_t location = (uintptr_t)alloc->location;
    size_t bytesPerBlock = alloc->bytesPerBlock; 
    uintptr_t ourStart = (location + Shray_rank * bytesPerBlock) / ShrayPagesz * ShrayPagesz;
    uintptr_t ourEnd = (Shray_rank == Shray_size - 1) ? 
        roundUp(location + alloc->size, ShrayPagesz) * ShrayPagesz  :
        location + roundUp((Shray_rank + 1) * bytesPerBlock, ShrayPagesz) * ShrayPagesz;

    DBUG_PRINT("Prefetch issued for [%p, %p[, we actually get [%p, %p[.",
            address, (void *)((uintptr_t)address + size), (void *)start, (void *)end);

    helpPrefetch(start, min(end, ourStart), alloc);
    helpPrefetch(max(start, ourEnd), end, alloc);
}

/* FIXME Not thread-safe. If we want to go that route, this should at the very least
 * only be called by the memory-thread. */
void ShrayDiscard(void *address, size_t size) 
{
    /* Round to maximal subset [actualStart, actualEnd[ \subseteq [address, address + size[
     * such that actualStart, actualEnd are page-aligned. */
    uintptr_t actualStart = roundUp((uintptr_t)address, ShrayPagesz) * ShrayPagesz;
    uintptr_t actualEnd = ((uintptr_t)address + size) / ShrayPagesz * ShrayPagesz;

    Allocation *alloc = heap.allocs + findAlloc(address);

    /* It would be strange to discard before we even have the data, but it is possible. */
    gasnet_wait_syncnbi_gets();

    MUNMAP_SAFE((void *)actualStart, actualEnd - actualStart);
    MMAP_SAFE(address, mmap((void *)actualStart, actualEnd - actualStart, PROT_NONE, 
                MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0));

    size_t firstPageNumber = (actualStart - (uintptr_t)alloc->location) / ShrayPagesz;
    size_t lastPageNumber = (actualEnd - (uintptr_t)alloc->location) / ShrayPagesz;

    BitmapSetZeroes(alloc->prefetched, firstPageNumber, lastPageNumber);
    BitmapSetZeroes(alloc->local, firstPageNumber, lastPageNumber);

    cache.usedMemory -= (lastPageNumber - firstPageNumber) * ShrayPagesz;
    alloc->usedMemory -= (lastPageNumber - firstPageNumber) * ShrayPagesz;
}

void ShrayReport(void)
{
    fprintf(stderr,
            "Shray report P(%d): %zu segfaults, %zu barriers, "
            "%zu bytes communicated.\n", Shray_rank, segfaultCounter, barrierCounter,
            segfaultCounter * ShrayPagesz);
}

unsigned int ShrayRank(void)
{
    return Shray_rank;
}

unsigned int ShraySize(void)
{
    return Shray_size;
}

void ShrayFinalize(int exit_code)
{
    gasnet_exit(exit_code);
}
