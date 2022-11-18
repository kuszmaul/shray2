/* Distribution: 1d it is a block distribution on the bytes, so
 * phi_s(k) = k + s * roundUp(n, p), in the higher dimensional case,
 * we distribute blockwise along the first dimension. See also the
 * definitions of Aw_r, Ar_r, Ap_r. */

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
    return (heap.allocs[index].location <= (uintptr_t)address) &&
        ((uintptr_t)address < heap.allocs[index].location + heap.allocs[index].size);
}

static void *toShadow(uintptr_t addr, Allocation *alloc)
{
    return (void *)(addr - alloc->location + (uintptr_t)alloc->shadow);
}

static uintptr_t roundUpPage(uintptr_t addr)
{
    return roundUp(addr, ShrayPagesz) * ShrayPagesz;
}

static uintptr_t roundDownPage(uintptr_t addr)
{
    return addr / ShrayPagesz * ShrayPagesz;
}

/* Aw_r := [startWrite(A, r), endWrite(A, r)[ is the part of A that rank r should
 * calculate, and that it writes to. (Aw_r)_r partitions A, Aw_r is not page-aligned. */
static inline uintptr_t startWrite(Allocation *alloc, unsigned int rank)
{
    return alloc->location + rank * alloc->bytesPerBlock;
}

static inline uintptr_t endWrite(Allocation *alloc, unsigned int rank)
{
    return (rank == Shray_size - 1) ?
        alloc->location + alloc->size :
        alloc->location + (rank + 1) * alloc->bytesPerBlock;
}

/* Ar_r := [startRead(A, r), endRead(A, r)[ is the part of A that is stored on node r.
 * (Ar_r)_r covers A, but is not a partition. Ar_r is the minimal page-aligned superset
 * of Aw_r. */
static inline uintptr_t startRead(Allocation *alloc, unsigned int rank)
{
    return roundDownPage(alloc->location + rank * alloc->bytesPerBlock);
}

static inline uintptr_t endRead(Allocation *alloc, unsigned int rank)
{
    return (rank == Shray_size - 1) ?
        roundUpPage(alloc->location + alloc->size) :
        roundUpPage(alloc->location + (rank + 1) * alloc->bytesPerBlock);
}

/* Ap_r := [startPartition(A, r), endPartition(A, r)[ is a subset of Ar_r such that
 * (Ap_r)_r is a partitioning of \bigcup_r (Ar_r)_r (which is A + some dummy entries
 * at the last page of the allocation). Note, Ap_r may be empty! */
static inline uintptr_t startPartition(Allocation *alloc, unsigned int rank)
{
    return (endRead(alloc, rank - 1) == startRead(alloc, rank) + ShrayPagesz) ?
        startRead(alloc, rank) + ShrayPagesz :
        startRead(alloc, rank);
}

static inline uintptr_t endPartition(Allocation *alloc, unsigned int rank)
{
    return endRead(alloc, rank);
}

/* Frees [start, end[. start, end need to be ShrayPagesz-aligned */
static inline void freeRAM(uintptr_t start, uintptr_t end)
{
    if (start + ShrayPagesz >= end) return;

    DBUG_PRINT("We free [%p, %p[", (void *)start, (void *)end);

    void *dummy;
    MUNMAP_SAFE((void *)start, end - start);
    MMAP_SAFE(dummy, mmap((void *)start, end - start, PROT_NONE,
                MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0));
}

/* Sets [start, end[ to zero in the prefetch, local bitmaps, and decreases the used
 * memory by the appropiate amount. */
static void discardBitmap(uintptr_t start, uintptr_t end, Allocation *alloc)
{
    if (start >= end) return;

    size_t firstPageNumber = (start - alloc->location) / ShrayPagesz;
    size_t lastPageNumber = (end - alloc->location) / ShrayPagesz;

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
        uintptr_t location = heap.allocs[middle].location;
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
     * Let Ar_ourRank = [ourStart, ourEnd[. Then we need to fetch
     * [start, end[ \cap Ar_Shray_rank^c =
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

    uintptr_t ourStart = startRead(alloc, Shray_rank);
    uintptr_t ourEnd = endRead(alloc, Shray_rank);

    PrefetchStruct result;

    result.get1.start = start;
    result.get1.end = min(end, ourStart);
    result.get2.start = max(start, ourEnd);
    result.get2.end = end;
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
    size_t index = ((uintptr_t)start - alloc->location) / ShrayPagesz;
    size_t size = pages * ShrayPagesz;
    BitmapSetZeroes(alloc->local, index, index + pages);

    freeRAM((uintptr_t)start, size);
    cache.usedMemory -= pages * ShrayPagesz;
    alloc->usedMemory -= pages * ShrayPagesz;
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
    return;
    cache_entry_t *next, *entry;
    size_t evicted = 0;
    size_t chain = 1;
    size_t size_pages = roundUp(size, ShrayPagesz);

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

    size_t pageNumber = (roundedAddress - alloc->location) / ShrayPagesz;
    DBUG_PRINT("Page %zu", pageNumber);


    /* FIXME this waits for all prefetches, which we do not want to do as we will have a
     * prefetch 1 -> compute 0 -> prefetch 2 -> compute 1 kind of pattern in most applications,
     * so when we do compute n we wait for prefetch n, but at that point we have already
     * issued prefetch n + 1 just before. */
    if (BitmapCheck(alloc->prefetched, pageNumber)) {
//        Range range = BitmapSurrounding(alloc->prefetched, pageNumber);
//
//        DBUG_PRINT("%p is currently being prefetched, along with [%p, %p[",
//               address, (void *)(alloc->location + range.start * ShrayPagesz),
//               (void *)(alloc->location + range.end * ShrayPagesz));

        /* TODO
         * GetStruct *get = findGet(roundedAddress, cache.prefetches);
         * gasnet_wait_syncnb_all(get->handles, get->numberOfHandles);
         * void *start = (void *)get->start;
         * MREMAP_MOVE(start, toShadow(get->start, alloc), get->end - get->start);
         * MMAP_SAFE(start, mmap(toShadow(get->start, alloc), get->end - get->start, PROT_WRITE,
         *           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
         * BitmapSetZeroes(alloc->prefetched, get.start / ShrayPagesz, get.end / ShrayPagesz);
         * BitmapSetZeroes(alloc->local, get.start / ShrayPagesz, get.end / ShrayPagesz);
         * remove(get, cache.prefetches);
         *
         * We only use alloc->prefetched bitmap to make checking
         * whether it is prefetched O(1) instead of linear in the number of prefetches. */

//        uintptr_t start = alloc->location + range.start * ShrayPagesz;
//        void *voidStart = (void *)start;
//        uintptr_t length = (range.end - range.start) * ShrayPagesz;

//        DBUG_PRINT("We bring pages [%zu, %zu[ ([%p, %p[) into the light",
//                range.start, range.end, (void *)start, (void *)(start + length));

//        MREMAP_MOVE(voidStart, toShadow(start, alloc), length);
//        /* We immediately remap the shadow part to not leave holes in our allocation.
//         * Because of lazy allocation, this does not increase the memory footprint.
//         * Using MREMAP_DONTUNMAP would. */
//        void *dummy;
//        MMAP_SAFE(dummy, mmap(toShadow(start, alloc), length, PROT_WRITE,
//                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
//
//        BitmapSetZeroes(alloc->prefetched, range.start, range.end);
//        BitmapSetZeroes(alloc->local, range.start, range.end);
    } else {
        DBUG_PRINT("%p is not being prefetched, perform blocking fetch.", address);
        if (cache.usedMemory + ShrayPagesz > cache.maximumMemory) {
            DBUG_PRINT("We free up %zu bytes of cache memory", cache.maximumMemory / 10);
            /*
             * TODO: Need to do a wait here as well, if the cache is full
             * and we need to evict we might need to evict prefetched data. In
             * that case we would need to do the same thing as in the if case
             * above.
             */
            evict(cache.maximumMemory / 10);
        }

        cache.usedMemory += ShrayPagesz;
//        alloc->usedMemory += ShrayPagesz;

        uintptr_t difference = roundedAddress - alloc->location;
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
    DBUG_PRINT("Update page %p of allocation %p", (void *)page, (void *)heap.allocs[index].location);

    /* We have communication with a rank when allocStart <= page + ShrayPagesz - 1 and
     * allocEnd - 1 >= page. Solving the inequalities yields the following loop. */
    uintptr_t location = heap.allocs[index].location;
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

/* Gets [get.start, get.end[ asynchronously to the allocation shadow
 * start, end are page-aligned and not owned by our rank. Sets the handles
 * of the get. */
static inline void helpPrefetch(GetStruct *get, Allocation *alloc)
{
    uintptr_t start = get->start;
    uintptr_t end = get->end;
    if (end <= start) return;

    uintptr_t location = alloc->location;
    uintptr_t bytesPerBlock = alloc->bytesPerBlock;

    unsigned int firstOwner = (start - location) / bytesPerBlock;
    /* We take the min with Shay_size - 1 because the last part of the last page is not owned by
     * anyone. */
    unsigned int lastOwner = min((end - 1 - location) / bytesPerBlock, Shray_size - 1);

    DBUG_PRINT("Prefetch [%p, %p[ from nodes %d, ..., %d",
            (void *)start, (void *)end, firstOwner, lastOwner);

    MALLOC_SAFE(get->handles, (lastOwner - firstOwner + 1) * sizeof(gasnet_handle_t));
    get->numberOfHandles = 0;
    /* As Ap_r for r = firstOwner, ..., lastOwner covers [start, end[, we can take
     * the intersection with it to get a partition [start_r, end_r[ of [start, end[.
     * This is necessary to ensure we do not get pages twice. */
    for (unsigned int rank = firstOwner; rank <= lastOwner; rank++) {
        uintptr_t start_r = max(start, startPartition(alloc, rank));
        uintptr_t end_r = min(end, endPartition(alloc, rank));

        if (start_r < end_r) {
            DBUG_PRINT("Get [%p, %p[ from node %d and store it in %p",
                    (void *)start_r, (void *)end_r, rank, toShadow(start_r, alloc));

            get->handles[get->numberOfHandles] =
                gasnet_get_nb_bulk(toShadow(start_r, alloc), rank, (void *)start_r, end_r - start_r);
            get->numberOfHandles++;

            DBUG_PRINT("We set this to prefetched (pages [%zu, %zu[)",
                (start_r - location) / ShrayPagesz, (end_r - location) / ShrayPagesz);

            BitmapSetOnes(alloc->prefetched, (start_r - location) / ShrayPagesz,
                (end_r - location) / ShrayPagesz);

            // TODO add to queue or ringbuffer
        }
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
        location = (void *)roundUpPage((uintptr_t)mmapAddress);
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
    while (index > 0 && heap.allocs[index - 1].location > (uintptr_t)location) {
        heap.allocs[index] = heap.allocs[index - 1];
        index--;
    }

    Allocation *alloc = heap.allocs + index;

    /* We distribute blockwise over the first dimension. */
    size_t bytesPerLatterDimensions = totalSize / firstDimension;
    size_t bytesPerBlock = roundUp(firstDimension, Shray_size) * bytesPerLatterDimensions;

    alloc->location = (uintptr_t)location;
    alloc->size = totalSize;
    alloc->bytesPerBlock = bytesPerBlock;
    alloc->shadow = shadow;

    size_t segmentLength = endRead(alloc, Shray_rank) - startRead(alloc, Shray_rank);

    DBUG_PRINT("Made a DSM allocation [%p, %p[, of which \n\t\tAw = [%p, %p[, "
            "\n\t\tAr = [%p, %p[,\n\t\tAp = [%p, %p[.",
            location, (void *)((uintptr_t)location + totalSize),
            (void *)startWrite(alloc, Shray_rank), (void *)endWrite(alloc, Shray_rank),
            (void *)startRead(alloc, Shray_rank), (void *)endRead(alloc, Shray_rank),
            (void *)startPartition(alloc, Shray_rank), (void *)endPartition(alloc, Shray_rank));

    MPROTECT_SAFE((void *)startRead(alloc, Shray_rank), segmentLength, PROT_READ | PROT_WRITE);

    alloc->local = BitmapCreate(roundUp(totalSize, ShrayPagesz));
    alloc->prefetched = BitmapCreate(roundUp(totalSize, ShrayPagesz));

    gasnetBarrier();
    BARRIERCOUNT;

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

    MPROTECT_SAFE((void *)alloc->location, alloc->size, PROT_NONE);

    uintptr_t firstPage = startRead(alloc, Shray_rank);
    uintptr_t lastPage = endRead(alloc, Shray_rank);

    MPROTECT_SAFE((void *)firstPage, lastPage - firstPage, PROT_READ | PROT_WRITE);
}

void ShraySync(void *array)
{
    /* So the other processors have finished writing before we get their parts of the pages. */
    gasnetBarrier();
    BARRIERCOUNT

    int index = findAlloc(array);
    Allocation *alloc = heap.allocs + index;

    /* Synchronise in case the first or last page is co-owned with someone else. */
    uintptr_t firstPage = startRead(alloc, Shray_rank);
    uintptr_t lastPage = endRead(alloc, Shray_rank) - ShrayPagesz;

    UpdatePage(firstPage, index);
    if (firstPage != lastPage) UpdatePage(lastPage, index);
    gasnet_wait_syncnbi_gets();
    DBUG_PRINT("We are done updating pages for %p", array);

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
    Allocation *alloc = heap.allocs + index;
    /* We leave potentially two pages mapped due to the alignment in ShrayMalloc, but
     * who cares. */
    MUNMAP_SAFE((void *)alloc->location, alloc->size);
    BitmapFree(alloc->local);
    BitmapFree(alloc->prefetched);
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
    if (size > cache.maximumMemory) {
        DBUG_PRINT("Can not prefetch %zu bytes since cache is only %zu. Ignoring prefetch",
                size, cache.maximumMemory);
        return;
    }

    PrefetchStruct prefetch = GetPrefetchStruct(address, size);

    DBUG_PRINT("Prefetch issued for [%p, %p[.", address, (void *)((uintptr_t)address + size));

    helpPrefetch(&(prefetch.get1), prefetch.alloc);
    helpPrefetch(&(prefetch.get2), prefetch.alloc);
    // TODO: this might be two entries, check if that is the case and if so add
    // 2 entries instead
//    queue_queue(cache.prefetchCaches, prefetch.alloc, (void*)prefetch.start1, prefetch.get2.end - prefetch.get1.start);
}

/* FIXME Not thread-safe. If we want to go that route, this should at the very least
 * only be called by the memory-thread. */
void ShrayDiscard(void *address, size_t size)
{
    PrefetchStruct prefetch = GetPrefetchStruct(address, size);
//    size_t index = queue_find(cache.prefetchCaches, prefetch.alloc, (void*)prefetch.start1);
//    if (index == NOLINK) {
//        DBUG_PRINT("Ignoring discard for %p since prefetch was not found", (void*)prefetch.start1);
//        return;
//    }

    /* It would be strange to discard before we even have the data, but it is possible. */
    gasnet_wait_syncnbi_gets();

    freeRAM(prefetch.get1.start, prefetch.get1.end);
    freeRAM(prefetch.get2.start, prefetch.get2.end);

    discardBitmap(prefetch.get1.start, prefetch.get1.end, prefetch.alloc);
    discardBitmap(prefetch.get2.start, prefetch.get2.end, prefetch.alloc);
    // TODO: see TODO for prefetch
   // queue_remove_at(cache.prefetchCaches, index);
    // TODO: discard needs to evict the actual entry, however evictCacheEntry
    // also calls unmap/map which conflicts with freeRAM
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
//    ringbuffer_free(cache.autoCaches);
//    queue_free(cache.prefetchCaches);
    gasnet_exit(exit_code);
}
