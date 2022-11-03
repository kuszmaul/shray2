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
	unsigned int p = ShraySize();
	unsigned int s = ShrayRank();

    /* Add A[s][t] B[t] to C. We start with t = s, and repeat the 
     * asynchronous get B[t + 1] - compute A[s][t] B[t] cycle. */

    ShrayGet(B + n / p * n * ((s + 1) % p), n / p * n * sizeof(double));
    /* A[s][t] is a n / p x n / p matrix, B[t] an n / p x n matrix. So 
     * for the dgemm routine m = n / p, k = n / p, n = n. As B[t], C[t] are 
     * contiguous, we do not have to treat them as submatrices. We treat 
     * A[s][t] as a submatrix of A[s] (of size n / p x n). */
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
            n / p, n, n / p, 1.0, &A[s * n / p * n + s * n / p], n,
            &B[s * n / p * n], n, 0.0, &C[s * n / p * n], n); 
            
    for (unsigned int i = 1; i < p; i++) {
        /* The block we calculate */
        int t = (i + s) % p;

        ShrayGetComplete(B + n / p * n * t);

        /* Get the next block */
        if ((t + 1) % p != s) {
            ShrayGet(B + n / p * n * ((t + 1) % p), n / p * n * sizeof(double));
        }

        cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                n / p, n, n / p, 1.0, &A[s * n / p * n + t * n / p], n,
                &B[t * n / p * n], n, 1.0, &C[s * n / p * n], n); 

        ShrayGetFree(B + n / p * n * t);
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
        fprintf(stderr, "For now only supports dimension divisible by number of nodes.\n");
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
        printf("Time %lf, %lf Gflops/s\n", duration, 2.0 * n * n * n / 1000000000 / duration);
    }

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
