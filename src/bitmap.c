#include "bitmap.h"
#include <stdio.h>
#include <sys/mman.h>
#include <immintrin.h>

/*******************************************
 * Error handling macros
 *******************************************/

#define MALLOC_SAFE(variable, size)                                                     \
    {                                                                                   \
        variable = malloc(size);                                                        \
        if (variable == NULL) {                                                         \
            fprintf(stderr, "Line %d, malloc failed with size %zu\n",                   \
                    __LINE__, size);                                                    \
            exit(EXIT_FAILURE);                                                         \
        }                                                                               \
    }

#define MMAP_SAFE(variable, fncall)                                                     \
    {                                                                                   \
        variable = fncall;                                                              \
        if (variable == MAP_FAILED) {                                                   \
            fprintf(stderr, "Line %d: ", __LINE__);                                     \
            perror("mmap failed");                                                      \
            exit(EXIT_FAILURE);                                                         \
        }                                                                               \
    }

#define MUNMAP_SAFE(address, length)                                                    \
    {                                                                                   \
        if (munmap(address, length) == -1) {                                            \
            fprintf(stderr, "Unmapping [%p, %p[\n", (void *)address,                    \
                    (void *)((uintptr_t)address + length));                             \
            perror("munmap failed");                                                    \
        }                                                                               \
    }

/*******************************************
 * Internal helper functions
 *******************************************/

static inline size_t max(size_t x, size_t y)
{
    return x > y ? x : y;
}

static inline size_t min(size_t x, size_t y)
{
    return x < y ? x : y;
}

/* Returns ceil(a / b) */
static inline uintptr_t roundUp(uintptr_t a, uintptr_t b)
{
    return (a + b - 1) / b;
}

#ifdef __GNUC__
#include "immintrin.h"
static inline int countBitsLeft(uint64_t integer, int bit)
{
    /* Now we want to know how many consecutive trailing bits are one. */
    integer >>= 63 - bit;

    /* Now we want to know how many trailing bits are 0. */
    integer = ~integer;

    return __builtin_ctzl(integer);
}

static inline int countBitsRight(uint64_t integer, int bit)
{
    /* Now we want to know how many consecutive leading bits are one. */
    integer <<= bit;

    /* Now we want to know how many consecutive leading bits are 0. */
    integer = ~integer;

    return __builtin_clzl(integer);
}

#else

static inline int countBitsRight(uint64_t integer, int bit)
{
    int result = 0;
    uint64_t mask = 0x8000000000000000u >> bit;

    while (integer & mask) {
        result++;
        mask >>= 1;
    }

    return result;
}

static inline int countBitsLeft(uint64_t integer, int bit)
{
    int result = 0;
    uint64_t mask = 0x8000000000000000u >> bit;

    while (integer & mask) {
        result++;
        mask <<= 1;
    }

    return result;
}

#endif /* __GNUC__ */

/* Returns n such that bitmap->bits[n] contains the index'th bit of the bitmap. */
inline static size_t integer(size_t index)
{
    return index / 64;
}

/* Returns the p such that the pth bit of bitmap->bits[n] is the index'th bit of the bitmap. */
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

    /* We use mmap because it initializes to zero, and allocates lazily on Linux. This
     * saves memory in case a lot of the bitmap stays zero. */
    MMAP_SAFE(bitmap->bits, mmap(NULL, roundUp(size, 64) * sizeof(uint64_t), PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
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
    return ((bitmap->bits[integer(index)] & (0x8000000000000000u >> bit(index))) != (uint64_t)0);
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
    uint64_t firstZeroesMask = 0xFFFFFFFFFFFFFFFFu << firstZeroes;
    uint64_t lastZeroesMask = 0xFFFFFFFFFFFFFFFFu >> lastZeroes;

    if (startIndex == endIndex) {
        bitmap->bits[startIndex] &= firstZeroesMask & lastZeroesMask;
        return;
    }

    bitmap->bits[integer(start)] &= firstZeroesMask;

    bitmap->bits[integer(end - 1)] &= lastZeroesMask;

    /* Sets the rest of the bits to 0, one integer at a time. Did not use memset as it uses
     * char, whose size and representation is implementation defined */
    for (long i = integer(start) + 1; i < (long)integer(end - 1); i++) {
        bitmap->bits[i] = 0;
    }
}

/* Sets [start, end[ to one. */
void BitmapSetOnes(Bitmap *bitmap, size_t start, size_t end)
{
    /* Number of bits to be set to one in the first / last uint64_t. */
    int firstOnes = 64 - bit(start);
    int lastOnes = 1 + bit(end - 1);
    size_t startIndex = integer(start);
    size_t endIndex = integer(end - 1);
    /* Has ones at the bits we want to set to one. */
    uint64_t firstOnesMask = ~(0xFFFFFFFFFFFFFFFFu << firstOnes);
    uint64_t lastOneMask = ~(0xFFFFFFFFFFFFFFFFu >> lastOnes);

    if (startIndex == endIndex) {
        bitmap->bits[startIndex] |= firstOnesMask & lastOneMask;
        return;
    }

    bitmap->bits[startIndex] |= firstOnesMask;

    bitmap->bits[endIndex] |= lastOneMask;

    /* Sets the rest of the bits to 1, one integer at a time. Did not use memset as it uses
     * char, whose size and representation is implementation defined */
    for (long i = integer(start) + 1; i < (long)integer(end - 1); i++) {
        bitmap->bits[i] = 0xFFFFFFFFFFFFFFFFu;
    }
}

void BitmapReset(Bitmap *bitmap)
{
    MUNMAP_SAFE(bitmap->bits, roundUp(bitmap->size, 64) * sizeof(uint64_t));
    MMAP_SAFE(bitmap->bits, mmap(NULL, roundUp(bitmap->size, 64) * sizeof(uint64_t),
                PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
}

void BitmapCopyOnes(Bitmap *dest, Bitmap *src)
{
    for (size_t i = 0; i < roundUp(dest->size, 64); i++) {
        dest->bits[i] |= src->bits[i];
    }
}

/* Returns maximal set [start, end[ containing index such that bitmap[i] = 1
 * for i in [start, end[. */
Range BitmapSurrounding(Bitmap *bitmap, size_t index)
{
    Range range;

    /* We slide start to the left as far as possible within the current integer. */
    range.start = index + 1 - countBitsLeft(bitmap->bits[integer(index)], bit(index));

    /* There are no more consecutive 1s in the integer to the left from us. */
    if (index / 64 == 0 || range.start % 64 != 0) goto BitmapEnd;

    size_t toTheLeft = index / 64 - 1;

    while (bitmap->bits[toTheLeft] == 0xFFFFFFFFFFFFFFFFu) {
        range.start -= 64;
        if (toTheLeft == 0) goto BitmapEnd;
        toTheLeft--;
    }

    range.start -= countBitsLeft(bitmap->bits[toTheLeft], 63);

BitmapEnd:
    /* We are the last integer. */
    if (index / 64 == (bitmap->size - 1) / 64) {
        range.end = min(index + countBitsRight(bitmap->bits[index / 64], index % 64),
                bitmap->size);
        return range;
    }

    /* We slide start to the right as far as possible within the current integer. */
    range.end = index + countBitsRight(bitmap->bits[index / 64], index % 64);

    /* There are no more consecutive 1s in the integer to the right from us. */
    if (range.end % 64 != 0) return range;

    /* We have 1s until the end of this integer and we are not the last integer. */
    size_t toTheRight = index / 64 + 1;

    while (toTheRight < (bitmap->size - 1) / 64 &&
            bitmap->bits[toTheRight] == 0xFFFFFFFFFFFFFFFFu)
    {
        range.end += 64;
        if (toTheRight == (bitmap->size - 1) / 64) return range;
        toTheRight++;
    }

    /* We are not the last integer in the bitmap. */
    if (toTheRight < (bitmap->size - 1) / 64) {
        range.end += countBitsRight(bitmap->bits[toTheRight], 0);
    }

    /* We are the last integer in the bitmap. */
    if (toTheRight == (bitmap->size - 1) / 64) {
        range.end += min(countBitsRight(bitmap->bits[toTheRight], 0), bitmap->size % 64);
    }

    return range;
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
