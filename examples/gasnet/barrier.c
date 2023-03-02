#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>
#include <gasnet.h>
#include <gasnet_coll.h>
#include <shray2/shray.h>
#include "../util/time.h"

#define GASNET_SAFE(fncall)                                                              \
    {                                                                                    \
        int retval;                                                                      \
        if ((retval = fncall) != GASNET_OK) {                                            \
            printf("Error during GASNet call\n");                                        \
            gasnet_exit(1);                                                              \
        }                                                                                \
    }

void gasnetBarrier()
{
    gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0, GASNET_BARRIERFLAG_ANONYMOUS);
}

int main(int argc, char **argv)
{
    GASNET_SAFE(gasnet_init(&argc, &argv));

    GASNET_SAFE(gasnet_attach(NULL, 0, 4096, 0));

    if (argc != 2) {
        printf("Usage: number of iterations\n");
        gasnet_exit(1);
    }

    int p = gasnet_nodes();
    int iterations = atoi(argv[1]);

    double duration;
    TIME(duration,
            for (int i = 0; i < iterations; i++) {
                gasnetBarrier();
            }
        )

    if (gasnet_mynode() == 0) {
        fprintf(stderr, "Average time for a gasnet barrier on %d processors (in microseconds):\n",
                p);
        printf("%lf", duration / (double) iterations * 1000000.0);
    }

    gasnetBarrier();

    gasnet_exit(0);
}
