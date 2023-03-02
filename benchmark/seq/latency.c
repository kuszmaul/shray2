#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>

#define TIME(duration, fncalls)                                        \
    {                                                                  \
        struct timeval tv1, tv2;                                       \
        gettimeofday(&tv1, NULL);                                      \
        fncalls                                                        \
        gettimeofday(&tv2, NULL);                                      \
        duration = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +    \
         (double) (tv2.tv_sec - tv1.tv_sec);                           \
    }

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: number of MB to do the bandwidth test on\n");
        exit(EXIT_FAILURE);
    }

    size_t size = atoll(argv[1]) * 1000 * 1000 / sizeof(double) * sizeof(double);

    double *numbers = malloc(size);
    for (size_t i = 0; i < size / sizeof(double); i++) {
        numbers[i] = i;
    }

    double duration;

    TIME(duration,
    for (size_t i = 0; i < size / sizeof(double); i++) {
        numbers[i] *= 2;
    }
    );

    printf("We have a bandwidth of %lf GB/s (%lf ns per page)\n",
            size / 1000000000.0 / duration, 1000000000.0 * duration / size * 4096);

    return EXIT_SUCCESS;
}
