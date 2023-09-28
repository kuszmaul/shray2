#pragma once

#include "bitmap.h"
#include "ringbuffer.h"
#include <stdio.h>
#include <stdarg.h>
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

/* A single allocation in the heap. */
typedef struct Allocation {
    uintptr_t location;
    size_t size;
    size_t firstDimension;
    /* The number of bytes owned by each node except the last one. */
    size_t bytesPerBlock;
    Bitmap *local;
    /* Cache for segfaults. */
    ringbuffer_t *autoCaches;
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
 * Global variables
 **************************************************/

extern unsigned int Shray_rank;
extern unsigned int Shray_size;
extern size_t Shray_SegfaultCounter;
extern size_t Shray_BarrierCounter;
extern size_t Shray_Pagesz;
extern size_t Shray_CacheLineSize;
extern double Shray_CacheAllocFactor;
extern Heap heap;

/**************************************************
 * Error handling
 **************************************************/

#define GASNET_SAFE(fncall)                                                   \
    {                                                                         \
        int retval;                                                           \
        if ((retval = fncall) != GASNET_OK) {                                 \
            printf("Error during GASNet call\n");                             \
            gasnet_exit(1);                                                   \
        }                                                                     \
    }

#define MPROTECT_SAFE(addr, len, prot)                                        \
    {                                                                         \
        DBUG_PRINT("mprotect: protected [%p, %p[ to %s", addr,                \
                (void *)((uintptr_t)addr + len), #prot);                      \
        if (mprotect(addr, len, prot) != 0) {                                 \
            fprintf(stderr, "%s:%d [node %d]: ",                              \
                    __FILE__, __LINE__, Shray_rank);                          \
            perror("mprotect failed");                                        \
            gasnet_exit(1);                                                   \
        }                                                                     \
    }

/* Moves [source, source + size[ to [dest, dest + size[ */
#define MREMAP_MOVE(dest, source, size)                                       \
    {                                                                         \
        DBUG_PRINT("mremap: Moved [%p, %p[ to [%p, %p[", source,              \
                (void *)((uintptr_t)source + size), dest,                     \
                (void *)((uintptr_t)dest + size));                            \
        void *dummy;                                                          \
        dummy = mremap(source, size, size, MREMAP_MAYMOVE | MREMAP_FIXED,     \
                dest);                                                        \
        if (dummy == MAP_FAILED) {                                            \
            fprintf(stderr, "%s:%d [node %d]: ",                              \
                    __FILE__, __LINE__, Shray_rank);                          \
            perror("mremap failed");                                          \
            gasnet_exit(1);                                                   \
        }                                                                     \
    }

#define MMAP_SAFE(variable, address, length, prot)                            \
    {                                                                         \
        variable = mmap(address, length, prot, MAP_ANONYMOUS | MAP_PRIVATE,   \
                -1, 0);                                                       \
        DBUG_PRINT("mmap: %p = mmap(%p, %zu, %s, "                            \
                   "MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);",                    \
                    variable, address, length, #prot);                        \
        if (variable == MAP_FAILED) {                                         \
            fprintf(stderr, "%s:%d [node %d]: ",                              \
                    __FILE__, __LINE__, Shray_rank);                          \
            perror("mmap failed");                                            \
            gasnet_exit(1);                                                   \
        }                                                                     \
    }

#define MMAP_FIXED_SAFE(address, length, prot)                                \
    {                                                                         \
        void *success = mmap(address, length, prot,                           \
                MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);              \
        DBUG_PRINT("mmap: %p = mmap(%p, %zu, %s, "                            \
                "MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);",           \
                success, address, length, #prot);                             \
        if (success == MAP_FAILED) {                                          \
            fprintf(stderr, "%s:%d [node %d]: ",                              \
                    __FILE__, __LINE__, Shray_rank);                          \
            perror("mmap failed");                                            \
            gasnet_exit(1);                                                   \
        }                                                                     \
    }

#define MUNMAP_SAFE(address, length)                                          \
    {                                                                         \
        if (munmap(address, length) == -1) {                                  \
            fprintf(stderr, "[node %d] unmapping [%p, %p[\n", Shray_rank,     \
                    address, (void *)((uintptr_t)address + length));          \
            perror("munmap failed");                                          \
        }                                                                     \
    }

#define MALLOC_SAFE(variable, size)                                           \
    {                                                                         \
        variable = malloc(size);                                              \
        if (variable == NULL) {                                               \
            fprintf(stderr, "%s:%d [node %d]: ",                              \
                    __FILE__, __LINE__, Shray_rank);                          \
            gasnet_exit(1);                                                   \
        }                                                                     \
    }

#define REALLOC_SAFE(variable, size)                                          \
    {                                                                         \
        variable = realloc(variable, size);                                   \
        if (variable == NULL) {                                               \
            fprintf(stderr, "%s:%d [node %d]: ",                              \
                    __FILE__, __LINE__, Shray_rank);                          \
            gasnet_exit(1);                                                   \
        }                                                                     \
    }

static inline void MadviseDontNeedSafe(void *addr, size_t size) {
  int r = madvise(addr, size, MADV_DONTNEED);
  DBUG_PRINT("madvise: %d = madvise(%p, %zu, MADV_DONTNEED);",
             r, addr, size);
  if (r != 0) {
    perror("madvise failed");
    gasnet_exit(1);
  }
}
