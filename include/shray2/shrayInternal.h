#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <gasnet.h>
#include <gasnet_coll.h>
#include <sys/mman.h>
#include <sys/time.h>

/**************************************************
 * Data structures
 **************************************************/

typedef struct {
    /* addresses[i] is a pointer to the virtual page 
     * stored in the ith cache line, or a reserved 
     * piece of memory in case the cache line is empty. */
    void **addresses;
    /* The position of the first cache line admitted. */
    size_t firstIn;
    /* This determines the size of our cache. */
    size_t numberOfLines;
    /* True when we have used the last cache line for the first time. Optimisation
     * for resetCache in low-communication applications. */ 
    bool allUsed;
} Cache;

/* A single allocation in the heap. */
typedef struct Allocation {
    void *location; 
    size_t size;
    /* The number of bytes owned by each node except the last one. */
    size_t bytesPerBlock;
} Allocation;

typedef struct Heap {
    /* size of allocs */
    size_t size;    
    /* Array of allocs, sorted from low to high in location. */
    Allocation *allocs;
    /* Number of actual allocations in the allocs */
    unsigned int numberOfAllocs;
} Heap;

/**************************************************
 * Error handling
 **************************************************/

#define GASNET_SAFE(fncall)                                                             \
    {                                                                                   \
        int retval;                                                                     \
        if ((retval = fncall) != GASNET_OK) {                                           \
            printf("Error during GASNet call\n");                                       \
            ShrayFinalize(1);                                                           \
        }                                                                               \
    }

#define MPROTECT_SAFE(addr, len, prot)                                                  \
    {                                                                                   \
        if (mprotect(addr, len, prot) != 0) {                                           \
            fprintf(stderr, "Line %d, node [%d]: ", __LINE__, Shray_rank);              \
            perror("mprotect failed");                                                  \
            ShrayFinalize(1);                                                           \
        }                                                                               \
    }

#define MREMAP_SAFE(variable, fncall)                                                   \
    {                                                                                   \
        variable = fncall;                                                              \
        if (variable == MAP_FAILED) {                                                   \
            fprintf(stderr, "Line %d, node [%d]: ", __LINE__, Shray_rank);              \
            perror("mremap failed");                                                    \
            ShrayFinalize(1);                                                           \
        }                                                                               \
    }

#define MMAP_SAFE(variable, fncall)                                                     \
    {                                                                                   \
        variable = fncall;                                                              \
        if (variable == MAP_FAILED) {                                                   \
            fprintf(stderr, "Line %d, node [%d]: ", __LINE__, Shray_rank);              \
            perror("mmap failed");                                                      \
            ShrayFinalize(1);                                                           \
        }                                                                               \
    }

#define MALLOC_SAFE(variable, size)                                                     \
    {                                                                                   \
        variable = malloc(size);                                                        \
        if (variable == NULL) {                                                         \
            fprintf(stderr, "Line %d, node [%d]: malloc failed with size %zu\n",        \
                    __LINE__, Shray_rank, size);                                        \
            ShrayFinalize(1);                                                           \
        }                                                                               \
    }

#define REALLOC_SAFE(variable, size)                                                    \
    {                                                                                   \
        variable = realloc(variable, size);                                             \
        if (variable == NULL) {                                                         \
            fprintf(stderr, "Line %d, node [%d]: realloc failed with size %zu\n",       \
                    __LINE__, Shray_rank, size);                                        \
            ShrayFinalize(1);                                                           \
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
 * Graphing segfaults
 **************************************************/

#ifdef GRAPH 
    #define PROFILE
    #define GRAPH_SEGV(segfault, segvNo)                                          \
        {                                                                         \
            if (Shray_rank == 0) {                                                \
                fprintf(stderr, "%zu, %zu\n", segvNo, segfault / ShrayPagesz);    \
            }                                                                     \
        }
#else
    #define GRAPH_SEGV(segfault, segvNo)
#endif

/**************************************************
 * Profiling
 **************************************************/

#ifdef PROFILE
    #define BARRIERCOUNT barrierCounter++;
    #define SEGFAULTCOUNT segfaultCounter++;
    #define PREFETCHMISS prefetchMissCounter++;
#else
    #define BARRIERCOUNT
    #define SEGFAULTCOUNT
    #define PREFETCHMISS
#endif
