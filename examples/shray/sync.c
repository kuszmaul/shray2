#include <shray2/shray.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 2) {
        printf("Usage: number of iterations you want to average over.\n");
        ShrayFinalize(1);
    }

    int iterations = atoi(argv[1]);

    unsigned int p = ShraySize();
    /* The constant does not really matter as long as it is larger than one page, and not
     * a multiple of the page-size. */
    double *A = ShrayMalloc(100000000 * p, 100000000 * p * sizeof(double));

    double duration;

    TIME(duration,
    for (int t = 0; t < iterations; t++) {
        ShraySync(A);
    }
    );

    if (ShrayOutput) {
        fprintf(stderr, "Average time for a synchronisation on %d processors (in microseconds):\n",
                p);
        printf("%lf\n", duration / (double) iterations * 1000000.0);
    }

    ShrayFree(A);

    ShrayFinalize(0);
}
