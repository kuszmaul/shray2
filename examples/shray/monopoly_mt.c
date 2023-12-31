#include <shray2/shray.h>
#include "../util/csr.h"
#include "../util/time.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <omp.h>

//#define CHECK

void init(double *a, size_t n)
{
    /* sum_i a[i] = 1. */
	for (size_t i = ShrayStart(a); i < ShrayEnd(a); ++i) {
		a[i] = (i % 2 == 0) ? 2.0 / n : 0.0;
	}
}

void spmv(csr_t *matrix, double *vector, double *out)
{
    size_t start = ShrayStart(out);
    size_t end = ShrayEnd(out);
    size_t k = 0;
    #pragma omp parallel for
	for (size_t i = start; i < end; i++) {
		double outval = 0;

		size_t row_start = matrix->row_indices[k];
		size_t row_end = matrix->row_indices[k + 1];
		for (size_t j = row_start; j < row_end; ++j) {
			size_t col = matrix->col_indices[j];
			double v = matrix->values[j];

			outval += v * vector[col];
		}

		out[i] = outval;
        k++;
	}
}

void steady_state(csr_t *matrix, double *vector, double *out, size_t iterations)
{
	for (size_t i = 0; i < iterations; ++i) {
		spmv(matrix, vector, out);
		ShraySync(out);
		double *tmp = vector;
		vector = out;
		out = tmp;
	}
}

int main(int argc, char **argv)
{
	ShrayInit(&argc, &argv);

	if (argc != 3) {
		fprintf(stderr, "Usage: n iterations\n");
		ShrayFinalize(1);
	}

	size_t n = atoll(argv[1]);
	size_t iterations = atoll(argv[2]);

    csr_t *matrix = monopoly(n, ShraySize(), ShrayRank());
    if (!matrix) {
        fprintf(stderr, "Something went wrong when allocating the sparse matrix\n");
        ShrayFinalize(1);
    }

	double *vector = (double *)ShrayMalloc(matrix->n, matrix->n * sizeof(double));
	double *out = (double *)ShrayMalloc(matrix->n, matrix->n * sizeof(double));

	init(vector, n);
	ShraySync(vector);

	double duration;

	TIME(duration, steady_state(matrix, vector, out, iterations););

#ifdef CHECK
    /* Steady state is (1/n, ..., 1/n). This may take a while to converge. */
    double error = 0.0; // L2 norm of difference
    for (size_t i = 0; i < n; i++) {
        error += (vector[i] - 1.0 / n) * (vector[i] - 1.0 / n);
    }
    error = sqrt(error);
    double relative_error = error / sqrt(1.0 / n);
    fprintf(stderr, "The state vector should converge to all %.10e's.\n"
                    "The relative error after %ld iterations is %.10e\n",
                    1.0 / n, iterations, relative_error);
#endif

	if (ShrayOutput) {
	    printf("%lf\n", matrix->nnz_total * iterations * 2.0 / 1000000000 / duration);
	}

	ShrayFree(out);
	ShrayFree(vector);
	csr_free(matrix);

	ShrayFinalize(0);
	return EXIT_SUCCESS;
}
