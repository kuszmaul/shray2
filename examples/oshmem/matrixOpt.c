#include <shmem.h>
#include <stdio.h>
#include <stdlib.h>
#include <cblas.h>
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

void init(double *matrix, size_t n, double value)
{
	for (size_t i = 0; i < n / shmem_n_pes(); i++) {
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
 * Assumes C is initialised to 0. */
void matmul(double *A, double *B, double *C, size_t n)
{
	int p = shmem_n_pes();

    double *Bt = malloc(n / p * n * sizeof(double));
    double *BtAsync = malloc(n / p * n * sizeof(double));

    /* Add A[s][t] B[t] to C. We start with t = 0 (B[0] = B), and repeat the 
     * asynchronous get B[t + 1] - compute A[s][t] B[t] cycle. */

    shmem_get_nbi(BtAsync, B, n / p * n, (shmem_my_pe() + 1) % p);
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
            n / p, n, n / p, 1.0, &A[0 * n / p], n, B, n, 1.0, C, n);

    for (int i = 1; i < p; i++) {
        /* Transfer the async get into Bt. */
        shmem_quiet();
        double *temp = Bt;
        Bt = BtAsync;
        BtAsync = temp;

        /* Get the next block */
        int t = (i + shmem_my_pe()) % p;
        if (t != (shmem_my_pe() - 1) % p) {
            shmem_get_nbi(BtAsync, B, n / p * n, (t + 1) % p);
        }

        /* blas can operate on subarrays, so no need to pack A[s][t]. 
         * A[s][t] is an n / p x n / p matrix, and a submatrix of A which 
         * has leading dimension n. B[t] is a n / p x n matrix, and C is 
         * an n / p x n matrix. */
        cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                n / p, n, n / p, 1.0, &A[t * n / p], n, Bt, n, 1.0, C, n);
    }

    free(BtAsync);
    free(Bt);
}

/* If A, B are all one's, C should be n at every entry. */
int check(double *C, size_t n, double epsilon)
{
	for (size_t i = 0; i < n / shmem_n_pes(); i++) {
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

	shmem_init();

	size_t n = atol(argv[1]);
    if (n % shmem_n_pes() != 0) {
        printf("Please make sure the number of processors divides n. Suggestion: n = %zu.\n",
                n / shmem_n_pes() * shmem_n_pes());
    }

	double *A = shmem_malloc(n / shmem_n_pes() * n * sizeof(double));
	double *B = shmem_malloc(n / shmem_n_pes() * n * sizeof(double));
	double *C = shmem_malloc(n / shmem_n_pes() * n * sizeof(double));

	init(A, n, 1);
	init(B, n, 1);
	init(C, n, 0);
    shmem_barrier_all();

    double duration;

	TIME(duration, matmul(A, B, C, n); shmem_barrier_all(););

    if (shmem_my_pe() == 0) {
        printf("Time %lf, %lf Gflops/s\n", duration, 2.0 * n * n * n / 1000000000 / duration);
    }

	if (check(C, n, 0.01)) {
	    printf("Success!\n");
	} else {
	    printf("Failure!\n");
	}

	shmem_free(A);
	shmem_free(B);
	shmem_free(C);

	shmem_finalize();

	exit(EXIT_SUCCESS);
}
