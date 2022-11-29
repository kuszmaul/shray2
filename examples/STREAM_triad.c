#include <shray2/shray.h>
#include <stdio.h>
#include <stdlib.h>

void init(double *arr, size_t n)
{
    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        arr[i] = 1.0;
    }
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 2) {
        printf("Usage: n (should be larger than 1000000 and 4x the sum of last-level caches.)\n"
               "Prints the achieved read bandwidth in GB/s.\n");
        ShrayFinalize(1);
    }

    size_t n = atoll(argv[1]);

    double SCALAR = 0.3;
    double *A = ShrayMalloc(n, n * sizeof(double));
    double *B = ShrayMalloc(n, n * sizeof(double));
    double *C = ShrayMalloc(n, n * sizeof(double));

    init(B, n);
    init(C, n);
    ShraySync(B);
    ShraySync(C);

    double duration;
    size_t start = ShrayStart(n);
    size_t end = ShrayEnd(n);

    TIME(duration,
    for (size_t i = start; i < end; i++) {
        A[i] = B[i] + C[i] * SCALAR;
    }

    ShraySync(A););

    if (ShrayOutput) printf("%lf\n", 2 * 8 * n / duration / 1000000000.0);

    ShrayFree(A);
    ShrayFree(B);
    ShrayFree(C);

    return 0;
}
