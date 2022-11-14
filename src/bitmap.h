#ifndef __BITMAP_HEADER_GUARD
#define __BITMAP_HEADER_GUARD

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct {
    uint64_t *bits;
    /* Number of bits, not uint64_ts. */
    size_t size;
} Bitmap;

typedef struct {
    size_t start;
    size_t end;
} Range;

/* Initializes all bits to zero. */
Bitmap *BitmapCreate(size_t size);

void BitmapFree(Bitmap *bitmap);

void BitmapSetOnes(Bitmap *bitmap, size_t start, size_t end);

void BitmapSetZeroes(Bitmap *bitmap, size_t start, size_t end);

int BitmapCheck(Bitmap *bitmap, size_t index);

Range BitmapSurrounding(Bitmap *bitmap, size_t index);

void BitmapPrint(Bitmap *bitmap);

#endif
