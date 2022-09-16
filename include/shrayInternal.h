#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>
#define __USE_GNU
#include <sys/mman.h>

/**************************************************
 * Data structures
 **************************************************/

typedef struct {
    /* Pointer to our cache lines. Note: can be deduced from the 
     * first pointer, but we need lValues for remapping. */
    void **cacheLines;
    /* We do not remap this, so we do not forget where our 
     * registered region is. */
    void **cacheLinesCopy;
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
    void *location; size_t size;
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

/**************************************************
 * Error handling
 **************************************************/

#define MPI_SAFE(fncall)                                                                \
    {                                                                                   \
        if (fncall != MPI_SUCCESS) {                                                    \
            perror("MPI call unsuccessfull\n");                                         \
            MPI_Abort(MPI_COMM_WORLD, 1);                                               \
        }                                                                               \
    }

#define MPROTECT_SAFE(fncall)                                                           \
    {                                                                                   \
        if (fncall != 0) {                                                              \
            perror("mprotect failed");                                                  \
            MPI_Abort(MPI_COMM_WORLD, 1);                                               \
        }                                                                               \
    }

#define MREMAP_SAFE(variable, fncall)                                                   \
    {                                                                                   \
        variable = fncall;                                                              \
        if (variable == MAP_FAILED) {                                                   \
            perror("mremap failed");                                                    \
            MPI_Abort(MPI_COMM_WORLD, 1);                                               \
        }                                                                               \
    }

#define MMAP_SAFE(variable, fncall)                                                     \
    {                                                                                   \
        variable = fncall;                                                              \
        if (variable == MAP_FAILED) {                                                   \
            perror("mmap failed");                                                      \
            MPI_Abort(MPI_COMM_WORLD, 1);                                               \
        }                                                                               \
    }

/**************************************************
 * Debugging
 **************************************************/

#ifdef DEBUG
    #define DBUG_PRINT(fmt, ...)                                                        \
        fprintf(stderr, "\t[node %d]: " fmt "\n", Shray_rank, __VA_ARGS__);
#else
    #define DBUG_PRINT(fmt, ...)
#endif

/**************************************************
 * Profiling
 **************************************************/

#ifdef PROFILE
    #define BARRIERCOUNT barrierCounter++;
    #define SEGFAULTCOUNT segfaultCounter++;
#else
    #define BARRIERCOUNT
    #define SEGFAULTCOUNT
#endif
