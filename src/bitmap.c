#include "bitmap.h"
#include <stdio.h>
#include <sys/mman.h>
#include <immintrin.h>

/*******************************************
 * Error handling macros
 *******************************************/

#define MALLOC_SAFE(variable, size)                                           \
    {                                                                         \
        variable = malloc(size);                                              \
        if (variable == NULL) {                                               \
            fprintf(stderr, "Line %d, malloc failed with size %zu\n",         \
                    __LINE__, size);                                          \
            exit(EXIT_FAILURE);                                               \
        }                                                                     \
    }

#define MMAP_SAFE(variable, fncall)                                           \
    {                                                                         \
        variable = fncall;                                                    \
        if (variable == MAP_FAILED) {                                         \
            fprintf(stderr, "Line %d: ", __LINE__);                           \
            perror("mmap failed");                                            \
            exit(EXIT_FAILURE);                                               \
        }                                                                     \
    }

#define MUNMAP_SAFE(address, length)                                          \
    {                                                                         \
        if (munmap(address, length) == -1) {                                  \
            fprintf(stderr, "Unmapping [%p, %p[\n", (void *)address,          \
                    (void *)((uintptr_t)address + length));                   \
            perror("munmap failed");                                          \
        }                                                                     \
    }

/*******************************************
 * Internal helper functions
 *******************************************/

/* Returns ceil(a / b) */
static inline uintptr_t roundUp(uintptr_t a, uintptr_t b)
{
    return (a + b - 1) / b;
}

/* Returns n such that bitmap->bits[n] contains the index'th bit of the
 * bitmap. */
inline static size_t integer(size_t index)
{
    return index / 64;
}

/* Returns the p such that the pth bit of bitmap->bits[n] is the index'th bit
 * of the bitmap. */
inline static int bit(size_t index)
{
    return index % 64;
}

/*******************************************
 * Bitmap functionality
 *******************************************/

Bitmap *BitmapCreate(size_t size)
{
    Bitmap *bitmap;
    MALLOC_SAFE(bitmap, sizeof(Bitmap));

    /* We use mmap because it initializes to zero, and allocates lazily on
     * Linux. This saves memory in case a lot of the bitmap stays zero. */
    MMAP_SAFE(bitmap->bits, mmap(NULL, roundUp(size, 64) * sizeof(uint64_t),
                PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    bitmap->size = size;

    return bitmap;
}

void BitmapFree(Bitmap *bitmap)
{
    MUNMAP_SAFE(bitmap->bits, roundUp(bitmap->size, 64) * sizeof(uint64_t));
    free(bitmap);
}

/* Returns true iff the index'th bit of bitmap is 1. */
int BitmapCheck(Bitmap *bitmap, size_t index)
{
    return ((bitmap->bits[integer(index)] &
                (0x8000000000000000u >> bit(index))) != (uint64_t)0);
}

/* Sets [start, end[ to zero. */
void BitmapSetZeroes(Bitmap *bitmap, size_t start, size_t end)
{
    /* Number of bits to be set to zero in the first / last uint64_t, if they
     * are different uint64_ts. */
    int firstZeroes = 64 - bit(start);
    int lastZeroes = 1 + bit(end - 1);
    size_t startIndex = integer(start);
    size_t endIndex = integer(end - 1);
    /* Has zeroes at the bits we want to set to zero. */
    uint64_t firstZeroesMask = (firstZeroes == 64) ?
        (uint64_t)0 : 0xFFFFFFFFFFFFFFFFu << firstZeroes;
    uint64_t lastZeroesMask = (lastZeroes == 64) ?
        (uint64_t)0 : 0xFFFFFFFFFFFFFFFFu >> lastZeroes;

    if (startIndex == endIndex) {
        bitmap->bits[startIndex] &= firstZeroesMask & lastZeroesMask;
        return;
    }

    bitmap->bits[integer(start)] &= firstZeroesMask;

    bitmap->bits[integer(end - 1)] &= lastZeroesMask;

    /* Sets the rest of the bits to 0, one integer at a time.
     * Did not use memset as it uses char, whose size and representation is
     * implementation defined */
    for (long i = integer(start) + 1; i < (long)integer(end - 1); i++) {
        bitmap->bits[i] = 0;
    }
}

void BitmapSetOne(Bitmap *bitmap, size_t index)
{
    uint64_t mask = 0x8000000000000000u >> bit(index);
    bitmap->bits[integer(index)] |= mask;
}

void BitmapReset(Bitmap *bitmap)
{
    MUNMAP_SAFE(bitmap->bits, roundUp(bitmap->size, 64) * sizeof(uint64_t));
    MMAP_SAFE(bitmap->bits, mmap(NULL, roundUp(bitmap->size, 64)
                * sizeof(uint64_t),
                PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
}

void BitmapPrint(Bitmap *bitmap)
{
    for (size_t i = 0; i < roundUp(bitmap->size, 64); i++) {
        uint64_t integer = bitmap->bits[i];

        for (size_t bit = 0; bit < 64; bit++) {
                printf("%u", (integer & 0x8000000000000000u) ? 1 : 0);
                integer <<= 1;
        }
    }

    printf("\n");
}
