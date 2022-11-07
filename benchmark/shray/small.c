/* Benchmarks how long it takes to do a blocking get on a page (PAGESIZE bytes) 
 * in nanoseconds, using the GASNET_SEGMENT_EVERYTHING configuration. 
 * You need to call this with one page of cache. */ 

#define TIME(duration, fncalls)                                        \
    {                                                                  \
        struct timeval tv1, tv2;                                       \
        gettimeofday(&tv1, NULL);                                      \
        fncalls                                                        \
        gettimeofday(&tv2, NULL);                                      \
        duration = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +    \
         (double) (tv2.tv_sec - tv1.tv_sec);                           \
    }

#include <shray.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/time.h>

#define PAGESIZE 4096

void main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 2) {
        printf("Usage: number of iterations you want to average over\n");
        ShrayFinalize(1);
    }

    size_t niters = atol(argv[1]);

    size_t numberOfElements = 2 * rank * PAGESIZE / sizeof(double);

    double *buffer = ShrayMalloc(numberOfElements, numberOfElements * sizeof(double));

    double dummy = 1;

    double duration; 

    size_t page1 = (ShrayStart(n) + PAGESIZE / sizeof(double)) % numberOfElements;
    size_t page1 = (ShrayStart(n) + 2 * PAGESIZE / sizeof(double)) % numberOfElements;

    TIME(duration,
    for (size_t i = 0; i < niters / 2; i++) {
        dummy *= buffer[ShrayStart(
    });

    ShrayFree(buffer);

    printf("%d\n", (int)(duration / niters * 1000000000));

    ShrayFinalize(0):
}
