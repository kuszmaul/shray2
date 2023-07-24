#include <shray2/shray.h>
#include <stdio.h>
#include <stdlib.h>

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

    for (size_t i = ShrayEnd(arr); i < n; i++) {
        sum += arr[i];
    }

    return sum;
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 2) {
        printf("Usage: lenght of the vector you reduce\n");
        ShrayFinalize(1);
    }

    size_t n = atoll(argv[1]);

    double *A = (double *)ShrayMalloc(n, n * sizeof(double));
    init(A);
    ShraySync(A);

    double duration;
    double result;


    if (ShrayOutput) {
        TIME(duration, result = reduceAuto(A, n););
        double microsPerPage = 1000000.0 * duration / ((n - ShrayEnd(A)) / 512);

        fprintf(stderr, "We reduced an array on %d processors at %lf microseconds per page:\n"
                "That is a bandwidth of %lf GB/s\n",
                ShraySize(), microsPerPage, 4096.0 / microsPerPage / 1000.0);
        printf("%lf", microsPerPage);
        fprintf(stderr, "%lf\n", result);
    }

    ShrayFree(A);

    ShrayReport();
    ShrayFinalize(0);
}
