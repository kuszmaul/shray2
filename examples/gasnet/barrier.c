#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>
#include <gasnet.h>
#include <gasnet_coll.h>
#include <math.h>
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

    if (argc != 3) {
        printf("Usage: number of iterations per timing, niter\n");
        gasnet_exit(1);
    }

    int p = gasnet_nodes();
    int iterations = atoi(argv[1]);
    int niter = atoi(argv[2]);

    double duration;

    /* Wellfords algorithm may be overkill, but numerical stability is
       nice. */
    int count = 0;
    double mean = 0;
    double m2 = 0;

    for (int t = 0; t < niter; t++) {
        TIME(duration,
                for (int i = 0; i < iterations; i++) {
                    gasnetBarrier();
                }
            )

        double avg_time = duration / (double) iterations * 1e6;

        count++;
        double delta_pre = avg_time - mean;
        mean += delta_pre / count;
        double delta_post = avg_time - mean;
        m2 += delta_pre * delta_post;
    }

    if (gasnet_mynode() == 0) {
        fprintf(stderr, "Average time for a gasnet barrier on %d processors (in microseconds):\n",
                p);
        printf("%d & %lf & %lf", p, mean, sqrt(m2 / count));
    }

    gasnetBarrier();

    gasnet_exit(0);
}
