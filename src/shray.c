/* Distribution: 1d it is a block distribution on the bytes, so 
 * phi_s(k) = k + s * roundUp(n, p), in the higher dimensional case, 
 * we distribute blockwise along the first dimension. */

#include "../include/shray.h"

/*****************************************************
 * Global variable declarations. 
 *****************************************************/

static Cache cache;
static Allocation *heap;

static int Shray_rank;
static int Shray_size;
static size_t segfaultCounter = 0;
static size_t barrierCounter = 0;

/*****************************************************
 * Helper functions
 *****************************************************/

int inRange(void *address, Allocation* alloc)
{
    return ((uintptr_t)alloc->location <= (uintptr_t)address) && 
        ((uintptr_t)address < (uintptr_t)alloc->location + alloc->size);
}

/* segfault is rounded to page boundary */
RDMA findOwner(void *segfault)
{
    RDMA rdma;

    /* We advance through the allocations, until we find the one containing our 
     * segfault. */
    Allocation *current = heap;

    while (!inRange(segfault, current) && current != NULL) {
        current = current->next;
    }

    if (current == NULL) {
            printf("Segfault (%p) outside of DSM area\n", segfault);
            MPI_Abort(MPI_COMM_WORLD, 5);
    }

    uintptr_t difference = (uintptr_t)segfault - (uintptr_t)(current->location);

    rdma.owner = difference / current->bytesPerBlock;
    rdma.offset = difference % current->bytesPerBlock + 
        (rdma.owner * current->bytesPerBlock) % PAGESIZE;

    rdma.win = current->win;

    return rdma;
}

void SegvHandler(int sig, siginfo_t *si, void *unused)
{
    segfaultCounter++;
    void *address = si->si_addr;

    /* Evict the previous cache line. */
    if (cache.addresses[cache.firstIn] != NULL) {
        DEBUG_PRINT_COMM(("P(%d): evict %p", Shray_rank, cache.addresses[cache.firstIn]));
        MPROTECT_SAFE(mprotect(cache.addresses[cache.firstIn], PAGESIZE, PROT_NONE));
    }

    /* Admit the current cache line. */
    cache.addresses[cache.firstIn] = (void *)((uintptr_t)address / PAGESIZE * PAGESIZE);
    MPROTECT_SAFE(mprotect(cache.addresses[cache.firstIn], PAGESIZE, PROT_READ | PROT_WRITE));

    /* Copy the remote page */
    RDMA rdma = findOwner(cache.addresses[cache.firstIn]);

    DEBUG_PRINT_COMM(("P(%d): Get %p (offset %zu) from P(%d)\n", Shray_rank, 
                cache.addresses[cache.firstIn], rdma.offset, rdma.owner));

    MPI_SAFE(MPI_Win_lock(MPI_LOCK_SHARED, rdma.owner, 0, *rdma.win));

    MPI_SAFE(MPI_Get(cache.addresses[cache.firstIn], PAGESIZE, MPI_BYTE, rdma.owner, 
            rdma.offset, PAGESIZE, MPI_BYTE, *rdma.win));

    MPI_SAFE(MPI_Win_unlock(rdma.owner, *rdma.win));
    DEBUG_PRINT_COMM(("P(%d): Get completed\n", Shray_rank));

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
        MPI_Abort(MPI_COMM_WORLD, 6);
    }
}

Allocation *createAllocation(void)
{
    Allocation *result = malloc(sizeof(Allocation));
    result->win = malloc(sizeof(MPI_Win));

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
    MPI_Init(NULL, NULL);

    MPI_Comm_size(MPI_COMM_WORLD, &Shray_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &Shray_rank);

    heap = createAllocation();

    cache.firstIn = 0;
    cache.numberOfLines = cacheSize / PAGESIZE;
    if ((cache.addresses = malloc(cache.numberOfLines * sizeof(void *))) == NULL) {
        perror("Allocating cache addresses has failed\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    for (size_t i = 0; i < cache.numberOfLines; i++) {
        cache.addresses[i] = NULL;
    }

    registerHandler();
}

void *ShrayMalloc(size_t firstDimension, size_t totalSize)
{
    Allocation *alloc = createAllocation();

    alloc->size = totalSize;

    MMAP_SAFE(alloc->location, mmap(NULL, alloc->size, PROT_NONE, MAP_ANONYMOUS |
                MAP_PRIVATE, -1, 0));

    /* We distribute blockwise over the first dimension. */
    size_t bytesPerLatterDimensions = alloc->size / firstDimension;
    alloc->bytesPerBlock = roundUp(firstDimension, Shray_size) * bytesPerLatterDimensions;

    /* Protect all pages that we do not own, and register what we do. */
    size_t firstPage = Shray_rank * alloc->bytesPerBlock / PAGESIZE;
    size_t lastPage = (Shray_rank == Shray_size - 1) ? alloc->size / PAGESIZE :
        (Shray_rank + 1) * alloc->bytesPerBlock / PAGESIZE;
    size_t segmentSize = (lastPage - firstPage + 1) * PAGESIZE;
    if (segmentSize == 0) {
        fprintf(stderr, "A processor has no data\n");
        MPI_Abort(MPI_COMM_WORLD, 5);
    }

    void *start = (void *)((uintptr_t)alloc->location + firstPage * PAGESIZE);
    MPROTECT_SAFE(mprotect(start, segmentSize, PROT_READ | PROT_WRITE));
    MPI_SAFE(MPI_Win_create(start, segmentSize, 1, MPI_INFO_NULL, MPI_COMM_WORLD, alloc->win));

    /* Insert a new allocation */
    heap = insertAtHead(heap, alloc);

    MPI_SAFE(MPI_Barrier(MPI_COMM_WORLD));

    barrierCounter++;

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
    MPI_SAFE(MPI_Barrier(MPI_COMM_WORLD));
    Allocation *current = heap;

    while (current != NULL && current->location != array) {
        current = current->next;
    }

    if (current == NULL) {
        perror("address is not the start of a DSM allocation.");
    }

    /* Synchronise in case the first or last page is co-owned with someone else. */
    size_t firstPage = Shray_rank * current->bytesPerBlock / PAGESIZE;
    size_t lastPage = (Shray_rank == Shray_size - 1) ? 
        current->size / PAGESIZE :
        (Shray_rank + 1) * current->bytesPerBlock / PAGESIZE;

    size_t firstByte = Shray_rank * current->bytesPerBlock;
    size_t lastByte = (Shray_rank + 1) * current->bytesPerBlock - 1;

    /* Get the last page of the previous rank, and copy its contents to our copy of that 
     * page. */
    if ((firstByte % PAGESIZE != 0) && (Shray_rank != 0)) {
        size_t theirFirstPage = (Shray_rank - 1) * current->bytesPerBlock / 
            PAGESIZE;
        size_t theirLastPage = (Shray_rank * current->bytesPerBlock - 1) / 
            PAGESIZE;

        void *destination = (void *)((uintptr_t)(current->location) + 
                firstPage * PAGESIZE);

        /* From this byte on, we own it, before this byte the previous rank does. */
        size_t pageBoundary = Shray_rank * current->bytesPerBlock % PAGESIZE;

        MPI_SAFE(MPI_Win_lock(MPI_LOCK_SHARED, Shray_rank - 1, 0, *(current->win)));
    
        MPI_SAFE(MPI_Get(destination, pageBoundary, MPI_BYTE, Shray_rank - 1, 
                    (theirLastPage - theirFirstPage) * PAGESIZE, 
                    pageBoundary, MPI_BYTE, *(current->win)));
    
        MPI_SAFE(MPI_Win_unlock(Shray_rank - 1, *(current->win)));
    }

    /* Get the first page of the next rank, and copy its contents to our copy of that 
     * page. */
    if ((lastByte % PAGESIZE != PAGESIZE - 1) && 
            (Shray_rank != Shray_size - 1)) {

        void *destination = (void *)((uintptr_t)current->location + 
                (Shray_rank + 1) * current->bytesPerBlock);

        /* Before this byte, we own it, after this byte the next rank does. */
        size_t pageBoundary = lastByte % PAGESIZE;

        MPI_SAFE(MPI_Win_lock(MPI_LOCK_SHARED, Shray_rank + 1, 0, *(current->win)));
    
        MPI_SAFE(MPI_Get(destination, PAGESIZE - pageBoundary - 1, MPI_BYTE, 
                    Shray_rank + 1, pageBoundary + 1, PAGESIZE - 
                    pageBoundary - 1, MPI_BYTE, *(current->win)));
    
        MPI_SAFE(MPI_Win_unlock(Shray_rank + 1, *(current->win)));
    }

    /* Protect only the pages we own. */
    MPROTECT_SAFE(mprotect(current->location, current->size, PROT_NONE));

    size_t segmentSize = (lastPage - firstPage + 1) * PAGESIZE;

    void *start = (void *)((uintptr_t)current->location + firstPage * PAGESIZE);
    MPROTECT_SAFE(mprotect(start, segmentSize, PROT_READ | PROT_WRITE));

    MPI_SAFE(MPI_Barrier(MPI_COMM_WORLD));
    barrierCounter += 2;
}

void ShrayFree(void *address)
{
    MPI_SAFE(MPI_Barrier(MPI_COMM_WORLD));
    /* indirect iterates through the links of the nodes, e.g. the pointers
     * to the next allocation. Double pointer is necessary because there is no 
     * link to heap (the head of the list). */
    Allocation **indirect = &heap; 

    while ((*indirect)->location != address) {
        indirect = &(*indirect)->next;
    }

    if (*indirect == NULL) {
        fprintf(stderr, "Illegal dsm free\n");
        MPI_Abort(MPI_COMM_WORLD, 2);
    } else {
        MPI_SAFE(MPI_Win_free((*indirect)->win));
        munmap(address, (*indirect)->size);
        Allocation *deleteThis = *indirect;
        *indirect = (*indirect)->next;
        free(deleteThis);
    }
    MPI_SAFE(MPI_Barrier(MPI_COMM_WORLD));
    barrierCounter += 2;
}

void ShrayReport(void)
{
    fprintf(stderr, 
            "Shray report P(%d): %zu segfaults, %zu barriers, %zu bytes communicated.\n",
            Shray_rank, segfaultCounter, barrierCounter, segfaultCounter * PAGESIZE);
}

void ShrayFinalize(void)
{
    MPI_SAFE(MPI_Finalize());
}
