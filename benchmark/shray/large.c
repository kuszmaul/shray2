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

#include <shray.h>
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
        printf("Usage: the number of megabytes you want to get\n");
        gasnet_exit(1);
    }

    size_t bufferSize = atol(argv[1]) * 1024 * 1024;

    unsigned int rank = gasnet_mynode();
    unsigned int size = gasnet_nodes();

    void *buffer;

    if (rank == 0) {
        MMAP_SAFE(buffer, mmap(NULL, bufferSize, PROT_READ | PROT_WRITE, 
            MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));
    }

    /* Broadcast location to the other nodes. */
    gasnet_coll_broadcast(gasnete_coll_team_all, &buffer, 0, &buffer, 
            sizeof(void *), GASNET_COLL_DST_IN_SEGMENT);

    if (rank != 0) {
        MMAP_SAFE(buffer, mmap(buffer, bufferSize, PROT_READ | PROT_WRITE, 
            MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0));
    }

    /* duration of our node */
    double duration; 
    /* duration of node 0. Only node 0 outputs the result, but we check it does not 
     * differ too much.*/
    double duration0; 

    TIME(duration,
        gasnet_get_nbi(buffer, (rank + 1) % size, buffer, bufferSize);
        gasnet_wait_syncnbi_gets();
    );

    gasnet_coll_broadcast(gasnete_coll_team_all, &duration0, 0, &duration,
        sizeof(double), GASNET_COLL_DST_IN_SEGMENT);

    if (duration0 / duration > 1.05 || duration / duration0 > 1.05) {
        fprintf(stderr, "Warning: on node %d the duration differs more than 5%% "
                "from the duration on node 1: %d vs %d ns.\n", 
                rank, (int)(duration / bufferSize * PAGESIZE * 1000000000), 
                (int)(duration0 / bufferSize * PAGESIZE * 1000000000));
    }

    if (rank == 0) {
        printf("%d\n", (int)(duration / bufferSize * 4096 * 1000000000));
    }

    gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0, GASNET_BARRIERFLAG_ANONYMOUS);

    /* Freeing memory is for weenies ;) */
    gasnet_exit(0);
}
