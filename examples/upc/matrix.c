#include <upc_relaxed.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

main() {}

//
//
//#define TIME(duration, fncalls)                                        \
//    {                                                                  \
//        struct timeval tv1, tv2;                                       \
//        gettimeofday(&tv1, NULL);                                      \
//        fncalls                                                        \
//        gettimeofday(&tv2, NULL);                                      \
//        duration = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +    \
//         (double) (tv2.tv_sec - tv1.tv_sec);                           \
//    }
//
///* Initializes n x n matrix to value. */
//void init(shared double *matrix, size_t n, double value)
//{
//    /* With rows as units, not bytes as in main. */
//    size_t blockSize = (n + THREADS - 1) / THREADS;
//
//    for (size_t i = MYTHREAD * blockSize; i < (MYTHREAD + 1) * blockSize && i < n; i++) {
//        for (size_t j = 0; j < n; j++) {
//            matrix[i * n + j] = value;
//        }
//    }
//}
//
//void matmul(shared double *A, shared double *B, shared double *C, size_t n)
//{
//    unsigned int s = MYTHREAD;
//    unsigned int p = THREADS;
//
//    /* A[s][t] is a n / p x n / p matrix, B[t] an n / p x n matrix. So
//     * for the dgemm routine m = n / p, k = n / p, n = n. As B[t], C[t] are
//     * contiguous, we do not have to treat them as submatrices. We treat
//     * A[s][t] as a submatrix of A[s] (of size n / p x n). */
//    double *Ast = &A[s * n / p * n + s * n / p];
//    double *Bs = &B[s * n / p * n];
//    double *Cs = &C[s * n / p * n];
//
//    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
//            n / p, n, n / p, 1.0, As, n, Bs, n, 0.0, Cs, n);
//
//    for (unsigned int t = (s + 1) % p; t != s; t = (t + 1) % p) {
//        double *Ast = &A[s * n / p * n + t * n / p];
//        double *Bt = &B[t * n / p * n];
//
//        cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
//                n / p, n, n / p, 1.0, Ast, n,
//                Bt, n, 1.0, Cs, n);
//    }
//}
//
///* If A, B are all one's, C should be n at every entry. */
//int check(shared double *C, size_t n, double epsilon)
//{
//    for (size_t i = 0; i < n; i++) {
//        for (size_t j = 0; j < n; j++) {
//            if ((C[i * n + j] - n) * (C[i * n + j] - n) > epsilon) {
//                printf("Failure at (%zu, %zu): %lf\n", i, j, C[i * n + j]);
//                return 0;
//            }
//        }
//    }
//
//    return 1;
//}
//
//int main(int argc, char **argv)
//{
//    if (argc != 2) {
//        printf("Takes one command-line argument, size of matrix.\n");
//        exit(EXIT_FAILURE);
//    }
//
//    size_t n = atol(argv[1]);
//    if (n % THREADS != 0) {
//        fprintf(stderr, "For only supports dimension divisible by number of nodes.\n");
//        exit(EXIT_FAILURE);
//    }
//
//    size_t blockSize = (n * n + THREADS - 1) / THREADS * sizeof(double);
//
//    shared double *A = upc_all_alloc(THREADS, blockSize);
//    shared double *B = upc_all_alloc(THREADS, blockSize);
//    shared double *C = upc_all_alloc(THREADS, blockSize);
//
//    init(A, n, 1.0);
//    init(B, n, 1.0);
//
//    upc_barrier;
//
//    double duration;
//
//    TIME(duration, matmul(A, B, C, n); upc_barrier;);
//
//    if (MYTHREAD == 0) {
//        printf("%lf\n", 2.0 * n * n * n / 1000000000 / duration);
//    }
//
//    if (check(C, n, 0.01)) {
//        fprintf(stderr, "Success!\n");
//    } else {
//        fprintf(stderr, "Failure!\n");
//    }
//
//    /* Yes, no special free function. The allocation is alive until the first thread
//     * deallocates it (and potentially longer). */
//    upc_barrier;
//
//    exit(EXIT_SUCCESS);
//}
