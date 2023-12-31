#ifndef CSR__GUARD
#define CSR__GUARD

#ifdef __cplusplus
extern "C" {
#endif

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

typedef struct {
	double *values;
	size_t *col_indices;
	size_t *row_indices;
	size_t nnz_local;
	size_t nnz_total;
	size_t n;
	size_t m_local; /* The number of local rows. */
	size_t m_total; /* The total number of rows. */
} csr_t;

/**
 * Free a matrix allocated by csr_parse.
 */
void csr_free(csr_t *matrix);

/**
 * Parse a sparse mm matrix for the local rank, assumes 1-based index
 *
 * \param file Name of the original non-separated mm matrix.
 * \param rank Rank number, 0-index.
 */
csr_t *csr_parse_local(const char *file, int rank);

/**
 * Print a CSR matrix.
 *
 * \param matrix Matrix to print.
 */
void csr_print(const csr_t *matrix);

/*
 * Creates a monopoly board of size n stochastic matrix. You throw two dices,
 * Calculates only the local portion for processor s out of p total.
 */
csr_t *monopoly(size_t n, unsigned int p, unsigned int s);

#ifdef __cplusplus
}
#endif

#endif
