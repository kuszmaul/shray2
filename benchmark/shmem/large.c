/* Benchmarks how long it takes to do a non-blocking get with synchronisation 
 * on a large message. Outputs the time in nanoseconds averaged per page,
 * using the GASNET_SEGMENT_EVERYTHING configuration. */ 

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
        printf("Usage: the number of megabytes you want to get\n");
        shmem_finalize();
    }

    size_t bufferSize = atol(argv[1]) * 1024 * 1024; 

    shmem_init();

    unsigned int rank = shmem_my_pe();
    unsigned int size = shmem_n_pes();

    double *buffer = shmem_malloc(bufferSize / sizeof(double));
    double *localBuffer = malloc(bufferSize);

    double duration;

    TIME(duration,
        shmem_getmem_nbi(localBuffer, buffer, bufferSize, (rank + 1) % size);
        shmem_quiet();
    );

    if (rank == 0) {
        printf("%d\n", (int)(duration / bufferSize * 4096 * 1000000000));
    }

    shmem_barrier_all();

    shmem_free(buffer);
    free(localBuffer);

    shmem_finalize();
}
