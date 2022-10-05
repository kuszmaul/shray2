#include <shmem.h>
#include <stdio.h>
#include <stdlib.h>
#include <cblas.h>

static inline size_t roundUp(size_t a, size_t b)
{
    return (a + b - 1) / b;
}

static void printMatrix(double *array, size_t n)
{
	if (shmem_my_pe() != 0) {
		return;
	}

	printf("matrix: \n");
	for (size_t i = 0; i < n; ++i) {
		for (size_t j = 0; j < n; ++j) {
			printf("%f, ", array[i * n + j]);
		}
		printf("\n");
	}
	printf("\n");
}

static inline size_t shmem_index_start(int mype, int npes, size_t n)
{
	return mype * roundUp(n, npes);
}

static inline size_t shmem_index_end(int mype, int npes, size_t n)
{
	return (mype == npes - 1)
		? n
		: (mype + 1) * roundUp(n, npes);
}


static void sync(long *pSync, double *x, int n)
{
	int npes = shmem_n_pes();
	for (int i = 0; i < npes; ++i) {
		int pe = i;
		size_t start = shmem_index_start(pe, npes, n);
		size_t end = shmem_index_end(pe, npes, n);
		shmem_broadcast64(
				x + (n * start),
				x + (n * start),
				n * (end - start),
				pe,
				0, 0, npes, pSync);
	}
}


void init(double *matrix, size_t n, double value)
{
	int mype = shmem_my_pe();
	int npes = shmem_n_pes();

	size_t start = shmem_index_start(mype, npes, n);
	size_t end = shmem_index_end(mype, npes, n);

	for (size_t i = start; i < end; i++) {
		for (size_t j = 0; j < n; j++) {
			matrix[i * n + j] = value;
		}
	}
}

void matmul(double *A, double *B, double *C, size_t n)
{
	int mype = shmem_my_pe();
	int npes = shmem_n_pes();

	size_t start = shmem_index_start(mype, npes, n);
	size_t end = shmem_index_end(mype, npes, n);

	cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
	        end - start, n, n, 1.0, A + start * n,
	        n, B, n, 0.0, C + start * n, n);
}

///* If A, B are all one's, C should be n at every entry. */
int check(double *C, size_t n, double epsilon)
{
	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++) {
			if ((C[i * n + j] - n) * (C[i * n + j] - n) > epsilon) {
				return 0;
			}
		}
	}

	return 1;
}

int main(int argc, char **argv)
{
	if (argc != 2) {
	    printf("Takes one command-line argument, size of matrix.\n");
	    exit(EXIT_FAILURE);
	}

	static long pSync[SHMEM_BCAST_SYNC_SIZE];
	for (int i = 0; i < SHMEM_BCAST_SYNC_SIZE; i++)
		pSync[i] = SHMEM_SYNC_VALUE;

	shmem_init();

	size_t n = atol(argv[1]);

	double *A = shmem_malloc(n * n * sizeof(double));
	double *B = shmem_malloc(n * n * sizeof(double));
	double *C = shmem_malloc(n * n * sizeof(double));

	init(A, n, 1);
	init(B, n, 1);
	sync(pSync, A, n);
	sync(pSync, B, n);

	matmul(A, B, C, n);
	sync(pSync, C, n);

	if (shmem_my_pe() == 0) {
		if (check(C, n, 0.01)) {
		    printf("Success!\n");
		} else {
		    printf("Failure!\n");
		}
	}

	shmem_free(A);
	shmem_free(B);
	shmem_free(C);

	shmem_finalize();

	exit(EXIT_SUCCESS);
}
