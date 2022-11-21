#include "bitmap.h"
#include "ringbuffer.h"
#include "queue.h"
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
#include <shray2/shray.h>
#include "debug.h"

/**************************************************
 * Data structures
 **************************************************/

typedef struct {
    size_t usedMemory;
    size_t maximumMemory;
    ringbuffer_t *autoCaches;
    queue_t *prefetchCaches;
} Cache;

/* A single allocation in the heap. */
typedef struct Allocation {
    uintptr_t location;
    size_t size;
    /* The number of bytes owned by each node except the last one. */
    size_t bytesPerBlock;
    Bitmap *local;
    Bitmap *prefetched;
    /* We put all the prefetched stuff here until it is remapped to the proper position. */
    void *shadow;
    /* We need to know this when invalidating the cache for an allocation. */
    size_t usedMemory;
} Allocation;

typedef struct Heap {
    /* size of allocs */
    size_t size;
    /* Array of allocs, sorted from low to high in location. */
    Allocation *allocs;
    /* Number of actual allocations in the allocs */
    unsigned int numberOfAllocs;
} Heap;

/* We prefetch [start1, end1[ \cup [start2, end2[ which is part of allocation alloc. */
typedef struct PrefetchStruct {
    uintptr_t start1;
    uintptr_t end1;
    uintptr_t start2;
    uintptr_t end2;
    Allocation *alloc;
} PrefetchStruct;

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
        DBUG_PRINT("Protected [%p, %p[ to %s", addr, (void *)((uintptr_t)addr + len),   \
            #prot);                                                                     \
        if (mprotect(addr, len, prot) != 0) {                                           \
            fprintf(stderr, "Line %d, [node %d]: ", __LINE__, Shray_rank);              \
            perror("mprotect failed");                                                  \
            ShrayFinalize(1);                                                           \
        }                                                                               \
    }

/* Moves [source, source + size[ to [dest, dest + size[ */
#define MREMAP_MOVE(dest, source, size)                                                 \
    {                                                                                   \
        DBUG_PRINT("Moved [%p, %p[ to [%p, %p[", source,                                \
                (void *)((uintptr_t)source + size), dest,                               \
                (void *)((uintptr_t)dest + size));                                      \
        dest = mremap(source, size, size, MREMAP_MAYMOVE | MREMAP_FIXED, dest);         \
        if (dest == MAP_FAILED) {                                                       \
            fprintf(stderr, "Line %d, [node %d]: ", __LINE__, Shray_rank);              \
            perror("mremap failed");                                                    \
            ShrayFinalize(1);                                                           \
        }                                                                               \
    }

#define MMAP_SAFE(variable, fncall)                                                     \
    {                                                                                   \
        variable = fncall;                                                              \
        if (variable == MAP_FAILED) {                                                   \
            fprintf(stderr, "Line %d, [node %d]: ", __LINE__, Shray_rank);              \
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
            fprintf(stderr, "Line %d, [node %d]: malloc failed with size %zu\n",        \
                    __LINE__, Shray_rank, size);                                        \
            ShrayFinalize(1);                                                           \
        }                                                                               \
    }

#define REALLOC_SAFE(variable, size)                                                    \
    {                                                                                   \
        variable = realloc(variable, size);                                             \
        if (variable == NULL) {                                                         \
            fprintf(stderr, "Line %d, [node %d]: realloc failed with size %zu\n",       \
                    __LINE__, Shray_rank, size);                                        \
            ShrayFinalize(1);                                                           \
        }                                                                               \
    }


