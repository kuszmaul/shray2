#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MALLOC_SAFE(variable, size)                                                     \
    {                                                                                   \
        variable = malloc(size);                                                        \
        if (variable == NULL) {                                                         \
            fprintf(stderr, "Line %d: malloc failed with size %zu\n",                   \
                    __LINE__, size);                                                    \
        }                                                                               \
    }

#define TEST(function)                                        \
    {                                                       \
        if (function) {                                     \
            printf("%s was succesfull\n", #function);         \
        } else {                                            \
            printf("%s was unsuccesfull\n", #function);      \
        }                                                   \
    }

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

    Bitmap bitmap = {bits, 190};

    /* Should be [4, 61[ for 3 < index < 61, and [64, 190[ index > 63. */
    Range range1 = BitmapSurrounding(bitmap, 17); 
    Range range2 = BitmapSurrounding(bitmap, 100); 
    Range range3 = BitmapSurrounding(bitmap, 188); 

    free(bits);

//    printf("[%zu, %zu[ [%zu, %zu[ [%zu, %zu[\n", 
//            range1.start, range1.end, range2.start, range2.end, range3.start, range3.end);

    return (range1.start == 4 && range1.end == 61 && 
            range2.start == 64 && range2.end == 190 && 
            range3.start == 64 && range3.end == 190); 
}

int testCheck(void)
{
    /* Only has a one on position 70 */
    uint64_t *bits = malloc(2 * sizeof(uint64_t));
    bits[0] = 0; bits[1] = 0x0200000000000000u;

    Bitmap bitmap = {bits, 110};

    printf("Only the 71th bit is one right?\n");
    BitmapPrint(bitmap);

    return (BitmapCheck(bitmap, 70) && !BitmapCheck(bitmap, 69) && !BitmapCheck(bitmap, 71)); 
}

void testSetting(void)
{
    /* The last two bits are dummies. */
    uint64_t *bits = malloc(3 * sizeof(uint64_t));
    bits[0] = 0xFFFFFFFFFFFFFFFFu; bits[1] = 0xFFFFFFFFFFFFFFFFu; bits[2] = 0xFFFFFFFFFFFFFFFFu;

    Bitmap bitmap = {bits, 64 * 3 - 2};

    printf("190 1s\n");
    BitmapPrint(bitmap);

    printf("We set [5, 180[ to 0\n");
    BitmapSetZeroes(bitmap, 5, 180);
    BitmapPrint(bitmap);

    printf("We set [6, 179[ to 1\n");
    BitmapSetOnes(bitmap, 6, 179);
    BitmapPrint(bitmap);

    free(bits);
}

int testCount(void)
{
    uint64_t bla = 0xF0F0F0F0F0F0F0F0u;
    return (countBitsLeft(bla, 8) == 1) && (countBitsRight(bla, 0) == 4);
}

int testStencil(void)
{
    Bitmap bitmap;
    BitmapCreate(&bitmap, 100000);

    BitmapSetZeroes(bitmap, 0, 100000);
    BitmapSetOnes(bitmap, 97637, 97656);

    int success = !BitmapCheck(bitmap, 97636);
    free(bitmap.bits);

    return success;
}

int main(void)
{
    testPrint();

    TEST(testSurrounding());
    TEST(testCheck());
    TEST(testCount());
    TEST(testStencil());

    testSetting();

    return 0;
}
