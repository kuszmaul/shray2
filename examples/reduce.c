#include <shray2/shray.h>
#include <assert.h>

void init(double *arr, size_t n)
{
    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        arr[i] = 1;
    }
}

/* For testing only, otherwise obviously reduce locally and send your result to the 
 * other nodes. */
double reduce(double *arr, size_t n)
{
	unsigned int p = ShraySize();
	unsigned int s = ShrayRank();

    double sum = 0.0;
    ShrayGet(arr + (s + 1) % p * n / p, n / p * sizeof(double));

    for (size_t i = 0; i < n / p; i++) {
        sum += arr[i + s * n / p];
    }

    for (unsigned int t = 1; t < p; t++) {
        unsigned int rank = (t + s) % p;

        ShrayGetComplete(arr + rank * n / p);

        if (t != p - 1) {
            ShrayGet(arr + (rank + 1) % p * n / p, n / p * sizeof(double));
        }

        for (size_t i = 0; i < n / p; i++) {
            sum += arr[i + rank * n / p];
        }

        ShrayGetFree(arr + rank * n / p);
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

    if (result == n) {
        printf("Success from node %d\n", ShrayRank());
    } else {
        printf("Failure from node %d (%lf != %lf)\n", ShrayRank(), result, 
               (double)n);
    }

    ShrayFinalize(0);
}
