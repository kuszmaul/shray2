#include <shray2/shray.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../util/time.h"

void init(double *arr)
{
    for (size_t i = ShrayStart(arr); i < ShrayEnd(arr); i++) {
        arr[i] = 1;
    }
}

/* For testing only, otherwise obviously reduce locally and send your result to the
 * other nodes. Each node sums up the array. */
double reduce(double *arr, size_t n)
{
	unsigned int p = ShraySize();
	unsigned int s = ShrayRank();

    double sum = 0.0;
    /* Prefetch the data owned by the next node, where node 0 comes after node p - 1. */
    ShrayPrefetch(arr + (s + 1) % p * n / p, n / p * sizeof(double));

    /* Local reduce */
    for (size_t i = 0; i < n / p; i++) {
        sum += arr[i + s * n / p];
    }

    for (unsigned int t = 1; t < p; t++) {
        unsigned int rank = (t + s) % p;

        if (t != p - 1) {
            ShrayPrefetch(arr + (rank + 1) % p * n / p, n / p * sizeof(double));
        }

        /* Reduce wrt rank 'rank' */
        for (size_t i = 0; i < n / p; i++) {
            if (arr[i + rank * n / p] != 1.0) {
            }
            sum += arr[i + rank * n / p];
        }

        ShrayDiscard(arr + rank * n / p);
    }

    return sum;
}

/* For testing only, otherwise obviously reduce locally and send your result to the
 * other nodes. Each node sums up the array. */
double reduceAuto(double *arr, size_t n)
{
    double sum = 0.0;

    for (size_t i = ShrayStart(arr); i < n; i++) {
        sum += arr[i];
    }

    return sum;
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 3) {
        printf("Usage: n NITER\n");
        ShrayFinalize(1);
    }
    size_t n = atol(argv[1]);
    int niter = atoi(argv[2]);

    double *bandwidths = (double *)malloc(niter * sizeof(double));

    for (int t = 0; t < niter; t++) {
        double *arr = (double *)ShrayMalloc(n, n * sizeof(double));

        init(arr);
        ShraySync(arr);

        if (ShrayRank() == 0) {
            double duration;
            double result;
            TIME(duration, result = reduceAuto(arr, n););
            fprintf(stderr, (result == n - ShrayStart(arr)) ?
                    "Success\n" : "Failure\n");
            bandwidths[t] = 8.0 * (n - ShrayStart(arr))  / 1e6 / duration;
        }

        ShrayFree(arr);
    }

    /* Wellfords algorithm may be overkill, but numerical stability is nice. */
    int count = 0;
    double mean = 0;
    double m2 = 0;
    for (int t = 0; t < niter; t++) {
        count++;
        double delta_pre = bandwidths[t] - mean;
        mean += delta_pre / count;
        double delta_post = bandwidths[t] - mean;
        m2 += delta_pre * delta_post;
    }

    printf(" %lf & %lf \\\\\n", mean, sqrt(m2 / count));

    free(bandwidths);

    ShrayReport();

    ShrayFinalize(0);
}
