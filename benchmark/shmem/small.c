/* Benchmarks how long it takes to do a blocking get on a page (PAGESIZE bytes) 
 * in nanoseconds. */ 

#define TIME(duration, fncalls)                                        \
    {                                                                  \
        struct timeval tv1, tv2;                                       \
        gettimeofday(&tv1, NULL);                                      \
        fncalls                                                        \
        gettimeofday(&tv2, NULL);                                      \
        duration = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +    \
         (double) (tv2.tv_sec - tv1.tv_sec);                           \
    }

#include <shmem.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/time.h>

#define PAGESIZE 4096

void main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: number of iterations you want to average over\n");
        exit(EXIT_FAILURE);
    }

	shmem_init();

    size_t niters = atol(argv[1]);

    unsigned int rank = shmem_my_pe();
    unsigned int size = shmem_n_pes();

    double *buffer = shmem_malloc(PAGESIZE / sizeof(double));

    /* duration of our node */
    double duration; 

    TIME(duration,
    for (size_t i = 0; i < niters; i++) {
        shmem_getmem(buffer, buffer, PAGESIZE, (rank + 1) % size);
    });

    if (rank == 0) {
        printf("%d\n", (int)(duration / niters * 1000000000));
    }

    shmem_barrier_all();

    shmem_free(buffer);

    shmem_finalize();
}
