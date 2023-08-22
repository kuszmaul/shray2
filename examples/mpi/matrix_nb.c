/*****************************************************************************
 * A simple implementation of a distributed matrix-matrix multiply,
 * with unblocking broadcasts, overlapping communication and computation.
 * We use a in theory suboptimal, but for our cluster better distribution of
 * A, B, C blockwise along the first dimension. This looks like
 *
 * | A11 ... A1p | | B1  |    | sum_t A1t Bt |
 * |     ...     | | ... |  = |     ...      |
 * | Ap1 ... App | | Bp  |    | sum_t Apt Bt |
 *
 * At P(s) we compute sum_t Ast Bt as follows:
 *
 * broadcast B1 unblocking
 * for t = 1 to p - 1
 *     start unblocking broadcast Bt+1
 *     wait for broadcast Bt
 *     Cs += Ast Bt
 * Cs += Asp Bp
 ****************************************************************************/

#include <cblas.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>

#define MPI_SAFE(fn)                                            \
    do {                                                        \
        int resultlen;                                          \
        char string[MPI_MAX_ERROR_STRING];                      \
        int err = fn;                                           \
        if (err != MPI_SUCCESS) {                               \
            MPI_Error_string(fn, string, &resultlen);           \
            printf("Error %s in function call %s, line %d.\n",  \
                    string, #fn, __LINE__);                     \
            MPI_Abort(MPI_COMM_WORLD, 1);                       \
        }                                                       \
    } while (0)

size_t n;
int rank;
int p;

/* Initialising on one node and then broadcasting is stupid, it
 * kills memory scalability. If the matrix is generated, init in
 * parallel like this, otherwise use parallel file I/O, as in
 * Chapter 14 of the MPI standard 4.0. */
double f(int i, int j) {
    return 1;
}

void init(double *mat, double (*f)(int i, int j))
{
    for (size_t i = 0; i < n / p; i++) {
        for (size_t j = 0; j < n; j++) {
            mat[i * n + j] = f(rank * n + i, j);
        }
    }
}

void matmul(double *A, double *B, double *C)
{
    double *Bt = malloc(n / p * n * sizeof(double));

    for (int t = 0; t < p; t++) {
        double *b = (rank == t) ? B : Bt;
        MPI_SAFE(MPI_Bcast(b, n / p * n, MPI_DOUBLE, t, MPI_COMM_WORLD));
        cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, n / p,
                    n, n / p, 1.0, A + t * n / p, n, b, n, 1.0, C, n);
    }

//    double *Bt_next = malloc(n / p * n * sizeof(double));
//
//    MPI_Request request;
//    MPI_Request request_next;
//
//    MPI_SAFE(MPI_Ibcast(Bt, n / p * n, MPI_DOUBLE, 0, MPI_COMM_WORLD,
//                        &request));
//
//    for (int t = 0; t < p - 1; t++) {
//        MPI_SAFE(MPI_Ibcast(Bt_next, n / p * n, MPI_DOUBLE, t + 1,
//                            MPI_COMM_WORLD, &request_next));
//        MPI_SAFE(MPI_Wait(&request, MPI_STATUS_IGNORE));
//        cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, n / p,
//                    n, n / p, 1.0, A + t * n / p, n, Bt, n, 1.0, C, n);
//
//        MPI_Request temp_r = request;
//        request = request_next;
//        request_next = temp_r;
//        double *temp_b = Bt;
//        Bt = Bt_next;
//        Bt_next = temp_b;
//    }
//
//    MPI_SAFE(MPI_Wait(&request, MPI_STATUS_IGNORE));
//    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, n / p,
//                n, n / p, 1.0, A + (p - 1) * n / p, n, Bt, n, 1.0, C, n);
//
//    free(Bt);
//    free(Bt_next);
    free(Bt);
}

/* If A, B are all one's, C should be n at every entry. */
int check(double *C, double epsilon)
{
    for (size_t i = 0; i < n / p; i++) {
        for (size_t j = 0; j < n; j++) {
            if ((C[i * n + j] - n) * (C[i * n + j] - n) > epsilon) {
                printf("P(%d): Failure at (%zu, %zu): %lf\n",
                        rank, i, j, C[i * n + j]);
                return 0;
            }
        }
    }

    return 1;
}

int main(int argc, char **argv)
{
    MPI_SAFE(MPI_Init(&argc, &argv));

    if (argc != 2 && argc != 4) {
        printf("Takes one command-line argument, size of matrix.\n");
        MPI_SAFE(MPI_Finalize());
    }

    n = atoi(argv[1]);

    MPI_SAFE(MPI_Comm_size(MPI_COMM_WORLD, &p));
    MPI_SAFE(MPI_Comm_rank(MPI_COMM_WORLD, &rank));

    if (n % p != 0) {
        fprintf(stderr, "Only supports n divisible by %d\n", p);
        MPI_SAFE(MPI_Finalize());
    }

    /* Initalize A, B to 1, C to 0 */
    double *A = malloc(n / p * n * sizeof(double));
    double *B = malloc(n / p * n * sizeof(double));
    double *C = calloc(n / p * n, sizeof(double));

    init(A, &f);
    init(B, &f);

    /* Benchmark */
    MPI_SAFE(MPI_Barrier(MPI_COMM_WORLD));

    MPI_SAFE(MPI_Barrier(MPI_COMM_WORLD));
    double start = MPI_Wtime();
    matmul(A, B, C);
    MPI_SAFE(MPI_Barrier(MPI_COMM_WORLD));
    double stop = MPI_Wtime();
    double duration = stop - start;

    if (rank == 0) {
        printf("%lf\n", 2.0 * n * n * n / 1000000000 / duration);
    }

    /* Check whether C is n at every position */
    if (check(C, 0.00001)) {
        fprintf(stderr, "Success from P(%d)!\n", rank);
    } else {
        fprintf(stderr, "Failure from P(%d)!\n", rank);
    }

    free(A);
    free(B);
    free(C);

    MPI_SAFE(MPI_Finalize());
}
