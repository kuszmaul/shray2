#include "../util/time.h"

#include <upc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char **argv)
{
	if (argc != 2) {
	    printf("Takes one command-line argument, n.\n");
	    exit(EXIT_FAILURE);
	}

	size_t n = atol(argv[1]);
    if (n % THREADS != 0) {
        printf("Please make sure the number of processors divides n. Suggestion: n = %zu.\n",
                n / THREADS * THREADS);
    }

	shared double *B = upc_all_alloc(THREADS, n / THREADS * sizeof(double));

    upc_barrier;

    if (MYTHREAD == 1) {
        B[0] = 1.0;
    }

    upc_barrier;

    printf("%lf\n", B[0]);

    if (MYTHREAD == 0) {
        upc_free(B);
    }

	exit(EXIT_SUCCESS);
}
