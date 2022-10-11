#include <shmem.h>
#include <stdio.h>
#include <stdlib.h>
#include <cblas.h>

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

    /* Add A[s][t] B[t] to C. */
    for (int t = 0; t < p; t++) {
        /* This is where the memory scalability comes from. We grab one block of 
         * B at a time, overwriting the block we are done with every time. */
        shmem_get(Bt, B, n / p * n, t);

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

	double *A = shmem_malloc(n / shmem_n_pes() * n * sizeof(double));
	double *B = shmem_malloc(n / shmem_n_pes() * n * sizeof(double));
	double *C = shmem_malloc(n / shmem_n_pes() * n * sizeof(double));

	init(A, n, 1);
	init(B, n, 1);
	init(C, n, 0);
    shmem_barrier_all();

	matmul(A, B, C, n);
    shmem_barrier_all();

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
