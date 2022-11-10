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

/* Returns 0 iff the index'th bit of bitmap is 0. */
static inline int BitmapCheck(Bitmap bitmap, size_t index)
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
        bitmap.bits[start / 64] &= 0xFFFFFFFFFFFFFFFFu - ((1 << firstZeroes) - 1);
    }

    if (lastZeroes != 0) {
        bitmap.bits[end / 64] &= (1 << lastZeroes) - 1;
    }

    for (size_t i = start / 64; i <= (end - 1) / 64; i++) {
        bitmap.bits[i / 64] = 0;
    }
}

/* Sets [start, end[ to one. */
static void BitmapSetOnes(Bitmap bitmap, size_t start, size_t end)
{
    /* Number of bits to be set to one in the first / last uint64_t. */
    int firstOnes = 64 - start % 64;
    int lastOnes = end % 64;

    if (firstOnes != 64) {
        bitmap.bits[start / 64] |= (1 << firstOnes) - 1;
    }

    if (lastOnes != 0) {
        bitmap.bits[end / 64] |= 0xFFFFFFFFFFFFFFFFu - ((1 << (64 - lastOnes)) - 1);
    }

    for (size_t i = start / 64; i <= (end - 1) / 64; i++) {
        bitmap.bits[i / 64] = 0xFFFFFFFFFFFFFFFFu;
    }
}

/* Returns maximal set [start, end[ containing index such that bitmap[i] = 1 
 * for i in [start, end[. */
static Range BitmapSurrounding(Bitmap bitmap, size_t index)
{
    Range range;

    size_t toTheRight = index / 64;
    size_t toTheLeft = index / 64;
    range.end = index + countBitsRight(bitmap.bits[toTheRight], index % 64);
    range.start = index - countBitsLeft(bitmap.bits[toTheLeft], index % 64);
    toTheRight++;

    while (bitmap.bits[toTheRight] == 0xFFFFFFFFFFFFFFFFu && 
            toTheRight < roundUp(bitmap.size, 64)) 
    {
        range.end += 64;
        toTheRight++;
    }

    while (bitmap.bits[toTheLeft] == 0xFFFFFFFFFFFFFFFFu &&
            toTheLeft > 0)
    {
        range.start -= 64;
        toTheLeft--;
    }

    range.start -= countBitsRight(bitmap.bits[toTheLeft], 0);

    if (toTheRight == (bitmap.size + 63) / 64) {
        /* Last integer in the bitmap. */
        range.end += max(bitmap.size % 64, countBitsRight(bitmap.bits[toTheRight], 0)); 
    } else {
        range.end += countBitsRight(bitmap.bits[toTheRight], 0);
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
