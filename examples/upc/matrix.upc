#include <upc.h>
#include <upc_tick.h>
#include <stdio.h>
#include <stdlib.h>
#include <cblas.h>
#include <sys/time.h>

#define TIME(duration, fncalls)                                        \
    {                                                                  \
        upc_barrier;                                                   \
        upc_tick_t start = upc_ticks_now();                            \
        fncalls                                                        \
        upc_barrier;                                                   \
        upc_tick_t end = upc_ticks_now();                              \
        duration = upc_ticks_to_ns(end - start) / 1000000000.0;        \
    }

void init(double *matrix, size_t n, double value)
{
	for (size_t i = 0; i < n / THREADS; i++) {
		for (size_t j = 0; j < n; j++) {
			matrix[i * n + j] = value;
		}
	}
}

void initParallel(shared double *matrix, size_t n, double value)
{
    size_t start = MYTHREAD * n / THREADS;
    size_t end = (MYTHREAD + 1) * n / THREADS;
	for (size_t i = start; i < end; i++) {
		for (size_t j = 0; j < n; j++) {
			matrix[i * n + j] = value;
		}
    }
}

/* Assumes A, B, C are distributed blockwise along the first dimension.
 *
 * We use that
 * (    A[0]  )      (   A[0]B   )
 * (    ...   ) B =  (   ...     )
 * ( A[p - 1] )      ( A[p - 1]B )
 *
 * where A[s] is the sth block of A, so the part owned by processor s (P(s)).
 * So P(s) has to calculate A[s] B.
 *
 * To ensure memory-scalability, we use
 *
 *                                     (   B[0]   )
 * A[s]B = ( A[s][0] ... A[s][p - 1] ) (   ...    )
 *                                     ( B[p - 1] )
 *
 * where A[s][t] means the sth block row-wise of A, and the tth block column-wise.
 *
 * Assumes C is initialised to 0. We do not need to communicate on A, C. */
void matmul(double *A, shared double *B, double *C, size_t n)
{
	int p = THREADS;

    double *Bt = malloc(n / p * n * sizeof(double));

    /* Add A[s][t] B[t] to C. */
    for (int t = 0; t < p; t++) {
        /* This is where the memory scalability comes from. We grab one block of
         * B at a time, overwriting the block we are done with every time. */
        upc_memget(Bt, &B[t], n / p * n * sizeof(double));

        /* blas can operate on subarrays, so no need to pack A[s][t].
         * A[s][t] is an n / p x n / p matrix, and a submatrix of A which
         * has leading dimension n. B[t] is a n / p x n matrix, and C is
         * an n / p x n matrix. */
        cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                n / p, n, n / p, 1.0, &A[t * n / p], n, Bt, n, 1.0, C, n);
    }

    free(Bt);
}

/* If A, B are all one's, C should be n at every entry. */
int check(double *C, size_t n, double epsilon)
{
	for (size_t i = 0; i < n / THREADS; i++) {
		for (size_t j = 0; j < n; j++) {
			if ((C[i * n + j] - n) * (C[i * n + j] - n) > epsilon) {
                printf("C[%zu, %zu] = %lf != %lf\n", i, j, C[i * n + j], (double)n);
				return 0;
			}
		}
	}

	return 1;
}

int main(int argc, char **argv)
{
	if (argc != 2) {
	    printf("Takes one command-line argument, size of matrix.\n");
	    exit(EXIT_FAILURE);
	}

	size_t n = atol(argv[1]);
    if (n % THREADS != 0) {
        printf("Please make sure the number of processors divides n. Suggestion: n = %zu.\n",
                n / THREADS * THREADS);
    }

	double *A = malloc(n / THREADS * n * sizeof(double));
	shared double *B = upc_all_alloc(THREADS, n / THREADS * n * sizeof(double));
	double *C = calloc(n / THREADS * n, sizeof(double));

	init(A, n, 1);
	initParallel(B, n, 1);
    upc_barrier;

    double duration;

	TIME(duration, matmul(A, B, C, n); upc_barrier;);

    if (MYTHREAD == 0) {
        printf("%lf\n", 2.0 * n * n * n / 1000000000 / duration);
    }

	if (check(C, n, 0.01)) {
	    fprintf(stderr, "Success!\n");
	} else {
	    fprintf(stderr, "Failure!\n");
	}

    free(A);
    free(C);
    if (MYTHREAD == 0) {
        upc_free(B);
    }

	exit(EXIT_SUCCESS);
}
