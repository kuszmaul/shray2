#ifndef __UTILS_GUARD_
#define __UTILS_GUARD_

#include <stddef.h>

extern int DSM_size;
extern int DSM_rank;

#ifdef PROFILE
    #define DIAG(statement) statement
#else
    #define DIAG(statement)
#endif

#ifdef DEBUGDSM
    #define DEBUG_PRINT_DSM(x) printf x
#else
    #define DEBUG_PRINT_DSM(x)
#endif

#ifdef DEBUGHEAP
    #define DEBUG_PRINT_HEAP(x) printf x
#else
    #define DEBUG_PRINT_HEAP(x)
#endif

#ifdef DEBUGCOMM
    #define DEBUG_PRINT_COMM(x) printf x
#else
    #define DEBUG_PRINT_COMM(x)
#endif

#ifdef DEBUGSYNC
    #define DEBUG_PRINT_SYNC(fmt, ...) \
    fprintf(stderr, "\t[node %d] " fmt "\n", \
            DSM_rank, __VA_ARGS__);
#else
    #define DEBUG_PRINT_SYNC(fmt, ...) 
#endif

/* Takes care of error handling for MPI functions that
 * return a success value. */
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

/* Returns ceil(a / b) */
inline size_t roundUp(size_t a, size_t b)
{
    return (a + b - 1) / b;
}

/* For the 1d case */
inline size_t start1d(size_t totalElems, size_t elementSize)
{
    return DSM_rank * roundUp(totalElems, DSM_size);
}

inline size_t end1d(size_t totalElems, size_t elementSize)
{
    return (DSM_rank == DSM_size - 1) ? totalElems :
        (DSM_rank + 1) * roundUp(totalElems, DSM_size);
}

/* For 1d blocking algorithms: adjusts the blockSize downwards to make sure
 * it divides the number of elements per node. */
inline size_t twiddleBlockSize(size_t blockSize, size_t elemsPerNode)
{
    size_t potentialDivisor = 1;
    size_t divisor = 1;

    while (potentialDivisor <= blockSize) {
        if (elemsPerNode % potentialDivisor == 0) {
            divisor = potentialDivisor;
        }
        potentialDivisor++;
    }

    return divisor;
}
#endif
