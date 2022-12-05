#include <shray2/shray.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

typedef struct {
	double *data;
	size_t *col_indices;
	size_t *row_indices;
	size_t nnz;
	size_t rows;
	size_t cols;
} csr_t;

/*
 * Expected format is
 *
 * % indicates a comment
 * m n nnz
 * col row value    : nnz times
 */
csr_t *parse_sparse(const char *file)
{
	FILE *handle = fopen(file, "r");
	if (!handle) {
		perror("Could not open file");
		ShrayFinalize(1);
	}

	size_t m = 0;
	size_t n = 0;
	size_t nnz = 0;

	char *line = NULL;
	size_t length = 0;
	ssize_t read = 0;
	// Read until we've reached the header
	while ((read = getline(&line, &length, handle)) != -1) {
		if (line[0] == '%') {
			continue;
		}
		if (read == 0) {
			fprintf(stderr, "Unexpected header format in input\n");
			ShrayFinalize(1);
		}

		int res = sscanf(line, "%zu %zu %zu", &m, &n, &nnz);
		if (res == 3) {
			break;
		} else if (errno != 0) {
			perror("Error reading input header");
			ShrayFinalize(1);
		} else {
			fprintf(stderr, "Unexpected header format in input\n");
			ShrayFinalize(1);
		}
	}

	if (n == 0 || m == 0 || nnz == 0) {
		fprintf(stderr, "Sparse matrix must have at least 1 element\n");
		ShrayFinalize(1);
	}

	csr_t *matrix = malloc(sizeof(csr_t));
	if (!matrix) {
		fprintf(stderr, "Could allocate spare matrix\n");
		ShrayFinalize(1);
	}

	matrix->nnz = nnz;
	matrix->rows = m + 1;
	matrix->cols = n;
	matrix->data = malloc(sizeof(double) * matrix->nnz);
	matrix->col_indices = malloc(sizeof(size_t) * matrix->nnz);
	matrix->row_indices = malloc(sizeof(size_t) * matrix->rows);

	if (!matrix->data || !matrix->col_indices || !matrix->row_indices) {
		fprintf(stderr, "Could allocate spare matrix\n");
		ShrayFinalize(1);
	}

	// Read the matrix data
	size_t col = 0;
	size_t row = 0;
	size_t last_row = 0;
	size_t i = 0;
	double value = 0;
	while ((read = getline(&line, &length, handle)) != -1) {
		if (read == 0) {
			continue;
		}
		if (i == nnz) {
			fprintf(stderr, "File contains more lines then the header claims\n");
			ShrayFinalize(1);
		}

		int res = sscanf(line, "%zu %zu %lf", &col, &row, &value);
		if (res != 3) {
			if (errno != 0) {
				perror("Error reading row");
				ShrayFinalize(1);
			} else {
				fprintf(stderr, "Unexpected row format in input\n");
				ShrayFinalize(1);
			}
		}

		matrix->data[i] = value;
		matrix->col_indices[i] = col - 1;
		if (row != last_row) {
			matrix->row_indices[last_row] = i;
			last_row = row;
		}

		++i;
	}

	if (i != nnz) {
		fprintf(stderr, "File contains less lines then the header claims\n");
		ShrayFinalize(1);
	}
	matrix->row_indices[matrix->rows - 1] = matrix->nnz;

	if (line) {
		free(line);
	}

	fclose(handle);
	return matrix;
}

void init(double *a, size_t n)
{
	for (size_t i = ShrayStart(n); i < ShrayEnd(n); ++i) {
		a[i] = i;
	}
}

void spmv(csr_t *matrix, double *vector, double *out, size_t n)
{
	for (size_t i = ShrayStart(n); i < ShrayEnd(n); ++i) {
		double outval = 0;
		size_t row_start = matrix->row_indices[i];
		size_t row_end = matrix->row_indices[i + 1];
		for (size_t j = row_start; j < row_end; ++j) {
			size_t col = matrix->col_indices[j];
			double v = matrix->data[j];

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

static void print_crm(const csr_t *matrix)
{
	printf("%zu %zu\n", matrix->nnz, matrix->rows);
	for (size_t i = 0; i < matrix->nnz; ++i) {
		printf("%lf ", matrix->data[i]);
	}
	printf("\n");
	for (size_t i = 0; i < matrix->nnz; ++i) {
		printf("%zu ", matrix->col_indices[i]);
	}
	printf("\n");
	for (size_t i = 0; i < matrix->rows; ++i) {
		printf("%zu ", matrix->row_indices[i]);
	}
	printf("\n");
}

int main(int argc, char **argv)
{
	ShrayInit(&argc, &argv);

	if (argc != 4) {
		fprintf(stderr, "Usage: FILE n iterations\n");
		ShrayFinalize(1);
	}

	// parse arguments
	csr_t *matrix = parse_sparse(argv[1]);
	size_t n = atoll(argv[2]);
	size_t iterations = atoll(argv[3]);
	if (n != matrix->cols) {
		fprintf(stderr, "Given vector (%zu) and matrix (%zu x %zu) differ in size\n",
				n,
				matrix->rows - 1,
				matrix->cols);
		ShrayFinalize(1);
	}

	double *vector = ShrayMalloc(n, n * sizeof(double));
	double *out = ShrayMalloc(n, n * sizeof(double));

	init(vector, n);
	ShraySync(vector);

	double duration;

	TIME(duration, steady_state(matrix, vector, out , n, iterations););

	if (ShrayOutput) {
	    printf("%lf\n", matrix->nnz * 2.0 / 1000000000 / duration);
	}

	free(matrix);
	ShrayFree(out);
	ShrayFree(vector);

	ShrayFinalize(0);
	return EXIT_SUCCESS;
}
