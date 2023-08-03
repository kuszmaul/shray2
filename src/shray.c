/* Distribution: 1d it is a block distribution on the bytes, so
 * phi_s(k) = k + s * roundUp(n, p), in the higher dimensional case,
 * we distribute blockwise along the first dimension. See also the
 * definitions of Aw_r, Ar_r, Ap_r. */

#include "shray.h"
#include "bitmap.h"
#include "ringbuffer.h"
#include "shray2/shray.h"
#include <assert.h>
#include <signal.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

/*****************************************************
 * Global variable declarations.
 *****************************************************/

/* We need this global in cases functions with different
 * SHRAY_SUFFIX'es are called in one program. */

#if !(DEBUG || PROFILE)
/* TODO: move cache stuff to its own file */

unsigned int Shray_rank;
unsigned int Shray_size;
size_t Shray_SegfaultCounter;
size_t Shray_BarrierCounter;
size_t Shray_Pagesz;
size_t Shray_CacheLineSize;
double Shray_CacheAllocFactor;
Heap heap;

bool ShrayOutput;
#endif

#define HOSTNAME_LENGTH 256
static char ShrayHost[HOSTNAME_LENGTH];

static bool thread_lock;

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
    BARRIERCOUNT
}

static int inRange(void *address, int index)
{
    return (heap.allocs[index].location <= (uintptr_t)address) &&
        ((uintptr_t)address < heap.allocs[index].location + heap.allocs[index].size);
}

static uintptr_t roundUpPage(uintptr_t addr)
{
    return roundUp(addr, Shray_Pagesz) * Shray_Pagesz;
}

static uintptr_t roundDownPage(uintptr_t addr)
{
    return addr / Shray_Pagesz * Shray_Pagesz;
}

/* Aw_r := [startWrite(A, r), endWrite(A, r)[ is the part of A that rank r
 * should calculate, and that it writes to. (Aw_r)_r partitions A, Aw_r is not
 * page-aligned. */
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

/* Ar_r := [startRead(A, r), endRead(A, r)[ is the part of A that is stored on
 * node r. (Ar_r)_r covers A, but is not a partition. Ar_r is the minimal
 * page-aligned superset of Aw_r. */
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

/* Ap_r := [startPartition(A, r), endPartition(A, r)[ is a subset of Ar_r such
 * that (Ap_r)_r is a partitioning of \bigcup_r (Ar_r)_r (which is A + some
 * dummy entries at the last page of the allocation). Note, Ap_r may be
 * empty! */
static inline uintptr_t startPartition(Allocation *alloc, unsigned int rank)
{
    return (endRead(alloc, rank - 1) == startRead(alloc, rank) + Shray_Pagesz) ?
        startRead(alloc, rank) + Shray_Pagesz :
        startRead(alloc, rank);
}

static inline uintptr_t endPartition(Allocation *alloc, unsigned int rank)
{
    return endRead(alloc, rank);
}

/* Frees [start, end[. start, end need to be Shray_Pagesz-aligned */
static inline void freeRAM(uintptr_t start, uintptr_t end)
{
    if (start >= end) return;

    DBUG_PRINT("We free [%p, %p[", (void *)start, (void *)end);

    MUNMAP_SAFE((void *)start, end - start);
    MMAP_FIXED_SAFE((void *)start, end - start, PROT_NONE);
}

/* Simple binary search, assumes heap.allocs is sorted. */
static int findAllocIndex(void *segfault)
{
    int low = 0;
    int high = heap.numberOfAllocs - 1;
    int middle = -1;

    while (low <= high) {
        middle = (low + high) / 2;
        uintptr_t location = heap.allocs[middle].location;
        if (location + heap.allocs[middle].size <= (uintptr_t)segfault) {
            low = middle + 1;
        } else if ((uintptr_t)segfault < location) {
            high = middle - 1;
        } else {
            break;
        }
    }

    if (!inRange(segfault, middle)) {
        fprintf(stderr, "%p is not in an allocation.\n", segfault);
        gasnet_exit(1);
    }

    return middle;
}

static Allocation *findAlloc(void *segfault)
{
    return heap.allocs + findAllocIndex(segfault);
}

/* Reset the pages used by the cache.
 * Assumes start is page aligned. */
static void evictCacheEntry(Allocation *alloc, uintptr_t start, size_t pages)
{
    size_t index = (start - alloc->location) / Shray_Pagesz;
    size_t size = pages * Shray_Pagesz;
    BitmapSetZeroes(alloc->local, index, index + pages);

    DBUG_PRINT("evictCacheEntry: we free page %zu", index);
    freeRAM(start, start + size);
}

/* Assumes both pages are page-aligned. */
static inline int isNextPage(void *x, void *y)
{
    return (uintptr_t)y - (uintptr_t)x == Shray_Pagesz;
}

static void handlePageFault(void *address)
{
    uintptr_t roundedAddress = roundDownPage((uintptr_t)address);

    Allocation *alloc = findAlloc((void *)roundedAddress);

    size_t pageNumber = (roundedAddress - alloc->location) / Shray_Pagesz;
    DBUG_PRINT("Page %zu", pageNumber);

    if (ringbuffer_full(alloc->autoCaches)) {
        cache_entry_t *entry = ringbuffer_front(alloc->autoCaches);
        DBUG_PRINT("Cache buffer is full, evicting %p", entry->start);
        evictCacheEntry(alloc, (uintptr_t)entry->start, 1);
    }

    uintptr_t difference = roundedAddress - alloc->location;
    unsigned int owner = difference / alloc->bytesPerBlock;

    DBUG_PRINT("Segfault is owned by node %d.", owner);

    gasnet_get(alloc->shadowPage, owner, (void *)roundedAddress, Shray_Pagesz);

    MREMAP_MOVE((void *)roundedAddress, alloc->shadowPage, Shray_Pagesz);
    MMAP_FIXED_SAFE(alloc->shadowPage, Shray_Pagesz, PROT_READ | PROT_WRITE);

    DBUG_PRINT("We set page %zu to locally available.", pageNumber);

    BitmapSetOne(alloc->local, pageNumber);
    ringbuffer_add(alloc->autoCaches, alloc, (void*)roundedAddress);
}

static inline void atomic_clear(bool *p)
{
        __atomic_clear(p, __ATOMIC_SEQ_CST);
}

static inline bool atomic_test_set(void *p)
{
        return __atomic_test_and_set(p, __ATOMIC_SEQ_CST);
}

static inline void lock()
{
    while (atomic_test_set(&thread_lock));
}

static inline void unlock()
{
    atomic_clear(&thread_lock);
}

static inline int pageFaultHandled(uintptr_t address)
{
    uintptr_t roundedAddress = roundDownPage(address);
    Allocation *alloc = findAlloc((void *)roundedAddress);
    size_t pageNumber = (roundedAddress - alloc->location) / Shray_Pagesz;
    return BitmapCheck(alloc->local, pageNumber);
}

static void SegvHandler(int sig, siginfo_t *si, void *unused)
{
    (void)sig;
    (void)unused;

    void *address = si->si_addr;

    DBUG_PRINT("Segfault %p", address);

    lock();
    SEGFAULTCOUNT;
    if (!pageFaultHandled((uintptr_t)address)) {
        handlePageFault(address);
    }
    unlock();
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
}

/* Is linear in the number of allocations */
static void ShrayResetCache(Allocation *alloc)
{
    freeRAM(alloc->location, startRead(alloc, Shray_rank));
    freeRAM(endRead(alloc, Shray_rank), alloc->location + alloc->size);

    ringbuffer_reset(alloc->autoCaches);
    BitmapReset(alloc->local);
}

/*****************************************************
 * Shray functionality
 *****************************************************/

void ShrayInit(int *argc, char ***argv)
{
    GASNET_SAFE(gasnet_init(argc, argv));
    /* Must be built with GASNET_SEGMENT_EVERYTHING, so these arguments are
     * ignored. */
    GASNET_SAFE(gasnet_attach(NULL, 0, 4096, 0));

    Shray_size = gasnet_nodes();
    Shray_rank = gasnet_mynode();

    ShrayOutput = (Shray_rank == 0);

    Shray_SegfaultCounter = 0;
    Shray_BarrierCounter = 0;

    if(gethostname(ShrayHost, HOSTNAME_LENGTH) != 0) {
        ShrayHost[0] = '\0';
    }

    char *cacheLineEnv = getenv("SHRAY_CACHELINE");
    if (cacheLineEnv == NULL) {
        Shray_CacheLineSize = 1;
    } else {
        Shray_CacheLineSize = atol(cacheLineEnv);
    }

    int pagesz = sysconf(_SC_PAGE_SIZE);
    if (pagesz == -1) {
        perror("Querying system page size failed.");
    }

    Shray_Pagesz = (size_t)pagesz * Shray_CacheLineSize;

    heap.size = sizeof(Allocation);
    heap.numberOfAllocs = 0;
    MALLOC_SAFE(heap.allocs, sizeof(Allocation));

    char *cacheSizeEnv = getenv("SHRAY_CACHEFACTOR");
    if (cacheSizeEnv == NULL) {
        Shray_CacheAllocFactor = 1;
    } else {
        Shray_CacheAllocFactor = strtod(cacheSizeEnv, NULL);
    }

    registerHandlers();
}

void *ShrayMalloc(size_t firstDimension, size_t totalSize)
{
    lock();

    void *location;

    /* For the segfault handler, we need the start of each allocation to be
     * Shray_Pagesz-aligned. We cheat a little by making it possible for this
     * to be multiple system-pages. So we mmap an extra page at the start and
     * end, and then move the pointer up. */
    if (Shray_rank == 0) {
        void *mmapAddress;
        MMAP_SAFE(mmapAddress, NULL, totalSize + 2 * Shray_Pagesz, PROT_NONE);
        location = (void *)roundUpPage((uintptr_t)mmapAddress);
        DBUG_PRINT("mmapAddress = %p, allocation start = %p",
                mmapAddress, location);
    }

    /* Broadcast location to the other nodes. */
    gasnet_coll_broadcast(gasnete_coll_team_all, &location, 0, &location,
            sizeof(void *), GASNET_COLL_DST_IN_SEGMENT);

    if (Shray_rank != 0) {
        MMAP_FIXED_SAFE(location, totalSize + Shray_Pagesz, PROT_NONE);
    }

    void *shadowPage;
    MMAP_SAFE(shadowPage, NULL, Shray_Pagesz, PROT_WRITE);
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
    size_t bytesPerBlock = roundUp(firstDimension, Shray_size) *
        bytesPerLatterDimensions;

    alloc->firstDimension = firstDimension;
    alloc->location = (uintptr_t)location;
    alloc->size = totalSize;
    alloc->bytesPerBlock = bytesPerBlock;
    alloc->shadowPage = shadowPage;

    size_t segmentLength = endRead(alloc, Shray_rank) -
                           startRead(alloc, Shray_rank);

    DBUG_PRINT("Made a DSM allocation [%p, %p[, of which \t\tAw = [%p, %p[, "
            "\t\tAr = [%p, %p[,\t\tAp = [%p, %p[.",
            location, (void *)((uintptr_t)location + totalSize),
            (void *)startWrite(alloc, Shray_rank),
            (void *)endWrite(alloc, Shray_rank),
            (void *)startRead(alloc, Shray_rank),
            (void *)endRead(alloc, Shray_rank),
            (void *)startPartition(alloc, Shray_rank),
            (void *)endPartition(alloc, Shray_rank));

    MPROTECT_SAFE((void *)startRead(alloc, Shray_rank), segmentLength,
            PROT_READ | PROT_WRITE);

    alloc->local = BitmapCreate(roundUp(totalSize, Shray_Pagesz));

    size_t cacheEntries = min(1, segmentLength / Shray_Pagesz *
                                            Shray_CacheAllocFactor);
    alloc->autoCaches = ringbuffer_alloc(cacheEntries);
    if (!alloc->autoCaches) {
        fprintf(stderr, "[node %d]: Could not allocate autocache", Shray_rank);
        gasnet_exit(1);
    }
    DBUG_PRINT("Allocated %zu automatic cache entries (%zu)", cacheEntries,
            totalSize / Shray_Pagesz);

    gasnetBarrier();

    unlock();
    return location;
}

size_t ShrayStart(void *array)
{
    Allocation *alloc = findAlloc(array);
    size_t result = Shray_rank * roundUp(alloc->firstDimension, Shray_size);
    return result;
}

size_t ShrayEnd(void *array)
{
    Allocation *alloc = findAlloc(array);
    size_t firstDimension = alloc->firstDimension;
    size_t result = (Shray_rank == Shray_size - 1) ? firstDimension :
        (Shray_rank + 1) * roundUp(firstDimension, Shray_size);
    return result;
}

static void UpdateLeftPage(Allocation *alloc)
{
    uintptr_t firstPage = startRead(alloc, Shray_rank);
    /* Rank s has to send
     * [start, end[ := Aw_s \cap [firstPage, firstPage + Shray_Pagesz[ to
     * rank t whenever [start, end[ \cap Ar_t is non-empty. */
    uintptr_t start = max(startWrite(alloc, Shray_rank), firstPage);
    uintptr_t end = min(endWrite(alloc, Shray_rank), firstPage + Shray_Pagesz);

    int rank = Shray_rank - 1;
    for (; rank >= 0 && endRead(alloc, rank) - 1 >= start; rank--) {
        DBUG_PRINT("Put [%p, %p[ into node %u",
                (void *)start, (void *)end, rank);
        gasnet_put_bulk(rank, (void *)start, (void *)start, end - start);
    }
}

static void UpdateRightPage(Allocation *alloc)
{
    uintptr_t lastPage = endRead(alloc, Shray_rank) - Shray_Pagesz;
    /* Rank s has to send
     * [start, end[ := Aw_s \cap [lastPage, lastPage + Shray_Pagesz[ to
     * rank t whenever [start, end[ \cap Ar_t is non-empty. */
    uintptr_t start = max(startWrite(alloc, Shray_rank), lastPage);
    uintptr_t end = min(endWrite(alloc, Shray_rank), lastPage + Shray_Pagesz);

    unsigned int rank = Shray_rank + 1;
    for (; rank < Shray_size && startRead(alloc, rank) < end; rank++) {
        DBUG_PRINT("Put [%p, %p[ into node %u",
                (void *)start, (void *)end, rank);
        gasnet_put_bulk(rank, (void *)start, (void *)start, end - start);
    }
}

void ShraySync(void *unused, ...)
{
    lock();

    void *array;
    va_list ap;
    va_start(ap, unused);

    /* Note that in shray2.h the ShraySync macro appends a NULL after the
     * arguments, and a NULL before the arguments. */
    while ((array = va_arg(ap, void *)) != NULL) {
        Allocation *alloc = findAlloc(array);
        UpdateLeftPage(alloc);
        UpdateRightPage(alloc);
        ShrayResetCache(alloc);
        DBUG_PRINT("We are updating pages for %p", array);
    }

    va_end(ap);

    gasnet_wait_syncnbi_puts();

    /* So no one reads from us before the communications are completed. */
    gasnetBarrier();
    unlock();
}

void ShrayFree(void *address)
{
    lock();
    DBUG_PRINT("ShrayFree: we free %p.", address);

    /* So everyone has finished reading before we free the array. */
    gasnetBarrier();

    int index = findAllocIndex(address);
    Allocation *alloc = heap.allocs + index;
    ringbuffer_reset(alloc->autoCaches);
    /* We leave potentially two pages mapped due to the alignment in
     * ShrayMalloc, but who cares. */
    MUNMAP_SAFE((void *)alloc->location, alloc->size);
    BitmapFree(alloc->local);
    heap.numberOfAllocs--;
    while ((unsigned)index < heap.numberOfAllocs) {
        heap.allocs[index] = heap.allocs[index + 1];
        index++;
    }
    unlock();
}

void ShrayReport(void)
{
    lock();
    fprintf(stderr, "Shray report P(%d) on %s: %zu segfaults, %zu barriers, "
            "%zu bytes communicated.\n", Shray_rank, ShrayHost,
            Shray_SegfaultCounter, Shray_BarrierCounter,
            Shray_SegfaultCounter * Shray_Pagesz);
    unlock();
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
    DBUG_PRINT("Terminating with code %d", exit_code);
    if (exit_code == 0) {
        gasnetBarrier();
    }
    gasnet_exit(exit_code);
}

void * ShrayWriteBuf(void *address, size_t size)
{
    if (((uintptr_t)address % Shray_Pagesz != 0) ||
         (size % Shray_Pagesz != 0)) {
        fprintf(stderr, "ShrayWriteBuf: [address, address + size[ must "
                        "have alignment %zu\n", Shray_Pagesz);
    }

    void *result;
    MMAP_SAFE(result, NULL, size, PROT_WRITE);

    return result;
}

void ShrayCommit(void *buf, void *address, size_t size)
{
    lock();
    MREMAP_MOVE(address, buf, size);
    unlock();
}

void ShrayUncommit(void *address, size_t size)
{
    lock();
    freeRAM((uintptr_t)address, (uintptr_t)address + size);
    unlock();
}
