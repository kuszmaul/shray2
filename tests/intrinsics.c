#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint64_t state = 0x9723487482918274u;

uint64_t xorshift64(uint64_t previous)
{
    uint64_t x = previous;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;

    state = x;
    return x;
}

void printInteger(uint64_t integer)
{
    for (size_t bit = 0; bit < 64; bit++) {
            printf("%u", (integer & 0x8000000000000000u) ? 1 : 0);
            integer <<= 1;
    }
    printf("\n");
}

#ifdef __GNUC__
#include "immintrin.h"
int countBitsLeft(uint64_t integer, int bit) 
{
    /* Now we want to know how many consecutive trailing bits are one. */
    integer >>= 63 - bit; 

//    printInteger(integer);

    /* Now we want to know how many trailing bits are 0. */
    integer = ~integer;
//    printInteger(integer);

    return __builtin_ctzl(integer);
}

int countBitsRight(uint64_t integer, int bit) 
{
    /* Now we want to know how many consecutive leading bits are one. */
    integer <<= bit;
    printInteger(integer);

    /* Now we want to know how many consecutive leading bits are 0. */
    integer = ~integer;
    printInteger(integer);

    return __builtin_clzl(integer);
}

#else

int countBitsRight(uint64_t integer, int bit) 
{
    int result = 0;
    uint64_t mask = 0x8000000000000000u >> bit;

    while (integer & mask) {
        result++;
        mask >>= 1;
    }

    return result;
}

int countBitsLeft(uint64_t integer, int bit) 
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

int main(void)
{
    for (int i = 0; i < 10; i++) {
        int bit = rand() % 64;
        uint64_t integer = xorshift64(state);
        printf("Bit %d, integer:\n", bit);
        printInteger(integer);
        printf("Left: %d, right: %d\n", countBitsLeft(integer, bit), countBitsRight(integer, bit));
    }

    return 0;
}
