/* Distribution: 1d it is a block distribution on the bytes, so
 * phi_s(k) = k + s * roundUp(n, p), in the higher dimensional case,
 * we distribute blockwise along the first dimension. See also the
 * definitions of Aw_r, Ar_r, Ap_r. */

#include "shray.h"
#include "bitmap.h"
#include "queue.h"
#include "ringbuffer.h"
#include "shray2/shray.h"
#include <assert.h>
#include <stdint.h>
#include <pthread.h>

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

// TODO: this is not really necessary anymore
enum WorkerState
{
    WORKER_STATE_WAITING = 0,
    WORKER_STATE_BUSY = 1,
    WORKER_STATE_TERMINATE = 2
};

typedef struct
{
    uintptr_t address;
    size_t thread_index;
    int request;
    volatile enum WorkerState state;
    shray_fn handler;
    worker_info_t info;
    pthread_t id;
    volatile int work;
} shray_worker_t;

// TODO: move threading to its own file? */
static size_t workerThreadCount;
static shray_worker_t *workerThreads;
static pthread_t memoryThread;
static pthread_barrier_t workerBarrier;

static pthread_key_t key;

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
    if (start >= end) return;

    DBUG_PRINT("We free [%p, %p[", (void *)start, (void *)end);

    MUNMAP_SAFE((void *)start, end - start);
    MMAP_FIXED_SAFE((void *)start, end - start, PROT_NONE);
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
    /* We get the minimal page-aligned subset [start, end[ of [address, size[,
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
    uintptr_t start = roundUpPage((uintptr_t)address);
    uintptr_t end = roundDownPage((uintptr_t)address + size);

    Allocation *alloc = heap.allocs + findAlloc(address);

    uintptr_t ourStart = startRead(alloc, Shray_rank);
    uintptr_t ourEnd = endRead(alloc, Shray_rank);

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
static void evictCacheEntry(Allocation *alloc, uintptr_t start, size_t pages)
{
    size_t index = (start - alloc->location) / ShrayPagesz;
    size_t size = pages * ShrayPagesz;
    BitmapSetZeroes(alloc->local, index, index + pages);
    BitmapSetZeroes(alloc->prefetched, index, index + pages);

    DBUG_PRINT("evictCacheEntry: we free page %zu", index);
    freeRAM(start, start + size);
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

        evictCacheEntry(entry->alloc, (uintptr_t)entry->start, chain);
        /* Only evict as much as needed */
        evicted += chain;
        if (evicted >= size_pages) {
            return;
        }
    }

    if (evicted < size_pages) {
        DBUG_PRINT("Was only able to evict %zu pages (requested %zu) SHOULD NEVER HAPPEN",
                evicted, size_pages);
    }
}

static void handlePageFault(void *address)
{
    uintptr_t roundedAddress = roundDownPage((uintptr_t)address);

    DBUG_PRINT("Segfault %p", address);

    Allocation *alloc = heap.allocs + findAlloc((void *)roundedAddress);

    size_t pageNumber = (roundedAddress - alloc->location) / ShrayPagesz;
    DBUG_PRINT("Page %zu", pageNumber);

    if (BitmapCheck(alloc->prefetched, pageNumber)) {
        /* Find the prefetch. */
        queue_entry_t *entry = queue_find(cache.prefetchCaches, roundedAddress);

        if (entry == NULL) {
            DBUG_PRINT("%p set to prefetched, but was not in the prefetch queue",
                    (void *)roundedAddress);
            ShrayFinalize(1);
        }

        gasnet_wait_syncnb(entry->handle);
        entry->gottem = 1;

        void *start = (void *)entry->start;

        DBUG_PRINT("%p is currently being prefetched, along with [%p, %p[ (pages [%zu, %zu[)",
               address, start, (void *)entry->end, (entry->start - alloc->location) / ShrayPagesz,
               (entry->end - alloc->location) / ShrayPagesz);

        MREMAP_MOVE(start, toShadow(entry->start, alloc), entry->end - entry->start);
        MMAP_FIXED_SAFE(toShadow(entry->start, alloc), entry->end - entry->start,
                PROT_READ | PROT_WRITE);
        BitmapSetOnes(alloc->local, (entry->start - alloc->location) / ShrayPagesz,
                (entry->end - alloc->location) / ShrayPagesz);
    } else {
        DBUG_PRINT("%p is not being prefetched, perform blocking fetch.", address);
        if (cache.usedMemory + ShrayPagesz > cache.maximumMemory) {
            DBUG_PRINT("We free up %zu bytes of cache memory",
                    min(cache.maximumMemory / 10, cache.usedMemory));
            evict(min(cache.maximumMemory / 10, cache.usedMemory));
        }

        cache.usedMemory += ShrayPagesz;
        alloc->usedMemory += ShrayPagesz;

        uintptr_t difference = roundedAddress - alloc->location;
        unsigned int owner = difference / alloc->bytesPerBlock;

        DBUG_PRINT("Segfault is owned by node %d.", owner);

        gasnet_get(alloc->shadowPage, owner, (void *)roundedAddress, ShrayPagesz);

        MREMAP_MOVE((void *)roundedAddress, alloc->shadowPage, ShrayPagesz);
        MMAP_FIXED_SAFE(alloc->shadowPage, ShrayPagesz, PROT_READ | PROT_WRITE);


        DBUG_PRINT("We set page %zu to locally available.", pageNumber);

        BitmapSetOnes(alloc->local, pageNumber, pageNumber + 1);
        ringbuffer_add(cache.autoCaches, alloc, (void*)roundedAddress);
    }
}

static void handleWorkerFault(void *address)
{
    shray_worker_t *worker = pthread_getspecific(key);
    if (!worker) {
        fprintf(stderr, "SEGFAULT occurred for a non-shray managed thread\n");
        ShrayFinalize(1);
    }

    DBUG_PRINT("Segfault in worker %zu", worker->thread_index);
    sigset_t mask;
    sigemptyset(&mask);

    worker->address = (uintptr_t)address;
    worker->request = 1;
    sigsuspend(&mask);
}

static void SegvHandler(int sig, siginfo_t *si, void *unused)
{
    SEGFAULTCOUNT
    if (pthread_self() == memoryThread) {
        /* Single-threaded case */
        handlePageFault(si->si_addr);
    } else {
        /* Segfault in a worker thread. */
        handleWorkerFault(si->si_addr);
    }
}

static void Usr1Handler(int sig, siginfo_t *si, void *unused)
{
    if (pthread_self() == memoryThread) {
        fprintf(stderr, "Memory thread received unexpected USR1");
        ShrayFinalize(1);
    }

    shray_worker_t *worker = pthread_getspecific(key);
    DBUG_PRINT("Received SIGUSR1 (worker %zu)", worker->thread_index);
}

static void registerHandlers(void)
{
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset (&sa.sa_mask);
    sa.sa_sigaction = SegvHandler;

    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("Registering SIGSEGV handler failed.\n");
        gasnet_exit(1);
    }

    sigemptyset (&sa.sa_mask);
    sa.sa_sigaction = Usr1Handler;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("Registering SIGUSR1 handler failed.\n");
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

static void *workerMain(void *args)
{
    shray_worker_t *worker = args;
    DBUG_PRINT("Starting working %zu (%lu)", worker->thread_index, worker->id);
    pthread_setspecific(key, worker);
    while (worker->state != WORKER_STATE_TERMINATE) {
        if (worker->work) {
            DBUG_PRINT("Worker %zu (%lu) received work", worker->thread_index, worker->id);
            worker->handler(&worker->info);
            worker->work = 0;
            DBUG_PRINT("Worker %zu (%lu) finished work", worker->thread_index, worker->id);
            pthread_barrier_wait(&workerBarrier);
        }
    }

    pthread_exit(NULL);
}

/* Gets [get.start, get.end[ asynchronously to the allocation shadow
 * start, end are page-aligned and not owned by our rank. Adds gets to the queue. */
static inline void helpPrefetch(uintptr_t start, uintptr_t end, Allocation *alloc)
{
    if (end <= start) return;

    uintptr_t location = alloc->location;
    uintptr_t bytesPerBlock = alloc->bytesPerBlock;

    unsigned int firstOwner = (start - location) / bytesPerBlock;
    /* We take the min with Shay_size - 1 because the last part of the last page is not owned by
     * anyone. */
    unsigned int lastOwner = min((end - 1 - location) / bytesPerBlock, Shray_size - 1);

    DBUG_PRINT("Prefetch [%p, %p[ from nodes %d, ..., %d",
            (void *)start, (void *)end, firstOwner, lastOwner);

    /* As Ap_r for r = firstOwner, ..., lastOwner covers [start, end[, we can take
     * the intersection with it to get a partition [start_r, end_r[ of [start, end[.
     * This is necessary to ensure we do not get pages twice. */
    for (unsigned int rank = firstOwner; rank <= lastOwner; rank++) {
        uintptr_t start_r = max(start, startPartition(alloc, rank));
        uintptr_t end_r = min(end, endPartition(alloc, rank));

        if (start_r < end_r) {
            DBUG_PRINT("Get [%p, %p[ from node %d and store it in %p",
                    (void *)start_r, (void *)end_r, rank, toShadow(start_r, alloc));

            gasnet_handle_t handle = gasnet_get_nb_bulk(toShadow(start_r, alloc),
                    rank, (void *)start_r, end_r - start_r);

            DBUG_PRINT("We set this to prefetched (pages [%zu, %zu[)",
                (start_r - location) / ShrayPagesz, (end_r - location) / ShrayPagesz);

            queue_queue(cache.prefetchCaches, alloc, start_r, end_r, handle);

            BitmapSetOnes(alloc->prefetched, (start_r - location) / ShrayPagesz,
                (end_r - location) / ShrayPagesz);
        }
    }
}

/* Is linear in the number of allocations when we do not prefetch. */
void ShrayResetCache()
{
    cache.usedMemory = 0;

    /* Finish the prefetches */
    size_t next_index = cache.prefetchCaches->data_start;
    while (next_index != NOLINK) {
    	queue_entry_t *entry = &cache.prefetchCaches->data[next_index];

        if (!entry->gottem) gasnet_wait_syncnb(entry->handle);

        next_index = entry->next;
    }

    /* Reset the protections and bitmaps. */
    for (unsigned int i = 0; i < heap.numberOfAllocs; i++) {
        Allocation *alloc = heap.allocs + i;
        alloc->usedMemory = 0;

        BitmapReset(alloc->local);
        BitmapReset(alloc->prefetched);
        freeRAM(alloc->location, startRead(alloc, Shray_rank));
        freeRAM(endRead(alloc, Shray_rank), alloc->location + alloc->size);
    }

    /* Free the cache data structures. */
    ringbuffer_reset(cache.autoCaches);
    queue_reset(cache.prefetchCaches);
}

static void startShrayWorkers()
{
    MALLOC_SAFE(workerThreads, sizeof(shray_worker_t) * workerThreadCount);
    /* + 1 for the memory thread */
    pthread_barrier_init(&workerBarrier, NULL, workerThreadCount + 1);
    for (size_t i = 0; i < workerThreadCount; ++i) {
        shray_worker_t *worker = &workerThreads[i];
        worker->handler = NULL;
        worker->thread_index = i;
        worker->info.args = NULL;
        worker->info.start = 0;
        worker->info.end = 0;
        worker->state = WORKER_STATE_WAITING;
        worker->request = 0;
        worker->address = 0;
        worker->work = 0;

        if (pthread_create(&workerThreads[i].id, NULL, workerMain, worker)) {
            fprintf(stderr, "Could not create worker thread %zu\n", i);
            ShrayFinalize(1);
        }
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

    // TODO: use macro to check whether or not multi-threading is supported
    char *workerThreadsEnv = getenv("SHRAY_WORKERTHREADS");
    if (workerThreadsEnv == NULL) {
        fprintf(stderr, "Please set the worker threads environment variable SHRAY_WORKERTHREADS, use 0 for single threaded\n");
        gasnet_exit(1);
    } else {
        workerThreadCount = atol(workerThreadsEnv);
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
    cache.prefetchCaches = queue_alloc(5);
    if (!cache.autoCaches || !cache.prefetchCaches) {
        fprintf(stderr, "Could not allocate cache buffers\n");
        gasnet_exit(1);
    }

    memoryThread = pthread_self();
    pthread_key_create(&key, NULL);
    if (workerThreadCount > 0) {
        startShrayWorkers();
    }

    registerHandlers();
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
        MMAP_SAFE(mmapAddress, NULL, totalSize + 2 * ShrayPagesz, PROT_NONE);
        location = (void *)roundUpPage((uintptr_t)mmapAddress);
        DBUG_PRINT("mmapAddress = %p, allocation start = %p", mmapAddress, location);
    }

    /* Broadcast location to the other nodes. */
    gasnet_coll_broadcast(gasnete_coll_team_all, &location, 0, &location,
            sizeof(void *), GASNET_COLL_DST_IN_SEGMENT);

    if (Shray_rank != 0) {
        MMAP_FIXED_SAFE(location, totalSize + ShrayPagesz, PROT_NONE);
    }

    void *shadow;
    MMAP_SAFE(shadow, NULL, totalSize + ShrayPagesz, PROT_WRITE);
    DBUG_PRINT("We allocate shadow [%p, %p[", shadow,
            (void *)((uintptr_t)shadow + totalSize + ShrayPagesz));
    void *shadowPage;
    MMAP_SAFE(shadowPage, NULL, ShrayPagesz, PROT_WRITE);
    DBUG_PRINT("We allocate shadow %p", shadowPage);

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
    alloc->shadowPage = shadowPage;

    size_t segmentLength = endRead(alloc, Shray_rank) - startRead(alloc, Shray_rank);

    DBUG_PRINT("Made a DSM allocation [%p, %p[, of which \t\tAw = [%p, %p[, "
            "\t\tAr = [%p, %p[,\t\tAp = [%p, %p[.",
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

    /* Technically this is not necessary after every sync, but the call is cheap, and it makes
     * the consistency easier. A reuse of buffers is now always allowed for the following reason:
     * Suppose we use the same buffer for A and B. We can only read old data of A when reading B
     * if the same location was read of A. But that is only allowed after a ShraySync(A), which
     * resets the cache. */
    ShrayResetCache();

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

void ShrayPrefetch(void *address, size_t size)
{
    PrefetchStruct prefetch = GetPrefetchStruct(address, size);

    DBUG_PRINT("Prefetch issued for [%p, %p[.", address, (void *)((uintptr_t)address + size));

    helpPrefetch(prefetch.start1, prefetch.end1, prefetch.alloc);
    helpPrefetch(prefetch.start2, prefetch.end2, prefetch.alloc);
}

static void DiscardGet(queue_entry_t *get, size_t index)
{
    DBUG_PRINT("We discard [%p, %p[", (void *)get->start, (void *)get->end);
    Allocation *alloc = (Allocation *)get->alloc;

    BitmapSetZeroes(alloc->prefetched, (get->start - alloc->location) / ShrayPagesz,
            (get->end - alloc->location) / ShrayPagesz);

    /* If not everything is local yet, we are not yet done prefetching. Wait for the prefetch
     * to complete, and free the shadow region. Otherwise, free the light-region. */
    if (get->gottem) {
        DBUG_PRINT("We had already finished get [%p, %p[.", (void *)get->start, (void *)get->end);
        freeRAM(get->start, get->end);
        BitmapSetZeroes(alloc->local, (get->start - alloc->location) / ShrayPagesz,
                (get->end - alloc->location) / ShrayPagesz);
    } else {
        DBUG_PRINT("We had not finished getting [%p, %p[.", (void *)get->start, (void *)get->end);
        gasnet_wait_syncnb(get->handle);
        uintptr_t start = (uintptr_t)toShadow(get->start, alloc);
        uintptr_t end = start + get->end - get->start;
        MUNMAP_SAFE((void *)start, end - start);
        MMAP_FIXED_SAFE((void *)start, end - start, PROT_READ | PROT_WRITE);
    }

    queue_remove_at(cache.prefetchCaches, index);
}

/* Returns true iff [subStart, subEnd[ \subseteq [start, end[. */
static int IsSubset(uintptr_t subStart, uintptr_t subEnd, uintptr_t start, uintptr_t end)
{
    return (subStart >= start && subStart < end && subEnd <= end && subEnd > subStart);
}

void ShrayDiscard(void *address, size_t size)
{
    PrefetchStruct prefetch = GetPrefetchStruct(address, size);

    /* Walk through the prefetch-queue, and delete everything prefetched by the
     * issue [address, address + size[. */
    size_t next_index = cache.prefetchCaches->data_start;
    while (next_index != NOLINK) {
        queue_entry_t *entry = &(cache.prefetchCaches->data[next_index]);

        if (IsSubset(entry->start, entry->end, prefetch.start1, prefetch.end1) ||
            IsSubset(entry->start, entry->end, prefetch.start2, prefetch.end2))
        {
            DiscardGet(entry, next_index);
        }

        next_index = entry->next;
    }
}

static int pageFaultHandled(uintptr_t address)
{
    uintptr_t roundedAddress = roundDownPage(address);
    Allocation *alloc = heap.allocs + findAlloc((void *)roundedAddress);
    size_t pageNumber = (roundedAddress - alloc->location) / ShrayPagesz;
    return BitmapCheck(alloc->local, pageNumber);
}

/* A page fault occurred in the given worker */
static void handleWorkerRequest(shray_worker_t *worker)
{
    DBUG_PRINT("Handling request for worker %zu (%p)", worker->thread_index, (void*)worker->address);
    if (!pageFaultHandled(worker->address)) {
        handlePageFault((void*)worker->address);
    }

    worker->request = 0;
    DBUG_PRINT("Sending USR1 to worker %zu (%lu)", worker->thread_index, worker->id);
    pthread_kill(worker->id, SIGUSR1);
}

void ShrayRunWorker(shray_fn handler, size_t n, void *args)
{
    DBUG_PRINT("Starting threaded workload (%zu workers)", workerThreadCount);
    if (workerThreadCount == 0) {
        fprintf(stderr, "Can not run ShrayRunWorker without any workers");
        ShrayFinalize(1);
    }

    /* Start the worker threads */
    size_t start = ShrayStart(n);
    size_t end = ShrayEnd(n);
    size_t workPerThread = (end - start) / workerThreadCount;
    for (size_t i = 0; i < workerThreadCount; ++i) {
        shray_worker_t *worker = &workerThreads[i];
        worker->handler = handler;
        worker->info.args = args;
        worker->info.start = start + i * workPerThread;
        worker->info.end = i == workerThreadCount - 1 ? end : worker->info.start + workPerThread;
        DBUG_PRINT("Thread %zu does work on slabs [%zu, %zu[\n", i,
                worker->info.start, worker->info.end);
        worker->state = WORKER_STATE_BUSY;
        worker->request = 0;
        worker->address = 0;

        worker->work = 1;
    }

    /* Listen for memory requests until all workers have finished. */
    size_t busy = workerThreadCount;
    while (busy != 0) {
        busy = workerThreadCount;

        for (size_t i = 0; i < workerThreadCount; ++i) {
            shray_worker_t *worker = &workerThreads[i];

            if (worker->request) {
                handleWorkerRequest(worker);
            }

            if (!worker->work) {
                busy -= 1;
            }
        }
    }

    pthread_barrier_wait(&workerBarrier);
    DBUG_PRINT("Finished threaded workload (%zu workers)", workerThreadCount);
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

static void terminateWorkers()
{
    for (size_t i = 0; i < workerThreadCount; ++i) {
        shray_worker_t *worker = &workerThreads[i];
        worker->state = WORKER_STATE_TERMINATE;
        /*
         * Can only join on threads that have finished their task. If this
         * function is called on non-error termination then no threads are
         * working. In case of a crash gasnet_exit will terminate the process
         * anyway.
         */
        if (!worker->work) {
            pthread_join(worker->id, NULL);
        }
    }
}

void ShrayFinalize(int exit_code)
{
    DBUG_PRINT("Terminating with code %d", exit_code);
    if (workerThreadCount > 0) {
        terminateWorkers();
        pthread_barrier_destroy(&workerBarrier);
    }
    free(workerThreads);
    ringbuffer_free(cache.autoCaches);
    queue_free(cache.prefetchCaches);
    gasnet_exit(exit_code);
}
