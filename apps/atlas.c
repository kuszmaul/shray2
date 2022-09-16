#include "../include/shray.h"
#include <cblas.h>

/* Initializes n x n matrix to value. */
void init(double *matrix, size_t n, double value)
{
    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        for (size_t j = 0; j < n; j++) {
            matrix[i * n + j] = value;
        }
    }
}

void matmul(double *A, double *B, double *C, size_t n)
{
    size_t start = ShrayStart(n);
    size_t end = ShrayEnd(n);

    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 
            end - start, n, n, 1.0, A + start * n, 
            n, B, n, 0.0, C + start * n, n);
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv, 8 * 1 * 4096000 / 4);

    if (argc != 2) {
        printf("Takes one command-line argument, size of matrix.\n");
        exit(EXIT_FAILURE);
    }

    size_t n = atol(argv[1]);

    double *A = ShrayMalloc(n, n * n * sizeof(double));
    double *B = ShrayMalloc(n, n * n * sizeof(double));
    double *C = ShrayMalloc(n, n * n * sizeof(double));

    init(A, n, 1);
    init(B, n, 1);
    ShraySync(A);
    ShraySync(B);

    SHRAY_TIME(matmul(A, B, C, n));
    ShraySync(C);

    ShrayFree(A);
    ShrayFree(B);
    ShrayFree(C);

    ShrayReport();

    ShrayFinalize();

    exit(EXIT_SUCCESS);
}
