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

/* FIXME we may want to be able to synchronize for each prefetch separately. This 
 * can be done with gasnet_wait_syncnb_all, but we need to add handles to this. */
typedef struct Prefetch {
    /* In practice we round this down to the page boundary. */
    void *location;
    struct Prefetch *next;
    uintptr_t block1start;
    uintptr_t block1end;
    uintptr_t block2start;
    uintptr_t block2end;
} Prefetch;
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

#define MUNMAP_SAFE(address, length)                                                    \
    {                                                                                   \
        if (munmap(address, length) == -1) {                                            \
            fprintf(stderr, "[node %d] unmapping [%p, %p[\n", Shray_rank, address,      \
                    (void *)((uintptr_t)address + length));                             \
            perror("munmap failed");                                                    \
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
