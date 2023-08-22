/*****************************************************************************
 * A simple implementation of a distributed matrix-matrix multiply, using
 * the SUMMA algorithm.
 * We divide the number of processors in a p x q grid, and take r
 * such that p, q | r.
 * We then block C := AB as p x q, A as p x r, B as r x q.
 * We use a block distribution for A, and B. So let b_p = r / p,
 * b_q = r / q. Then A[i, l] is owned by P(i, l / b_q) and
 * B[l, j] by P(l / b_p, j), C[i, j] by P(i, j).
 *
 * We then use algorithm (from the perspective of P(i, j))
 *
 * C[i, j] = 0
 * for (l = 0; l < r; l++) {
 *     Broadcast A[i, l] to P(i, 0), ... P(i, r - 1) from its owner.
 *     Broadcast B[l, j] to P(0, j), ... P(r - 1, j) from its owner.
 *     C[i, j] += A[i, l] * B[l, j]
 * }
 *
 * This can be optimised by using a unblocking broadcast and overlapping it
 * with A[i, l] * B[l, j].
 *
 * Example for a [2, 3] processor grid (r = 6), where (ij) is owned by
 * P(i, j):
 *
 * A = | (00) | (00) | (01) | (01) | (02) | (02) |
 *     | (10) | (10) | (11) | (11) | (12) | (12) |
 *
 * B = | (00) | (01) | (02) |
 *     | (00) | (01) | (02) |
 *     | (00) | (01) | (02) |
 *     | (10) | (11) | (12) |
 *     | (10) | (11) | (12) |
 *     | (10) | (11) | (12) |
 *
 *                 B00 B01 B02 B03
 *                 B10 B11 B12 B13
 * A0 A1 A2 A3     B20 B21 B22 B23 =
 *                 B30 B31 B32 B33
 *
 *  All the Bst are contiguous in memory, which the As are not
 *
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
int me[2];
int rank;
int p;
int q;
int r;
MPI_Comm comm_cart;
MPI_Comm row_comm;
MPI_Comm col_comm;

/* Initialising on one node and then broadcasting is stupid, it
 * kills memory scalability. If the matrix is generated, init in
 * parallel like this, otherwise use parallel file I/O, as in
 * Chapter 14 of the MPI standard 4.0. */
double f(int i, int j) {
    return 1;
}

void init(double *mat, double (*f)(int i, int j))
{
    /* In scalars, all matrices are locally n / p x n / q */
    for (size_t i = 0; i < n / p; i++) {
        for (size_t j = 0; j < n / q; j++) {
            mat[i * n / q + j] = f(me[0] * n / p + i, me[1] * n / q + j);
        }
    }
}

void matmul(double *A, double *B, double *C)
{
    /* We have row-major format, so Ail is not contiguous in memory,
     * but Blj is. */
    double *Ail = malloc(n / p * n / r * sizeof(double));
    double *Blj = malloc(n / r * n / q * sizeof(double));

    /* We could also store A in column-major to avoid this problem. */
    MPI_Datatype A_type;
    const int big_sizes[2] = {n / p, n / q};
    const int sub_sizes[2] = {n / p, n / r};
    const int start[2] = {0, 0};
    MPI_SAFE(MPI_Type_create_subarray(2, big_sizes, sub_sizes, start,
                                      MPI_ORDER_C, MPI_DOUBLE, &A_type));
    MPI_SAFE(MPI_Type_commit(&A_type));

    int b_p = r / p;
    int b_q = r / q;

    for (int l = 0; l < r; l++) {
        /* At the root of the broadcast, we do not copy out the data,
         * and at the root A[i, l] is not contiguous in memory, so we
         * introduce some variables for the case distinction. */
        int root_a = l / b_p;
        int root_b = l / b_q;
        double *a = (me[0] == root_a) ? A + l % b_q * n / r : Ail;
        double *b = (me[1] == root_b) ? B + l % b_p * n / r * n / q : Blj;
        int lda = (me[0] == root_a) ? n / q : n / r;
        MPI_Datatype a_type = (me[0] == root_a) ? A_type : MPI_DOUBLE;
        int a_count = (me[0] == root_a) ? 1 : n / p * n / r;

        MPI_SAFE(MPI_Bcast(a, a_count, a_type, root_a, col_comm));
        MPI_SAFE(MPI_Bcast(b, n / r * n / q, MPI_DOUBLE, root_b, row_comm));

        cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                    n / p, n / q, n / r, 1.0, a, lda,
                    b, n / q, 1.0, C, n / q);
    }

    free(Ail);
    free(Blj);
}

/* If A, B are all one's, C should be n at every entry. */
int check(double *C, double epsilon)
{
    for (size_t i = 0; i < n / p; i++) {
        for (size_t j = 0; j < n / q; j++) {
            if ((C[i * n / q + j] - n) * (C[i * n / q + j] - n) > epsilon) {
                printf("P(%d, %d): Failure at (%zu, %zu): %lf\n",
                        me[0], me[1], i, j, C[i * n / q + j]);
                return 0;
            }
        }
    }

    return 1;
}

int gcd(int p, int q)
{
    if (q == 0) {
        return p;
    } else {
        return gcd(q, p % q);
    }
}

int lcm(int p, int q)
{
    return p * q / gcd(p, q);
}

int main(int argc, char **argv)
{
    MPI_SAFE(MPI_Init(&argc, &argv));

    if (argc != 2 && argc != 4) {
        printf("Takes one command-line argument, size of matrix.\n"
               "Optionally specify the processor grid after.\n");
        MPI_SAFE(MPI_Finalize());
    }

    n = atoi(argv[1]);

    /* Setting up p x q processor grid */
    int size;
    MPI_SAFE(MPI_Comm_size(MPI_COMM_WORLD, &size));
    MPI_SAFE(MPI_Comm_rank(MPI_COMM_WORLD, &rank));

    if (argc != 4) {
        for (int divisor = 1; divisor * divisor <= size; divisor++) {
            if (size % divisor == 0) p = divisor;
        }
        q = size / p;
    } else {
        p = atoi(argv[2]);
        q = atoi(argv[3]);
    }
    r = lcm(p, q);

    const int dims[2] = {p, q};
    const int periods[2] = {false, false};
    MPI_SAFE(MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods,
                             true, &comm_cart));
    MPI_SAFE(MPI_Cart_coords(comm_cart, rank, 2, me));

    if (rank == 0) {
        fprintf(stderr, "Using processor grid %d x %d and %d for the shared "
                    "dimension.\n", p, q, r);
    }

    const int remain_dims1[2] = {true, false};
    MPI_SAFE(MPI_Cart_sub(comm_cart, remain_dims1, &row_comm));
    const int remain_dims2[2] = {false, true};
    MPI_SAFE(MPI_Cart_sub(comm_cart, remain_dims2, &col_comm));

    MPI_SAFE(MPI_Comm_rank(row_comm, me));
    MPI_SAFE(MPI_Comm_rank(col_comm, me + 1));

    if (n % p != 0 || n % q != 0) {
        fprintf(stderr, "Only supports n divisible by %d and %d\n", p, q);
        MPI_SAFE(MPI_Finalize());
    }

    int col_size, row_size;
    MPI_SAFE(MPI_Comm_size(col_comm, &col_size));
    MPI_SAFE(MPI_Comm_size(row_comm, &row_size));
    printf("Row comm is %d, col comm is %d\n", row_size, col_size);

    /* Initalize A, B to 1, C to 0 */
    double *A = malloc(n / p * n / q * sizeof(double));
    double *B = malloc(n / p * n / q * sizeof(double));
    double *C = calloc(n / p * n / q, sizeof(double));

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
        fprintf(stderr, "Success from P(%d, %d)!\n", me[0], me[1]);
    } else {
        fprintf(stderr, "Failure from P(%d, %d)!\n", me[0], me[1]);
    }

    free(A);
    free(B);
    free(C);

    MPI_SAFE(MPI_Finalize());
}
