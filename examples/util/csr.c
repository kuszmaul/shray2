#include "csr.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

void csr_free(csr_t *matrix)
{
	if (matrix) {
		if (matrix->values) {
			free(matrix->values);
		}
		if (matrix->col_indices) {
			free(matrix->col_indices);
		}
		if (matrix->row_indices) {
			free(matrix->row_indices);
		}

		free(matrix);
	}
}

static csr_t *alloc_matrix(size_t m_local, size_t m_total, size_t n, size_t nnz_local, size_t nnz_total)
{
	csr_t *matrix = malloc(sizeof(csr_t));
	if (!matrix) {
		fprintf(stderr, "Could allocate spare matrix\n");
		return NULL;
	}

	matrix->nnz_local = nnz_local;
	matrix->nnz_total = nnz_total;
	matrix->m_local = m_local;
	matrix->m_total = m_total;
	matrix->n = n;
	matrix->values = malloc(sizeof(double) * matrix->nnz_local);
	matrix->col_indices = malloc(sizeof(size_t) * matrix->nnz_local);
	matrix->row_indices = malloc(sizeof(size_t) * (matrix->m_local + 1));

	if (!matrix->values || !matrix->col_indices || !matrix->row_indices) {
		fprintf(stderr, "Could allocate spare matrix\n");
		csr_free(matrix);
		return NULL;
	}

	return matrix;
}

static char *strcatalloc(const char *s1, const char *s2, int rank)
{
	char rank_str[100];
	snprintf(rank_str, 100, "%d", rank);

	size_t s1len = strlen(s1);
	size_t s2len = strlen(s2);
	size_t ranklen = strlen(rank_str);
	char *res = malloc(s1len + s2len + ranklen + 1);
	if (!res) {
		return NULL;
	}

	strcpy(res, s1);
	strcat(res, s2);
	strcat(res, rank_str);
	res[s1len + s2len + ranklen] = '\0';

	return res;
}

csr_t *csr_parse_local(const char *file, int rank)
{
	int mmrank = rank + 1;
	csr_t *result = NULL;
	char *info_name = NULL;
	char *col_name  = NULL;
	char *row_name  = NULL;
	char *val_name  = NULL;
	FILE *org_file = NULL;
	FILE *info_file = NULL;
	FILE *col_file  = NULL;
	FILE *row_file  = NULL;
	FILE *val_file  = NULL;

	info_name = strcatalloc(file, "_info", mmrank);
	col_name = strcatalloc(file, "_column", mmrank);
	row_name = strcatalloc(file, "_row", mmrank);
	val_name = strcatalloc(file, "_values", mmrank);
	if (!info_name || !col_name || !row_name || !val_name) {
		goto cleanup;
	}

	org_file = fopen(file, "r");
	info_file = fopen(info_name, "r");
	col_file = fopen(col_name, "r");
	row_file = fopen(row_name, "r");
	val_file = fopen(val_name, "r");
	if (!org_file || !info_file || !col_file || !row_file || !val_file) {
		perror("Could not open one of the matrix files");
		goto cleanup;
	}

	/*
	 * From here on just assume the files are correct since they are
	 * generated
	 */
	size_t m_local = 0;
	size_t m_total = 0;
	size_t n_local = 0;
	size_t n_total = 0;
	size_t nnz_local = 0;
	size_t nnz_total = 0;
	if (fscanf(info_file, "%zu %zu %zu", &m_local, &n_local, &nnz_local) != 3) exit(EXIT_FAILURE);
	if (fscanf(org_file, "%zu %zu %zu", &m_total, &n_total, &nnz_total) != 3) exit(EXIT_FAILURE);

	result = alloc_matrix(m_local, m_total, n_total, nnz_local, nnz_total);
	if (!result) {
		goto cleanup;
	}

	int i = 0;
	size_t tmp = 0;
	/* Adjust the 1-based index. */
	while (fscanf(col_file, "%zu", &tmp) != -1) {
		result->col_indices[i++] = tmp - 1;
	}
	i = 0;
	while (fscanf(row_file, "%zu", &tmp) != -1) {
		result->row_indices[i++] = tmp - 1;
	}
	result->row_indices[result->m_local] = nnz_local;
	i = 0;
	while (fscanf(val_file, "%lf", &result->values[i++]) != -1);

cleanup:
	if (info_name) {
		free(info_name);
	}
	if (col_name) {
		free(col_name);
	}
	if (row_name) {
		free(row_name);
	}
	if (val_name) {
		free(val_name);
	}
	if (org_file) {
		fclose(org_file);
	}
	if (info_file) {
		fclose(info_file);
	}
	if (col_file) {
		fclose(col_file);
	}
	if (row_file) {
		fclose(row_file);
	}
	if (val_file) {
		fclose(val_file);
	}

	return result;
}

void csr_print(const csr_t *matrix)
{
	printf("%zu (%zu) %zu (%zu)\n", matrix->nnz_total, matrix->nnz_local, matrix->m_total, matrix->m_local);
	for (size_t i = 0; i < matrix->nnz_local; ++i) {
		printf("%lf ", matrix->values[i]);
	}
	printf("\n");
	for (size_t i = 0; i < matrix->nnz_local; ++i) {
		printf("%zu ", matrix->col_indices[i]);
	}
	printf("\n");
	for (size_t i = 0; i <= matrix->m_local; ++i) {
		printf("%zu ", matrix->row_indices[i]);
	}
	printf("\n");
}

/* probabilities[eyes] is the probability to throw eyes with two dice. */
static double probabilities[13] = {0, 0, 1.0 / 36, 2.0 / 36, 3.0 / 36, 4.0 / 36, 5.0 / 36, 6.0 / 36,
                                   5.0 / 36, 4.0 / 36, 3.0 / 36, 2.0 / 36, 1.0 / 36};

csr_t *monopoly(size_t n, unsigned int p, unsigned int s)
{
    size_t blockSize = (n + p - 1) / p;
    size_t m_local = (s == p - 1) ? n - (p - 1) * blockSize : blockSize;
    csr_t *board = alloc_matrix(m_local, n, n, 11 * m_local, 11 * n);

    size_t i = 0;
    for (size_t row = 0; row < m_local; row++) {
        size_t global_row = s * blockSize + row;
        for (int eyes = 2; eyes <= 12; eyes++) {
            board->values[i] = probabilities[eyes];
            board->col_indices[i] = (global_row + eyes) % n;
            i++;
        }
        board->row_indices[row] = 11 * row;
    }

    board->row_indices[m_local] = 11 * m_local;

    return board;
}
