#include <shray2/shray.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <cblas.h>

#ifdef __cplusplus
}
#endif

#include <stdio.h>
#include <stdlib.h>

/* Initializes n x n matrix to value. */
void init(double *matrix, size_t n, double value)
{
    for (size_t i = ShrayStart(matrix); i < ShrayEnd(matrix); i++) {
        for (size_t j = 0; j < n; j++) {
            matrix[i * n + j] = value;
        }
    }
}

void matmul(double *A, double *B, double *C, size_t n)
{
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
            ShrayEnd(C) - ShrayStart(C), n, n, 1.0, A + ShrayStart(C) * n, n,
            B, n, 0.0, C + ShrayStart(C) * n, n);
}

/* If A, B are all one's, C should be n at every entry. */
int check(double *C, size_t n, double epsilon)
{
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            if ((C[i * n + j] - n) * (C[i * n + j] - n) > epsilon) {
                printf("Failure at (%zu, %zu): %lf\n", i, j, C[i * n + j]);
                return 0;
            }
        }
    }

    return 1;
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 2) {
        printf("Takes one command-line argument, size of matrix.\n");
        ShrayFinalize(1);
    }

    size_t n = atol(argv[1]);
    if (n % ShraySize() != 0) {
        fprintf(stderr, "For only supports dimension divisible by number of nodes.\n");
        ShrayFinalize(1);
    }

    double *A = (double *)ShrayMalloc(n, n * n * sizeof(double));
    double *B = (double *)ShrayMalloc(n, n * n * sizeof(double));
    double *C = (double *)ShrayMalloc(n, n * n * sizeof(double));

    init(A, n, 1);
    init(B, n, 1);
    ShraySync(A);
    ShraySync(B);

    double duration;

    TIME(duration, matmul(A, B, C, n); ShraySync(C););

    if (ShrayOutput) {
        printf("%lf\n", 2.0 * n * n * n / 1000000000 / duration);
    }

    ShrayReport();

    //if (check(C, n, 0.01)) {
    //    fprintf(stderr, "Success!\n");
    //} else {
    //    fprintf(stderr, "Failure!\n");
    //}

    ShrayFree(A);
    ShrayFree(B);
    ShrayFree(C);

    ShrayFinalize(0);

    exit(EXIT_SUCCESS);
}
