#include <shray2/shray.h>
#include <cblas.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    /* Size of matrices A, B, C. */
    size_t n;
    double *A;
    double *B;
    double *C;
    unsigned int t;
} matrix_t;

/* Initializes n x n matrix to value. */
void init(double *matrix, size_t n, double value)
{
    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        for (size_t j = 0; j < n; j++) {
            matrix[i * n + j] = value;
        }
    }
}

/* This function adds A[start:end,t] B[t] to C[start:end, :]. */
void matmul_mt(worker_info_t *info)
{
    matrix_t *arguments = (matrix_t *)info->args;
    size_t n = arguments->n;
    double *A = arguments->A;
    double *B = arguments->B;
    double *C = arguments->C;
    unsigned int t = arguments->t;

    unsigned int p = ShraySize();
    size_t start = info->start;
    size_t end = info->end;

    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
            end - start, n, n / p, 1.0, &A[start * n + t * n / p], n,
            &B[t * n / p * n], n, 1.0, &C[start * n], n);
}

void matmul(double *A, double *B, double *C, size_t n)
{
	unsigned int p = ShraySize();
	unsigned int s = ShrayRank();

    /* Add A[s][t] B[t] to C. We start with t = s, and repeat the
     * asynchronous get B[t + 1] - compute A[s][t] B[t] cycle. */

    ShrayPrefetch(B + n / p * n * ((s + 1) % p), n / p * n * sizeof(double));
    /* A[s][t] is a n / p x n / p matrix, B[t] an n / p x n matrix. So
     * for the dgemm routine m = n / p, k = n / p, n = n. As B[t], C[t] are
     * contiguous, we do not have to treat them as submatrices. We treat
     * A[s][t] as a submatrix of A[s] (of size n / p x n). */
    matrix_t matrixInfo;
    matrixInfo.n = n;
    matrixInfo.A = A;
    matrixInfo.B = B;
    matrixInfo.C = C;
    matrixInfo.t = s;

    ShrayRunWorker(matmul_mt, n, &matrixInfo);

    for (unsigned int t = (s + 1) % p; t != s; t = (t + 1) % p) {
        /* Get the next block */
        if ((t + 1) % p != s) {
            ShrayPrefetch(B + n / p * n * ((t + 1) % p), n / p * n * sizeof(double));
        }

        matrixInfo.t = t;
        ShrayRunWorker(matmul_mt, n, &matrixInfo);

        ShrayDiscard(B + n / p * n * t, n / p * n * sizeof(double));
    }
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

    double *A = ShrayMalloc(n, n * n * sizeof(double));
    double *B = ShrayMalloc(n, n * n * sizeof(double));
    double *C = ShrayMalloc(n, n * n * sizeof(double));

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

    if (check(C, n, 0.01)) {
        fprintf(stderr, "Success!\n");
    } else {
        fprintf(stderr, "Failure!\n");
    }

    ShrayFree(A);
    ShrayFree(B);
    ShrayFree(C);

    ShrayFinalize(0);

    exit(EXIT_SUCCESS);
}
