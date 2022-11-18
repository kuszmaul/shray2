#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST(function)                                        \
    {                                                       \
        if (function) {                                     \
            printf("%s was succesfull\n", #function);         \
        } else {                                            \
            printf("%s was unsuccesfull\n", #function);      \
        }                                                   \
    }

#include "../src/bitmap.c"

void testPrint(void)
{
    /* The last two bits are dummies. */
    printf("Bitmap that should have 1s except for indices [0, 3], [61, 63]\n");

    Bitmap *bitmap = BitmapCreate(190);
    bitmap->bits[0] = 0x0FFFFFFFFFFFFFF8u;
    bitmap->bits[1] = 0xFFFFFFFFFFFFFFFFu;
    bitmap->bits[2] = 0xFFFFFFFFFFFFFFFFu;

    BitmapPrint(bitmap);

    BitmapFree(bitmap);
}

int testSurrounding(void)
{
    Bitmap *bitmap = BitmapCreate(190);
    bitmap->bits[0] = 0x0FFFFFFFFFFFFFF8u;
    bitmap->bits[1] = 0xFFFFFFFFFFFFFFFFu;
    bitmap->bits[2] = 0xFFFFFFFFFFFFFFFFu;

    /* Should be [4, 61[ for 3 < index < 61, and [64, 190[ index > 63. */
    Range range1 = BitmapSurrounding(bitmap, 17);
    Range range2 = BitmapSurrounding(bitmap, 100);
    Range range3 = BitmapSurrounding(bitmap, 188);

    bitmap->bits[0] = 0x8000000000000000u;
    bitmap->bits[1] = 0xFFFFFFFFFFFFFFFFu;
    bitmap->bits[2] = 0xFFFFFFFFFFFFFFFFu;

    Range range4 = BitmapSurrounding(bitmap, 0);

    BitmapFree(bitmap);

    return (range1.start == 4 && range1.end == 61 &&
            range2.start == 64 && range2.end == 190 &&
            range3.start == 64 && range3.end == 190 &&
            range4.start == 0 && range4.end == 1);
}

int testCheck(void)
{
    /* Only has a one on position 70 and 0 */
    Bitmap *bitmap = BitmapCreate(110);
    bitmap->bits[0] = 0x8000000000000000u; bitmap->bits[1] = 0x0200000000000000u;

    printf("Only the first and 71th bit are one right?\n");
    BitmapPrint(bitmap);

    int result = (BitmapCheck(bitmap, 0) && BitmapCheck(bitmap, 70) &&
            !BitmapCheck(bitmap, 69) && !BitmapCheck(bitmap, 71));

    BitmapFree(bitmap);

    return result;
}

void testSetting(void)
{
    /* The last two bits are dummies. */
    Bitmap *bitmap = BitmapCreate(190);
    bitmap->bits[0] = 0xFFFFFFFFFFFFFFFFu;
    bitmap->bits[1] = 0xFFFFFFFFFFFFFFFFu;
    bitmap->bits[2] = 0xFFFFFFFFFFFFFFFFu;

    printf("190 1s\n");
    BitmapPrint(bitmap);

    printf("We set [5, 180[ to 0\n");
    BitmapSetZeroes(bitmap, 5, 180);
    BitmapPrint(bitmap);

    printf("We set [6, 179[ to 1\n");
    BitmapSetOnes(bitmap, 6, 179);
    BitmapPrint(bitmap);

    BitmapFree(bitmap);
}

void testSetting2(void)
{
    Bitmap *bitmap = BitmapCreate(1000);

    BitmapSetZeroes(bitmap, 0, 1000);
    BitmapSetOnes(bitmap, 976, 983);

    BitmapPrint(bitmap);
}

int testCount(void)
{
    uint64_t bla = 0xF0F0F0F0F0F0F0F0u;
    return (countBitsLeft(bla, 8) == 1) && (countBitsRight(bla, 0) == 4);
}

int testStencil(void)
{
    Bitmap *bitmap = BitmapCreate(100000);

    BitmapSetZeroes(bitmap, 0, 100000);
    BitmapSetOnes(bitmap, 97637, 97656);

    for (size_t i = 0; i < 100000; i++) {
        if (BitmapCheck(bitmap, i)) printf("%zu\n", i);
    }

    int success = !BitmapCheck(bitmap, 97636);
    BitmapFree(bitmap);

    return success;
}

int testReduce(void)
{
    Bitmap *bitmap = BitmapCreate(20);
    BitmapSetOnes(bitmap, 0, 9);

    int success = BitmapCheck(bitmap, 0);

    BitmapFree(bitmap);

    return success;
}

int main(void)
{
    testPrint();

    TEST(testSurrounding());
    TEST(testCheck());
    TEST(testCount());
    TEST(testStencil());
    TEST(testReduce());

    testSetting();

    testSetting2();

    return 0;
}
