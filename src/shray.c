/* Distribution: 1d it is a block distribution on the bytes, so
 * phi_s(k) = k + s * roundUp(n, p), in the higher dimensional case,
 * we distribute blockwise along the first dimension. */

#include "shray.h"
#include "bitmap.h"
#include "queue.h"
#include "ringbuffer.h"
#include <assert.h>
#include <stdint.h>

/*****************************************************
 * Global variable declarations.
 *****************************************************/

/* TODO: move cache stuff to its own file */
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

static void *toShadow(void *addr, Allocation *alloc)
{
    return (void *)((uintptr_t)addr - (uintptr_t)alloc->location + (uintptr_t)alloc->shadow);
}

static uintptr_t roundUpPage(uintptr_t addr)
{
    return roundUp(addr, ShrayPagesz) * ShrayPagesz;
}

static uintptr_t roundDownPage(uintptr_t addr)
{
    return addr / ShrayPagesz * ShrayPagesz;
}

/* Frees [start, end[. start, end need to be ShrayPagesz-aligned */
static inline void freeRAM(uintptr_t start, uintptr_t end)
{
    if (start + ShrayPagesz >= end) return;

    void *dummy;
    MUNMAP_SAFE((void *)start, end - start);
    MMAP_SAFE(dummy, mmap((void *)start, end - start, PROT_NONE,
                MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0));
}

/* Sets [start, end[ to zero in the prefetch, local bitmaps, and decreases the used
 * memory by the appropiate amount. */
static void discardBitmap(uintptr_t start, uintptr_t end, Allocation *alloc)
{
    size_t firstPageNumber = (start - (uintptr_t)alloc->location) / ShrayPagesz;
    size_t lastPageNumber = (end - (uintptr_t)alloc->location) / ShrayPagesz;

    BitmapSetZeroes(alloc->prefetched, firstPageNumber, lastPageNumber);
    BitmapSetZeroes(alloc->local, firstPageNumber, lastPageNumber);

    cache.usedMemory -= (lastPageNumber - firstPageNumber) * ShrayPagesz;
    alloc->usedMemory -= (lastPageNumber - firstPageNumber) * ShrayPagesz;
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

static PrefetchStruct GetPrefetchStruct(void *address, size_t size)
{
    /* We get the minimal page-aligned superset [start, end[ of [address, size[,
     * except for the stuff we already have.
     *
     * If our node owns [ourStart, ourEnd[ (rounded to pages!), then we need to fetch
     * [start, end[ \cap [ourStart, ourEnd[^c =
     * [start, end[ \cap (]-\infty, ourStart[ \cup [ourEnd, \infty[) =
     * ([start, end[ \cap ]-\infty, ourStart[) \cup ([start, end[ \cap [ourEnd, \infty[) =
     * [start, min(end, ourStart)[ \cup [max(start, ourEnd), end[.
     *
     * This union is disjoint as min(end, ourStart) <= ourStart < ourEnd <= max(start, ourEnd).
     */
    uintptr_t start = roundDownPage((uintptr_t)address);
    uintptr_t end = roundUpPage((uintptr_t)address + size);

    Allocation *alloc = heap.allocs + findAlloc((void *)start);

    if (findAlloc((void *)start) != findAlloc((void *)(end - ShrayPagesz))) {
        DBUG_PRINT("ShrayGet [%p, %p[ is not within a single allocation.", (void *)start,
                (void *)end);
    }

    uintptr_t location = (uintptr_t)alloc->location;
    size_t bytesPerBlock = alloc->bytesPerBlock;
    uintptr_t ourStart = roundDownPage(location + Shray_rank * bytesPerBlock);
    uintptr_t ourEnd = (Shray_rank == Shray_size - 1) ?
        roundUpPage(location + alloc->size) :
        roundUpPage(location + (Shray_rank + 1) * bytesPerBlock);

    PrefetchStruct result;

    result.start1 = start;
    result.end1 = min(end, ourStart);
    result.start2 = max(start, ourEnd);
    result.end2 = end;
    result.alloc = alloc;

    return result;
}

/**
 * Reset the pages used by the cache.
 * Assumes start is page aligned.
 */
static void evictCacheEntry(Allocation *alloc, void *start, size_t pages)
{
    void *tmp;
    size_t index = ((uintptr_t)start - (uintptr_t)alloc->location) / ShrayPagesz;
    size_t size = pages * ShrayPagesz;
    BitmapSetZeroes(alloc->local, index, index + pages);

    MUNMAP_SAFE(start, size);
    MMAP_SAFE(tmp, mmap(start, size, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));
}

/* Assumes both pages are page-aligned. */
static inline int isNextPage(void *x, void *y)
{
    return (uintptr_t)y - (uintptr_t)x == ShrayPagesz;
}

/*
 * Evicts at least size bytes from the cache.
 */
static void evict(size_t size)
{
    cache_entry_t *next, *entry;
    size_t evicted = 0;
    size_t chain = 1;
    size_t size_pages = (size + ShrayPagesz - 1) / ShrayPagesz;

    /* First try to evict automatically retrieved pages. */
    while (!ringbuffer_empty(cache.autoCaches)) {
        chain = 1;
        entry = ringbuffer_front(cache.autoCaches);
        ringbuffer_del(cache.autoCaches);

        /* Check how many we can evict in one go. */
        while (!ringbuffer_empty(cache.autoCaches)) {
            if (chain >= size_pages) {
                break;
            }

            next = ringbuffer_front(cache.autoCaches);
            if (!isNextPage(entry->alloc, next->alloc)) {
                break;
            }

            ringbuffer_del(cache.autoCaches);
            ++chain;
        }

        evictCacheEntry(entry->alloc, entry->start, chain);
        /* Only evict as much as needed */
        evicted += chain;
        if (evicted >= size_pages) {
            return;
        }
    }

    /* Next, try to go through the prefetches in FIFO order to evict. */
    while (!queue_empty(cache.prefetchCaches)) {
        queue_entry_t q_entry = queue_dequeue(cache.prefetchCaches);
        chain = (size + ShrayPagesz - 1) / ShrayPagesz;
        evictCacheEntry(q_entry.alloc, q_entry.start, chain);
        evicted += chain;
        if (evicted >= size_pages) {
            return;
        }
    }

    if (evicted < size_pages) {
        DBUG_PRINT("Was only able to evict %zu pages (requested %zu)",
                evicted, size);
    }
}

static void SegvHandler(int sig, siginfo_t *si, void *unused)
{
    SEGFAULTCOUNT
    void *address = si->si_addr;
    uintptr_t roundedAddress = roundDownPage((uintptr_t)address);

    DBUG_PRINT("Segfault %p", address);

    Allocation *alloc = heap.allocs + findAlloc((void *)roundedAddress);

    size_t pageNumber = (roundedAddress - (uintptr_t)alloc->location) / ShrayPagesz;

    /* FIXME this waits for all prefetches, which we do not want to do as we will have a
     * prefetch 1 -> compute 0 -> prefetch 2 -> compute 1 kind of pattern in most applications,
     * so when we do compute n we wait for prefetch n, but at that point we have already
     * issued prefetch n + 1 just before. */
    if (BitmapCheck(alloc->prefetched, pageNumber)) {
        Range range = BitmapSurrounding(alloc->prefetched, pageNumber);

        DBUG_PRINT("%p is currently being prefetched, along with [%p, %p[",
               address, (void *)((uintptr_t)alloc->location + range.start * ShrayPagesz),
               (void *)((uintptr_t)alloc->location + range.end * ShrayPagesz));

        gasnet_wait_syncnbi_gets();

        void *start = (void *)((uintptr_t)alloc->location + range.start * ShrayPagesz);
        uintptr_t length = (range.end - range.start) * ShrayPagesz;

        DBUG_PRINT("We bring pages [%zu, %zu[ ([%p, %p[) into the light",
                range.start, range.end, start, (void *)((uintptr_t)start + length));

        MREMAP_MOVE(start, toShadow(start, alloc), length);
        /* We immediately remap the shadow part to not leave holes in our allocation.
         * Because of lazy allocation, this does not increase the memory footprint.
         * Using MREMAP_DONTUNMAP would. */
        void *dummy;
        MMAP_SAFE(dummy, mmap(toShadow(start, alloc), length, PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));

        BitmapSetZeroes(alloc->prefetched, range.start, range.end);
        BitmapSetZeroes(alloc->local, range.start, range.end);
    } else {
        DBUG_PRINT("%p is not being prefetched, perform blocking fetch.", address);
        if (cache.usedMemory + ShrayPagesz > cache.maximumMemory) {
            DBUG_PRINT("We free up %zu bytes of cache memory", cache.maximumMemory / 10);
            /*
             * TODO: Need to do a sync here as well, if the cache is full
             * and we need to evict we might need to evict prefetched data. In
             * that case we would need to do the same thing as in the if case
             * above.
             */
            evict(cache.maximumMemory / 10);
        }

        cache.usedMemory += ShrayPagesz;
        alloc->usedMemory += ShrayPagesz;

        uintptr_t difference = roundedAddress - (uintptr_t)alloc->location;
        unsigned int owner = difference / alloc->bytesPerBlock;

        DBUG_PRINT("Segfault is owned by node %d.", owner);

        MPROTECT_SAFE((void *)roundedAddress, ShrayPagesz, PROT_READ | PROT_WRITE);

        gasnet_get((void *)roundedAddress, owner, (void *)roundedAddress, ShrayPagesz);

        DBUG_PRINT("We set pages [%zu, %zu[ to locally available.",
                pageNumber, pageNumber + 1);

        BitmapSetOnes(alloc->local, pageNumber, pageNumber + 1);
        ringbuffer_add(cache.autoCaches, alloc, (void*)roundedAddress);
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
        /* TODO: does this need to be added to cache? */
        gasnet_get_nbi_bulk((void *)start, rank, (void *)start, end - start);
    }
}

/* Gets [start, end[ asynchronously to the allocation shadow
 * start, end are page-aligned and not owned by our rank. */
static inline void helpPrefetch(uintptr_t start, uintptr_t end, Allocation *alloc)
{
    if (end <= start) return;

    uintptr_t location = (uintptr_t)alloc->location;
    size_t size = alloc->size;
    uintptr_t bytesPerBlock = alloc->bytesPerBlock;

    unsigned int firstOwner = (start - location) / bytesPerBlock;
    /* We take the min with Shay_size because the last part of the last page is not owned by
     * anyone. */
    unsigned int lastOwner = min((end - 1 - location) / bytesPerBlock, Shray_size - 1);

    DBUG_PRINT("We set page numbers [%zu, %zu[ ([%p, %p[) to prefetched.",
            (start - location) / ShrayPagesz, (end - location) / ShrayPagesz,
            (void *)start, (void *)end);

    BitmapSetOnes(alloc->prefetched,
            (start - location) / ShrayPagesz, (end - location) / ShrayPagesz);

    /* FIXME shared ownership is possible, so we may get pages redundantly. */

    DBUG_PRINT("Prefetch [%p, %p[ from nodes %d, ..., %d",
            (void *)start, (void *)end, firstOwner, lastOwner);

    for (unsigned int rank = firstOwner; rank <= lastOwner; rank++) {
        uintptr_t theirStart = (location + rank * bytesPerBlock) /
            ShrayPagesz * ShrayPagesz;
        uintptr_t theirEnd = (rank == Shray_size - 1) ?
            roundUp(location + size, ShrayPagesz) * ShrayPagesz :
            roundUp(location + (rank + 1) * bytesPerBlock, ShrayPagesz) * ShrayPagesz;
        void *dest = (void *)max(start, theirStart);
        size_t nbytes = min(end, theirEnd) - max(start, theirStart);

        DBUG_PRINT("We prefetch [%p, %p[ from node %d and store it in %p",
                dest, (void *)((uintptr_t)dest + nbytes), rank, toShadow(dest, alloc));

        gasnet_get_nbi_bulk(toShadow(dest, alloc), rank, dest, nbytes);
    }
}

/* Resetting the protections is done by ShrayRealloc and ShraySync */
static void resetCache(Allocation *alloc)
{
    cache.usedMemory -= alloc->usedMemory;
    alloc->usedMemory = 0;

    BitmapReset(alloc->local);
    BitmapReset(alloc->prefetched);
    ringbuffer_reset(cache.autoCaches);
    queue_reset(cache.prefetchCaches);
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
    cache.autoCaches = ringbuffer_alloc(cache.maximumMemory / ShrayPagesz);
    cache.prefetchCaches = queue_alloc(cache.maximumMemory / ShrayPagesz);
    if (!cache.autoCaches || !cache.prefetchCaches) {
        fprintf(stderr, "Could not allocate cache buffers\n");
        gasnet_exit(1);
    }

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

    void *shadow;
    MMAP_SAFE(shadow, mmap(NULL, totalSize + ShrayPagesz, PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE,
                -1, 0));
    DBUG_PRINT("We allocate shadow [%p, %p[", shadow,
            (void *)((uintptr_t)shadow + totalSize + ShrayPagesz));

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
    uintptr_t firstPage = roundDownPage(firstByte);
    uintptr_t lastPage = roundDownPage(lastByte);

    size_t segmentSize = lastPage - firstPage + ShrayPagesz;

    DBUG_PRINT("Made a DSM allocation [%p, %p[, of which we own [%p, %p[.",
            location, (void *)((uintptr_t)location + totalSize),
            (void *)firstByte, (void *)(lastByte + 1));

    MPROTECT_SAFE((void *)firstPage, segmentSize, PROT_READ | PROT_WRITE);

    size_t totalPages = (roundUpPage((uintptr_t)location + totalSize) -
        (uintptr_t)location) / ShrayPagesz;

    heap.allocs[index].location = location;
    heap.allocs[index].size = totalSize;
    heap.allocs[index].bytesPerBlock = bytesPerBlock;
    heap.allocs[index].usedMemory = 0;
    heap.allocs[index].local = BitmapCreate(totalPages);
    heap.allocs[index].prefetched = BitmapCreate(totalPages);
    heap.allocs[index].shadow = shadow;

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

    Allocation *alloc = heap.allocs + findAlloc(array);
    resetCache(alloc);

    /* We cannot allow writes to invalid memory. */
    gasnet_wait_syncnbi_gets();

    MPROTECT_SAFE(alloc->location, alloc->size, PROT_NONE);

    uintptr_t firstByte = ((uintptr_t)alloc->location + Shray_rank * alloc->bytesPerBlock);
    uintptr_t lastByte = (Shray_rank == Shray_size - 1) ?
        (uintptr_t)alloc->location + alloc->size - 1 :
        (uintptr_t)alloc->location + (Shray_rank + 1) * alloc->bytesPerBlock - 1;

    /* First byte, last byte rounded down to page number. */
    uintptr_t firstPage = roundDownPage(firstByte);
    uintptr_t lastPage = roundDownPage(lastByte);

    size_t segmentSize = lastPage - firstPage + ShrayPagesz;

    MPROTECT_SAFE((void *)firstPage, segmentSize, PROT_READ | PROT_WRITE);
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
    uintptr_t firstPage = roundDownPage(location + firstByte);
    uintptr_t lastPage = roundDownPage(location + lastByte);

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
    PrefetchStruct prefetch = GetPrefetchStruct(address, size);

    DBUG_PRINT("Prefetch issued for [%p, %p[.", address, (void *)((uintptr_t)address + size));

    helpPrefetch(prefetch.start1, prefetch.end1, prefetch.alloc);
    helpPrefetch(prefetch.start2, prefetch.end2, prefetch.alloc);
    queue_queue(cache.prefetchCaches, prefetch.alloc, (void*)prefetch.start1, prefetch.end2 - prefetch.start1);
}

/* FIXME Not thread-safe. If we want to go that route, this should at the very least
 * only be called by the memory-thread. */
void ShrayDiscard(void *address, size_t size)
{
    PrefetchStruct prefetch = GetPrefetchStruct(address, size);
    /* It would be strange to discard before we even have the data, but it is possible. */
    gasnet_wait_syncnbi_gets();

    DBUG_PRINT("We free [%p, %p[ and [%p, %p[", (void *)prefetch.start1, (void *)prefetch.end1,
            (void *)prefetch.start2, (void *)prefetch.end2);

    freeRAM(prefetch.start1, prefetch.end1);
    freeRAM(prefetch.start2, prefetch.end2);

    discardBitmap(prefetch.start1, prefetch.end1, prefetch.alloc);
    discardBitmap(prefetch.start2, prefetch.end2, prefetch.alloc);
    size_t index = queue_find(cache.prefetchCaches, prefetch.alloc, (void*)prefetch.start1);
    queue_remove_at(cache.prefetchCaches, index);
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
    ringbuffer_free(cache.autoCaches);
    queue_free(cache.prefetchCaches);
    gasnet_exit(exit_code);
}
