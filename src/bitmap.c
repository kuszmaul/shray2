/* Bit hacking fun */

#include <stdint.h>

typedef struct {
    uint64_t *bits;
    /* Number of bits, not uint64_ts. */
    size_t size;
} Bitmap;

typedef struct {
    size_t start;
    size_t end;
}: Range;

/* Returns 0 iff the index'th bit of BitmapCheck is 0. */
inline int BitmapCheck(Bitmap bitmap, size_t index)
{
    return bitmap.bits[index / 64] & 
        (0b1000000000000000000000000000000000000000000000000000000000000000u >> (index % 64));
}

/* Counts how many consecutive bits in integer are 1 starting from the index'th bit. */
inline int countBitsRight(uint64_t integer, unsigned int index) 
{
    int result = 0;
    uint64_t mask = 0b1000000000000000000000000000000000000000000000000000000000000000u >> index;

    while (integer & mask) {
        result++;
        mask >>= 1;
    }

    return result;
}

/* Counts how many consecutive bits in integer are 1 ending at the index'th bit. */
inline int countBitsLeft(uint64_t integer, unsigned int index) 
{
    int result = 0;
    uint64_t mask = 0b1000000000000000000000000000000000000000000000000000000000000000u >> index;

    while (integer & mask) {
        result++;
        mask <<= 1;
    }

    return result;
}

/* Sets [start, end[ to zero. */
void setZeroes(Bitmap bitmap, size_t start, size_t end)
{
    int firstOnes = 64 - start % 64;
    int lastOnes = end % 64;

    if (firstOnes != 64) {
        bitmap.bits[start / 64] &= ((1 << firstOnes) - 1);
    }
}

///* Returns maximal set [start, end[ containing index such that bitmap[i] = 1 
// * for i in [start, end[. */
//Range BitmapSurrounding(Bitmap bitmap, size_t index)
//{
//    Range range;
//
//    size_t toTheRight = toTheLeft = index / 64;
//    range.end = index + countBitsRight(bitmap[toTheRight], index % 64);
//    range.start = index - countBitsLeft(bitmap[toTheLeft], index % 64);
//    toTheRight++;
//
//    while (bitmap.bits[toTheRight] == 0xFFFFFFFFFFFFFFFFu && 
//            toTheRight < (bitmap.size + 63) / 64) 
//    {
//        range.end += 64;
//        toTheRight++;
//    }
//
//    while (bitmap.bits[toTheLeft] == 0xFFFFFFFFFFFFFFFFu &&
//            toTheLeft >= 0)
//    {
//        range.start -= 64;
//        toTheLeft--;
//    }
//
//    range.left -= countBitsRight(bitmap[toTheLeft], 0);
//    range.end += countBitsRight(bitmap[toTheRight], 0);
//
//    return range;
//}
