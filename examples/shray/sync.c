#include <shray2/shray.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../util/time.h"

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 3) {
        printf("Usage: number of iterations you want to average over, niter.\n");
        ShrayFinalize(1);
    }

    int iterations = atoi(argv[1]);
    int niter = atoi(argv[2]);

    unsigned int p = ShraySize();
    /* The constant does not really matter as long as it is larger than one page, and not
     * a multiple of the page-size. */
    double *A = (double *)ShrayMalloc(100000000 * p, 100000000 * p * sizeof(double));

    double duration;

    /* Wellfords algorithm may be overkill, but numerical stability is
       nice. */
    int count = 0;
    double mean = 0;
    double m2 = 0;

    for (int i = 0; i < niter; i++) {
        TIME(duration,
        for (int t = 0; t < iterations; t++) {
            ShraySync(A);
        }
        );

        double avg_time = duration / (double) iterations * 1e6;

        count++;
        double delta_pre = avg_time - mean;
        mean += delta_pre / count;
        double delta_post = avg_time - mean;
        m2 += delta_pre * delta_post;
    }

    if (ShrayOutput) {
        fprintf(stderr, "Average time for a synchronisation on %d processors (in microseconds):\n",
                p);
        printf(" & %lf & %lf \\\\\n", mean, sqrt(m2 / count));
    }

    ShrayFree(A);

    ShrayReport();

    ShrayFinalize(0);
}
