#include "../util/csr.h"
#include "../util/time.h"
#include "../util/host.h"

#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <ga.h>
#include <mpi.h>
#include <sys/param.h>
#include <macdecls.h>
#include <cblas.h>

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
void matmul(int g_a, int g_b, int g_c, size_t n)
{
	int p = GA_Nnodes();
	int rank = GA_Nodeid();

	double *A, *C;
	double *Bt = malloc(n / p * n * sizeof(double));

	int lo_A[2], hi_A[2], ld_A[2];
	int lo_C[2], hi_C[2], ld_C[2];

	NGA_Distribution(g_a, rank, lo_A, hi_A);
	NGA_Distribution(g_c, rank, lo_C, hi_C);
	NGA_Access(g_a, lo_A, hi_A, &A, ld_A);
	NGA_Access(g_c, lo_C, hi_C, &C, ld_C);

	/* Add A[s][t] B[t] to C. */
	for (int t = 0; t < p; t++) {
		/* This is where the memory scalability comes from. We grab one block of
		 * B at a time, overwriting the block we are done with every time. */
		int lo_B[2] = {n / p * t, 0};
		int hi_B[2] = {n / p * (t + 1) - 1, n - 1};
		int ld_B[2] = {n, 0};
		NGA_Get(g_b, lo_B, hi_B, Bt, ld_B);

		/* blas can operate on subarrays, so no need to pack A[s][t].
		 * A[s][t] is an n / p x n / p matrix, and a submatrix of A which
		 * has leading dimension n. B[t] is a n / p x n matrix, and C is
		 * an n / p x n matrix. */
		cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
		        n / p, n, n / p, 1.0, &A[t * n / p], n, Bt, n, 1.0, C, n);
	}

	free(Bt);
	NGA_Release(g_a, lo_A, hi_A);
	NGA_Release(g_c, lo_C, hi_C);
}

int main(int argc, char **argv)
{
	int heap = 400000000;
	int stack = 400000000;

	MPI_Init(&argc,&argv);
	GA_Initialize();

	if (argc != 2) {
		GA_Error("Usage: n\n", 1);
	}

	/* Initialize the global allocator */
	if (!MA_init(C_DBL, stack, heap)) {
		GA_Error("MA_init failed", 1);
	}

	size_t n = atoll(argv[1]);
	int nprocs = GA_Nnodes();

	int dimensions[2] = { n, n };
	int chunks[2] = { n / nprocs, n };
	int g_a = NGA_Create(C_DBL, 2, dimensions, "A", chunks);
	if (!g_a) {
		GA_Error("Could not allocate matrix A", 1);
	}
	int g_b = NGA_Duplicate(g_a, "B");
	if (!g_b) {
		GA_Error("Could not allocate matrix B", 1);
	}
	int g_c = NGA_Duplicate(g_a, "C");
	if (!g_c) {
		GA_Error("Could not allocate matrix C", 1);
	}

	double one = 1.0;
	NGA_Fill(g_a, &one);
	NGA_Fill(g_b, &one);
	NGA_Zero(g_c);
	GA_Sync();

	double duration;
	TIME(duration, matmul(g_a, g_b, g_c, n); GA_Sync(););

	if (GA_Nodeid() == 0) {
	    printf("%lf\n", 2.0 * n * n * n / 1000000000 / duration);
	}
	hostname_print();

	GA_Destroy(g_a);
	GA_Destroy(g_b);
	GA_Destroy(g_c);
	GA_Terminate();
	MPI_Finalize();

	return EXIT_SUCCESS;
}
