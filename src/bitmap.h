#ifndef __BITMAP_HEADER_GUARD
#define __BITMAP_HEADER_GUARD

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#define BITMAP_ENTRY_SIZE 64

typedef struct {
    uint64_t *bits;
    /* Number of bits, not uint64_ts. */
    size_t size;
} Bitmap;

/* Initializes all bits to zero. */
Bitmap *BitmapCreate(size_t size);

/* Frees bitmap and all its resources. */
void BitmapFree(Bitmap *bitmap);

/* Sets [start, end[ to zero. */
void BitmapSetZeroes(Bitmap *bitmap, size_t start, size_t end);

/* Sets [start, end[ to one. */
void BitmapSetOnes(Bitmap *bitmap, size_t start, size_t end);

/* Sets everything to zero, more performant than BitmapSetZeroes and frees
 * up memory. */
void BitmapReset(Bitmap *bitmap);

/* Returns 1 iff the index'th bit of bitmap is 1. */
int BitmapCheck(Bitmap *bitmap, size_t index);

void BitmapPrint(Bitmap *bitmap);

#endif
