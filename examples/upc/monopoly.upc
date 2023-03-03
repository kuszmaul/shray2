#include <upc.h>
#include <upc_tick.h>
#include "../util/csr.h"
#include "../util/host.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
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

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

void init(shared double *a, size_t n)
{
    size_t blockSize = (n + THREADS - 1) / THREADS;
    size_t start = MYTHREAD * blockSize;
    size_t end = MAX((MYTHREAD + 1) * blockSize, n);

	for (size_t i = start; i < end; ++i) {
		a[i] = i;
	}
}

void spmv(csr_t *matrix, shared double *vector, shared double *out, size_t n)
{
    size_t blockSize = (matrix->n + THREADS - 1) / THREADS;
    size_t start = MYTHREAD * blockSize;
    size_t end = MIN((MYTHREAD + 1) * blockSize, n);

	for (size_t i = start, k = 0; i < end; i++, k++) {
		double outval = 0;

		size_t row_start = matrix->row_indices[k];
		size_t row_end = matrix->row_indices[k + 1];
		for (size_t j = row_start; j < row_end; ++j) {
			size_t col = matrix->col_indices[j];
			double v = matrix->values[j];

			outval += v * vector[col];
		}

		out[i] = outval;
	}
}

void steady_state(csr_t *matrix, shared double *vector, shared double *out,
        size_t n, size_t iterations)
{
	for (size_t i = 0; i < iterations; ++i) {
		spmv(matrix, vector, out, n);
        upc_barrier;
		shared double *tmp = vector;
		vector = out;
		out = tmp;
	}
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "Usage: n iterations\n");
        exit(EXIT_FAILURE);
	}

    size_t n = atoll(argv[1]);
    size_t iterations = atoll(argv[2]);

	csr_t *matrix = monopoly(n, THREADS, MYTHREAD);
	if (!matrix) {
		fprintf(stderr, "Could not generate matrix\n");
        exit(EXIT_FAILURE);
	}

    size_t blockSizeIn = (matrix->n + THREADS - 1) / THREADS * sizeof(double);
    size_t blockSizeOut = (matrix->m_total + THREADS - 1) / THREADS * sizeof(double);
    shared double *vector = upc_all_alloc(THREADS, blockSizeIn);
    shared double *out = upc_all_alloc(THREADS, blockSizeOut);

	init(vector, matrix->n);
    upc_barrier;

	double duration;

	TIME(duration, steady_state(matrix, vector, out, matrix->n, iterations););

	hostname_print();
	if (MYTHREAD == 0) {
	    printf("%lf\n", matrix->nnz_total * 2.0 * iterations / 1000000000 / duration);
	}

    if (MYTHREAD == 0) {
        upc_free(vector);
        upc_free(out);
    }

	return EXIT_SUCCESS;
}
