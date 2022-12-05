#include <shray2/shray.h>
#include "../util/csr.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

void init(double *a, size_t n)
{
	for (size_t i = ShrayStart(n); i < ShrayEnd(n); ++i) {
		a[i] = i;
	}
}

void spmv(csr_t *matrix, double *vector, double *out, size_t n)
{
	for (size_t i = ShrayStart(matrix->m_total), k = 0; i < ShrayEnd(matrix->m_total); ++i, ++k) {
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

void steady_state(csr_t *matrix, double *vector, double *out, size_t n, size_t iterations)
{
	for (size_t i = 0; i < iterations; ++i) {
		spmv(matrix, vector, out, n);
		ShraySync(out);
		double *tmp = vector;
		vector = out;
		out = tmp;
	}
}

int main(int argc, char **argv)
{
	ShrayInit(&argc, &argv);

	if (argc != 4) {
		fprintf(stderr, "Usage: FILE n iterations\n");
		ShrayFinalize(1);
	}

	csr_t *matrix = csr_parse_local(argv[1], ShrayRank());
	if (!matrix) {
		fprintf(stderr, "Could not parse matrix for %s\n", argv[1]);
		ShrayFinalize(1);
	}

	size_t n = atoll(argv[2]);
	size_t iterations = atoll(argv[3]);
	if (n != matrix->n) {
		fprintf(stderr, "Given vector (%zu) and matrix (%zu (%zu) x %zu) differ in size\n",
				n,
				matrix->m_total,
				matrix->m_local,
				matrix->n);
		ShrayFinalize(1);
	}

	double *vector = ShrayMalloc(n, n * sizeof(double));
	double *out = ShrayMalloc(matrix->m_total, matrix->m_total * sizeof(double));

	init(vector, n);
	ShraySync(vector);

	double duration;

	TIME(duration, steady_state(matrix, vector, out, n, iterations););

	if (ShrayOutput) {
	    printf("%lf\n", matrix->nnz * 2.0 / 1000000000 / duration);
	}

	ShrayFree(out);
	ShrayFree(vector);
	csr_free(matrix);

	ShrayFinalize(0);
	return EXIT_SUCCESS;
}
