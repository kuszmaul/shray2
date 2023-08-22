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

void spmv(csr_t *matrix, int g_vector, int g_out)
{
	int rank = GA_Nodeid();
	int lo_out[1], hi_out[1], ld_out[1];
	double *out;
	double value;

	NGA_Distribution(g_out, rank, lo_out, hi_out);
	NGA_Access(g_out, lo_out, hi_out, &out, ld_out);

	for (int i = lo_out[0]; i <= hi_out[0]; ++i) {
		double outval = 0;

		int offset = i - lo_out[0];
		size_t row_start = matrix->row_indices[offset];
		size_t row_end = matrix->row_indices[offset + 1];
		for (size_t j = row_start; j < row_end; ++j) {
			size_t col = matrix->col_indices[j];
			double v = matrix->values[j];

			int lo_val[1] = { col };
			int hi_val[1] = { col };
			int ld_val[1] = { 0 };

			NGA_Get(g_vector, lo_val, hi_val, &value, ld_val);

			outval += v * value;
		}

		out[offset] = outval;
	}

	NGA_Release(g_out, lo_out, hi_out);
}

void steady_state(csr_t *matrix, int g_vector, int g_out, size_t iterations)
{
	for (size_t i = 0; i < iterations; ++i) {
		spmv(matrix, g_vector, g_out);
		GA_Sync();
		int tmp = g_vector;
		g_vector = g_out;
		g_out = tmp;
	}
}

int main(int argc, char **argv)
{
	int heap = 3000000;
	int stack = 3000000;

	MPI_Init(&argc,&argv);
	GA_Initialize();

	if (argc != 3) {
		GA_Error("Usage: FILE iterations\n", 1);
	}

	/* Initialize the global allocator */
	if (!MA_init(C_DBL, stack, heap)) {
		GA_Error("MA_init failed", 1);
	}

	size_t n = atoll(argv[1]);
	size_t iterations = atoll(argv[2]);

	csr_t *matrix = monopoly(n, GA_Nnodes(), GA_Nodeid());
	if (!matrix) {
		fprintf(stderr, "Could not generate matrix\n");
        exit(EXIT_FAILURE);
	}

	int v_dimensions[1] = { matrix->n };
	int g_vector = NGA_Create(C_DBL, 1, v_dimensions, "input vector", NULL);
	if (!g_vector) {
		GA_Error("Could not allocate input array", 1);
	}

	int g_out = NGA_Duplicate(g_vector, "output vector");
	if (!g_out) {
		GA_Error("Could not allocate output array", 1);
	}

	double one = 1.0;
	GA_Fill(g_vector, &one);
	GA_Sync();

	double duration;
	TIME(duration, steady_state(matrix, g_vector, g_out, iterations););

	if (GA_Nodeid() == 0) {
	    printf("%lf\n", matrix->nnz_total * iterations * 2.0 / 1000000000 / duration);
	}
	hostname_print();

	GA_Destroy(g_vector);
	GA_Destroy(g_out);
	csr_free(matrix);
	GA_Terminate();
	MPI_Finalize();

	return EXIT_SUCCESS;
}
