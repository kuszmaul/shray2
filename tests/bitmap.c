#include <stdint.h>
#include <stdio.h>
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

#include "../src/utils.c"

void testPrint(void)
{
    /* The last two bits are dummies. */
    printf("Bitmap that should have 1s except for indices [0, 3], [61, 63]\n");
    uint64_t *bits = malloc(3 * sizeof(uint64_t));
    bits[0] = 0x0FFFFFFFFFFFFFF8u; bits[1] = 0xFFFFFFFFFFFFFFFFu; bits[2] = 0xFFFFFFFFFFFFFFFFu;

    Bitmap bitmap = {bits, 64 * 3 - 2};
    BitmapPrint(bitmap);

    free(bits);
}

int testSurrounding(void)
{
    /* The last two bits are dummies. */
    uint64_t *bits = malloc(3 * sizeof(uint64_t));
    bits[0] = 0x0FFFFFFFFFFFFFF8u; bits[1] = 0xFFFFFFFFFFFFFFFFu; bits[2] = 0xFFFFFFFFFFFFFFFFu;

    Bitmap bitmap = {bits, 64 * 3 - 2};

    /* Should be [4, 61[ for 3 < index < 61, and [64, 64 * 3 - 2[ index > 63. */
    Range range1 = BitmapSurrounding(bitmap, 17); 
    Range range2 = BitmapSurrounding(bitmap, 100); 

    free(bits);

    return (range1.start == 4 && range1.end == 61 && 
            range2.start == 64 && range2.end == 64 * 3 - 2); 
}

int main(void)
{
    testPrint();

    if (testSurrounding) {
        printf("BitmapSurrounding works as expected\n");
    } else {
        printf("BitmapSurrounding does not work as expected\n");
    }

    return 0;
}
