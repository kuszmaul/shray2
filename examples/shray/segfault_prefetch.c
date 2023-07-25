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
double reduce(double *arr, size_t n, size_t lookahead)
{
    double sum = 0.0;

    ShrayPrefetch(arr + ShrayEnd(arr), lookahead * sizeof(double));
    for (size_t ib = ShrayEnd(arr); ib < n; ib += lookahead) {
        if (ib + 2 * lookahead < n) {
            ShrayPrefetch(&arr[ib + lookahead], lookahead * sizeof(double));
        }
        for (size_t i = ib; i < ib + lookahead && i < n; i++) {
            sum += arr[i];
        }
        ShrayDiscard(&arr[ib]);
    }

    return sum;
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 3) {
        printf("Usage: lenght of the vector you reduce, lookahead\n");
        ShrayFinalize(1);
    }

    size_t n = atoll(argv[1]);
    size_t lookahead = atoll(argv[2]);

    double *A = (double *)ShrayMalloc(n, n * sizeof(double));
    init(A);
    ShraySync(A);

    double duration;
    double result;

    if (ShrayOutput) {
        TIME(duration, result = reduce(A, n, lookahead););
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
