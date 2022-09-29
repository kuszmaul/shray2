#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <gasnet.h>
#include <gasnet_coll.h>
#include <sys/mman.h>

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
    /* The address we are currently prefetching. */
    void *prefetch;
} Cache;

/* A single allocation in the heap. */
typedef struct Allocation {
    void *location; 
    size_t size;
    struct Allocation *next;
    /* The number of bytes owned by each node except the last one. */
    /* FIXME voor 1d arrays zou je halve elementen kunnen ownen. Of toch niet? Houden 
     * we daar in ShrayMalloc rekening mee? */
    size_t bytesPerBlock;
} Allocation;

/**************************************************
 * Error handling
 **************************************************/

#define GASNET_SAFE(fncall)                                                             \
    {                                                                                   \
        int retval;                                                                     \
        if ((retval = fncall) != GASNET_OK) {                                           \
            printf("Error during GASNet call\n");                                       \
            ShrayFinalize();                                                            \
        }                                                                               \
    }

#define MPROTECT_SAFE(fncall)                                                           \
    {                                                                                   \
        if (fncall != 0) {                                                              \
            fprintf(stderr, "Line %d: ", __LINE__);                                     \
            perror("mprotect failed");                                                  \
            ShrayFinalize();                                                             \
        }                                                                               \
    }

#define MREMAP_SAFE(variable, fncall)                                                   \
    {                                                                                   \
        variable = fncall;                                                              \
        if (variable == MAP_FAILED) {                                                   \
            perror("mremap failed");                                                    \
            fprintf(stderr, "Line %d: ", __LINE__);                                     \
            ShrayFinalize();                                                             \
        }                                                                               \
    }

#define MMAP_SAFE(variable, fncall)                                                     \
    {                                                                                   \
        variable = fncall;                                                              \
        if (variable == MAP_FAILED) {                                                   \
            perror("mmap failed");                                                      \
            fprintf(stderr, "Line %d: ", __LINE__);                                     \
            ShrayFinalize();                                                             \
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
