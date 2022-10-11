#include <shray2/shray.h>
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

    /* Calculates C(start:end,:) = A(start:end,:) B. */
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
            end - start, n, n, 1.0, A + start * n,
            n, B, n, 0.0, C + start * n, n);
}

/* If A, B are all one's, C should be n at every entry. */
int check(double *C, size_t n, double epsilon)
{
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            if ((C[i * n + j] - n) * (C[i * n + j] - n) > epsilon) {
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

    matmul(A, B, C, n);
    ShraySync(C);

    ShrayReport();

    if (check(C, n, 0.01)) {
        printf("Success!\n");
    } else {
        printf("Failure!\n");
    }

    ShrayFree(A);
    ShrayFree(B);
    ShrayFree(C);

    ShrayFinalize(0);

    exit(EXIT_SUCCESS);
}
