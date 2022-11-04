/* Benchmarks how long it takes to do a blocking get on a page (PAGESIZE bytes) 
 * in nanoseconds, using the GASNET_SEGMENT_EVERYTHING configuration. */ 

#define MMAP_SAFE(variable, fncall)                                                     \
    {                                                                                   \
        variable = fncall;                                                              \
        if (variable == MAP_FAILED) {                                                   \
            fprintf(stderr, "Line %d, node [%d]: ", __LINE__, gasnet_mynode());         \
            perror("mmap failed");                                                      \
            gasnet_exit(1);                                                             \
        }                                                                               \
    }

#define GASNET_SAFE(fncall)                                                             \
    {                                                                                   \
        int retval;                                                                     \
        if ((retval = fncall) != GASNET_OK) {                                           \
            printf("Error during GASNet call\n");                                       \
            gasnet_exit(1);                                                             \
        }                                                                               \
    }

#define TIME(duration, fncalls)                                        \
    {                                                                  \
        struct timeval tv1, tv2;                                       \
        gettimeofday(&tv1, NULL);                                      \
        fncalls                                                        \
        gettimeofday(&tv2, NULL);                                      \
        duration = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +    \
         (double) (tv2.tv_sec - tv1.tv_sec);                           \
    }

#include <gasnet.h>
#include <gasnet_coll.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/time.h>

#define PAGESIZE 4096

void main(int argc, char **argv)
{
    GASNET_SAFE(gasnet_init(&argc, &argv));

    /* Must be built with GASNET_SEGMENT_EVERYTHING, so these arguments are ignored. */
    GASNET_SAFE(gasnet_attach(NULL, 0, PAGESIZE, 0));

    if (argc != 2) {
        printf("Usage: number of iterations you want to average over\n");
        gasnet_exit(1);
    }

    size_t niters = atol(argv[1]);

    unsigned int rank = gasnet_mynode();
    unsigned int size = gasnet_nodes();

    void *buffer;

    if (rank == 0) {
        MMAP_SAFE(buffer, mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, 
            MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));
    }

    /* Broadcast location to the other nodes. */
    gasnet_coll_broadcast(gasnete_coll_team_all, &buffer, 0, &buffer, 
            sizeof(void *), GASNET_COLL_DST_IN_SEGMENT);

    if (rank != 0) {
        MMAP_SAFE(buffer, mmap(buffer, PAGESIZE, PROT_READ | PROT_WRITE, 
            MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0));
    }

    /* duration of our node */
    double duration; 
    /* duration of node 0. Only node 0 outputs the result, but we check it does not 
     * differ too much.*/
    double duration0; 

    TIME(duration,
    for (size_t i = 0; i < niters; i++) {
        gasnet_get(buffer, (rank + 1) % size, buffer, PAGESIZE);
    });

    gasnet_coll_broadcast(gasnete_coll_team_all, &duration0, 0, &duration,
        sizeof(double), GASNET_COLL_DST_IN_SEGMENT);

    if (duration0 / duration > 1.005 || duration / duration0 > 1.005) {
        fprintf(stderr, "Warning: on node %d the duration differs more than 0.5%% "
                "from the duration on node 1: %d vs %d ns.\n", 
                rank, (int)(duration / niters * 1000000000), 
                (int)(duration0 / niters * 1000000000));
    }

    if (rank == 0) {
        printf("%d\n", (int)(duration / niters * 1000000000));
    }

    gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0, GASNET_BARRIERFLAG_ANONYMOUS);

    /* Freeing memory is for weenies ;) */
    gasnet_exit(0);
}
