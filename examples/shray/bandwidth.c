#include <shray2/shray.h>
#include <stdio.h>
#include <stdlib.h>
#include "../util/time.h"

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 2) {
        printf("Usage: total communication volume in MiB.\n");
        ShrayFinalize(1);
    }

    if (ShraySize() != 2) {
        printf("Please run on two processors instead of %d\n", ShraySize());
        ShrayFinalize(1);
    }

    size_t n = 2 * atol(argv[1]) * 1024 * 1024 / 8;

    double duration;

    double *array = (double *)ShrayMalloc(n, n * sizeof(double));

    TIME(duration,
        if (ShrayRank() == 1) {
            /* We touch the first element of each page in order to trigger
             * the communication. */
            for (size_t i = 0; i < ShrayStart(array); i += 512) {
                array[i] *= 2;
            }
        }
    );

    ShrayFree(array);

    if (ShrayRank() == 1) {
        double bandwidth = n * sizeof(double) / 2 / duration / 1e6;
        fprintf(stderr, "We achieve a bandwidth of %lf MB/s\n", bandwidth);
        printf("%lf", bandwidth);
    }

    ShrayFinalize(0);
}
