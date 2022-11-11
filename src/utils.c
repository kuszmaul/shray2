/* Bit hacking fun */

/*************************************************
 * General stuff 
 *************************************************/

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

/*************************************************
 * Bitmap stuff 
 *************************************************/

void BitmapCreate(Bitmap *bitmap, size_t size)
{
    MALLOC_SAFE(bitmap->bits, roundUp(size, 64));
    bitmap->size = size;
}

/* Returns 0 iff the index'th bit of bitmap is 0. */
static inline uint64_t BitmapCheck(Bitmap bitmap, size_t index)
{
    return bitmap.bits[index / 64] & (0x8000000000000000u >> (index % 64));
}

/* FIXME Can we get rid of the loop by a smart number theory thing? */
/* Counts how many consecutive bits in integer are 1 starting from the index'th bit. */
static inline int countBitsRight(uint64_t integer, unsigned int index) 
{
    int result = 0;
    uint64_t mask = 0x8000000000000000u >> index;

    while (integer & mask) {
        result++;
        mask >>= 1;
    }

    return result;
}

/* Counts how many consecutive bits in integer are 1 ending at the index'th bit. */
static inline int countBitsLeft(uint64_t integer, unsigned int index) 
{
    int result = 0;
    uint64_t mask = 0x8000000000000000u >> index;

    while (integer & mask) {
        result++;
        mask <<= 1;
    }

    return result;
}

/* Sets [start, end[ to zero. */
static void BitmapSetZeroes(Bitmap bitmap, size_t start, size_t end)
{
    /* Number of bits to be set to zero in the first / last uint64_t. */
    int firstZeroes = 64 - start % 64;
    int lastZeroes = end % 64;

    if (firstZeroes != 64) {
        bitmap.bits[start / 64] &= 0xFFFFFFFFFFFFFFFFu - 
            (((uint64_t)1 << firstZeroes) - 1);
    }

    if (lastZeroes != 0) {
        bitmap.bits[(end - 1) / 64] &= ((uint64_t)1 << (64 - lastZeroes)) - 1;
    }

    for (long i = start / 64 + 1; i < (long)(end - 1) / 64; i++) {
        bitmap.bits[i] = 0;
    }
}

/* Sets [start, end[ to one. */
static void BitmapSetOnes(Bitmap bitmap, size_t start, size_t end)
{
    /* Number of bits to be set to one in the first / last uint64_t. */
    int firstOnes = 64 - start % 64;
    int lastOnes = end % 64;

    if (firstOnes != 64) {
        bitmap.bits[start / 64] |= ((uint64_t)1 << firstOnes) - 1;
    }

    if (lastOnes != 0) {
        bitmap.bits[(end - 1) / 64] |= 0xFFFFFFFFFFFFFFFFu -  (((uint64_t)1 << (64 - lastOnes)) - 1);
    }

    for (long i = start / 64 + 1; i < (long)(end - 1) / 64; i++) {
        bitmap.bits[i] = 0xFFFFFFFFFFFFFFFFu;
    }
}

/* Returns maximal set [start, end[ containing index such that bitmap[i] = 1 
 * for i in [start, end[. */
static Range BitmapSurrounding(Bitmap bitmap, size_t index)
{
    Range range;

    range.start = index + 1 - countBitsLeft(bitmap.bits[index / 64], index % 64);
    /* We have 1s until the start of this integer and it is not the first. */
    if (range.start % 64 == 0 && index / 64 != 0) {
        size_t toTheLeft = index / 64 - 1;

        while (toTheLeft > 0 && bitmap.bits[toTheLeft] == 0xFFFFFFFFFFFFFFFFu) {
            range.start -= 64;
            toTheLeft--;
        }

        /* In this case we have taken the left-most integer into account already. */
        if (!(index / 64 == 0 && toTheLeft == 0)) {
            range.start -= countBitsLeft(bitmap.bits[toTheLeft], 63);
        }
    }

    /* We are the last integer. */
    if (index / 64 == (bitmap.size - 1) / 64) {
        range.end = min(index + countBitsRight(bitmap.bits[index / 64], index % 64), 
                bitmap.size);
        return range;
    }

    range.end = index + countBitsRight(bitmap.bits[index / 64], index % 64);

    /* We have 1s until the end of this integer and we are not the last integer. */
    if (range.end % 64 == 0) {
        size_t toTheRight = index / 64 + 1;

        while (toTheRight < (bitmap.size - 1) / 64 && 
                bitmap.bits[toTheRight] == 0xFFFFFFFFFFFFFFFFu)
        {
            range.end += 64;
            toTheRight++;
        }

        /* We are not the last integer in the bitmap. */
        if (toTheRight < (bitmap.size - 1) / 64) {
            range.end += countBitsRight(bitmap.bits[toTheRight], 0);
        }

        /* We are the last integer in the bitmap. */
        if (toTheRight == (bitmap.size - 1) / 64) {
            range.end += min(countBitsRight(bitmap.bits[toTheRight], 0), bitmap.size % 64);
        }
    }

    return range;
}

static void BitmapPrint(Bitmap bitmap)
{
    for (size_t i = 0; i < roundUp(bitmap.size, 64); i++) {
        uint64_t integer = bitmap.bits[i];

        for (size_t bit = 0; bit < 64; bit++) {
            printf("%u", (integer & 0x8000000000000000u) ? 1 : 0);
            integer <<= 1;
        }
    }

    printf("\n");
}
