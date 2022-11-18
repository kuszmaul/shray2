#include <shray2/shray.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void init(double *arr, size_t n)
{
    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        arr[i] = 1;
//        printf("arr[%zu] = %lf\n", i, arr[i]);
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
//                printf("%d %p %lf\n", ShrayRank(), (void *)(arr + i + rank * n / p),
//                        arr[i + rank * n / p]);
            }
            sum += arr[i + rank * n / p];
        }

        ShrayDiscard(arr + rank * n / p, n / p * sizeof(double));
    }

    return sum;
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 2) {
        printf("Usage: n\n");
        ShrayFinalize(1);
    }
    size_t n = atol(argv[1]);
    assert(n % ShraySize() == 0);

    double *arr = ShrayMalloc(n, n * sizeof(double));

    init(arr, n);
    ShraySync(arr);

    double result = reduce(arr, n);
    ShrayFree(arr);

    ShrayReport();

    if (result == n) {
        printf("Success from node %d\n", ShrayRank());
    } else {
        printf("Failure from node %d (%lf != %lf)\n", ShrayRank(), result,
               (double)n);
    }

    ShrayFinalize(0);
}
