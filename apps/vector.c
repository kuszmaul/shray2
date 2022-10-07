#include "../include/shray.h"

/* Initializes n x m matrix to its row-major index. */
void init(double *matrix, size_t n, size_t m)
{
    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        for (size_t j = 0; j < m; j++) {
            matrix[i * m + j] = i * m + j;
        }
    }
}

/* C = A + B */
void matmulAdd(double *A, double *B, double *C, size_t m, size_t n)
{    
    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        for (size_t j = 0; j < m; j++) {
            C[i * m + j] = A[i * m + j] + B[i * m + j];
        }
    }
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    size_t n = 1000;
    size_t m = 2000;

    double *A = ShrayMalloc(n, n * m * sizeof(double));
    double *B = ShrayMalloc(n, n * m * sizeof(double));
    double *C = ShrayMalloc(n, n * m * sizeof(double));

    init(A, n, m);
    init(B, n, m);
    ShraySync(A);
    ShraySync(B);

    matmulAdd(A, B, C, n, m);
    ShraySync(C);

    for (int i = 0; i < 10; i++) {
        printf("C[%d] = %lf. Should be %d\n", i, C[i], 2 * i);
    }

    ShrayFree(A);
    ShrayFree(B);

    ShrayFinalize();
}
