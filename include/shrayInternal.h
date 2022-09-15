#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <mpi.h>
#define __USE_GNU
#include <sys/mman.h>
#include "utils.h"

#define PAGESIZE 4096

typedef struct {
    /* addresses[i] is a pointer to the virtual page 
     * stored in the ith cache line. */
    void **addresses;
    /* The position of the first cache line admitted. */
    size_t firstIn;
    /* This determines the size of our cache. */
    size_t numberOfLines;
} Cache;

/* A single allocation in the heap. */
typedef struct Allocation {
    void *location;
    size_t size;
    /* We create a window on the part of the virtual address space that is physically 
     * stored on our node. */
    MPI_Win *win;
    struct Allocation *next;
    /* The number of bytes owned by each node except the last one. */
    size_t bytesPerBlock;
} Allocation;

/* For retrieving a remote page. */
typedef struct {
    int owner;
    size_t offset;
    MPI_Win *win;
} RDMA;

RDMA findOwner(void *segfault);

void SegvHandler(int sig, siginfo_t *si, void *unused);

void registerHandler(void);

Allocation *createAllocation(void);

Allocation *insertAtHead(Allocation *head, Allocation *newHead);
