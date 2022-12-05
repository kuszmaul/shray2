#ifndef CSR__GUARD
#define CSR__GUARD

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

typedef struct {
	double *values;
	size_t *col_indices;
	size_t *row_indices;
	size_t nnz;
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

#endif
