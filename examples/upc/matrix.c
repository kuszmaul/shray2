#include <upc_relaxed.h>
#include <cblas.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


#define TIME(duration, fncalls)                                        \
    {                                                                  \
        struct timeval tv1, tv2;                                       \
        gettimeofday(&tv1, NULL);                                      \
        fncalls                                                        \
        gettimeofday(&tv2, NULL);                                      \
        duration = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +    \
         (double) (tv2.tv_sec - tv1.tv_sec);                           \
    }

int main()
{
    printf("hello world\n");
    return 0;
}

///* Initializes n x n matrix to value. */
//void init(double *matrix, size_t n, double value)
//{
//    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
//        for (size_t j = 0; j < n; j++) {
//            matrix[i * n + j] = value;
//        }
//    }
//}
//
//void matmul(double *A, double *B, double *C, size_t n)
//{
//}
//
///* If A, B are all one's, C should be n at every entry. */
//int check(double *C, size_t n, double epsilon)
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
//    double *A = ShrayMalloc(n, n * n * sizeof(double));
//    double *B = ShrayMalloc(n, n * n * sizeof(double));
//    double *C = ShrayMalloc(n, n * n * sizeof(double));
//
//
//
//    ShraySync(B);
//
//    double duration;
//
//    TIME(duration, matmul(A, B, C, n); ShraySync(C););
//
//    if (ShrayOutput) {
//        printf("%lf\n", 2.0 * n * n * n / 1000000000 / duration);
//    }
//
//    ShrayReport();
//
//    if (check(C, n, 0.01)) {
//        fprintf(stderr, "Success!\n");
//    } else {
//        fprintf(stderr, "Failure!\n");
//    }
//
//    ShrayFree(A);
//    ShrayFree(B);
//    ShrayFree(C);
//
//    ShrayFinalize(0);
//
//    exit(EXIT_SUCCESS);
//}
