#include <shray2/shray.h>
#include "../util/csr.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

typedef struct {
    csr_t *matrix;
    double *vector;
    double *out;
} matrix_t;

void init(double *a, size_t n)
{
	for (size_t i = ShrayStart(n); i < ShrayEnd(n); ++i) {
		a[i] = i;
	}
}

void spmv_mt(worker_info_t *info)
{
    matrix_t *arguments = (matrix_t *)info->args;
    csr_t *matrix = arguments->matrix;
    double *vector = arguments->vector;
    double *out = arguments->out;

	for (size_t i = info->start, k = 0; i < info->end; ++i, ++k) {
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

void steady_state(csr_t *matrix, double *vector, double *out, size_t iterations)
{
    matrix_t matrixInfo;
    matrixInfo.matrix = matrix;
    matrixInfo.vector = vector;
    matrixInfo.out = out;

	for (size_t i = 0; i < iterations; ++i) {
		ShrayRunWorker(spmv_mt, matrix->n, &matrixInfo);
		ShraySync(matrixInfo.out);
		double *tmp = matrixInfo.vector;
		matrixInfo.vector = matrixInfo.out;
		matrixInfo.out = tmp;
	}
}

int main(int argc, char **argv)
{
	ShrayInit(&argc, &argv);

	if (argc != 3) {
		fprintf(stderr, "Usage: FILE iterations\n");
		ShrayFinalize(1);
	}

	csr_t *matrix = csr_parse_local(argv[1], ShrayRank());
	if (!matrix) {
		fprintf(stderr, "Could not parse matrix for %s\n", argv[1]);
		ShrayFinalize(1);
	}

	size_t iterations = atoll(argv[2]);

	double *vector = ShrayMalloc(matrix->n, matrix->n * sizeof(double));
	double *out = ShrayMalloc(matrix->m_total, matrix->m_total * sizeof(double));

	init(vector, matrix->n);
	ShraySync(vector);

	double duration;

	TIME(duration, steady_state(matrix, vector, out, iterations););

	if (ShrayOutput) {
	    printf("%lf\n", matrix->nnz_total * 2.0 / 1000000000 / duration);
	}

	ShrayFree(out);
	ShrayFree(vector);
	csr_free(matrix);

	ShrayFinalize(0);
	return EXIT_SUCCESS;
}
