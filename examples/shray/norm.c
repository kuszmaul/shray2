#include <stdio.h>
#include <stdlib.h>
#include <gasnet.h>
#include <gasnet_coll.h>
#include <shray2/shray.h>
#include "../util/time.h"

void gasnetBarrier(void)
{
    gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0, GASNET_BARRIERFLAG_ANONYMOUS);
}

/* Collective operation, replaces number by its sum over
 * all nodes. */
void gasnet_sum(long *number)
{
    long *numbers = malloc(ShraySize() * sizeof(long));
    gasnet_coll_gather_all(gasnete_coll_team_all, numbers, number,
            sizeof(long), GASNET_COLL_DST_IN_SEGMENT);
    for (unsigned long i = 0; i < ShraySize(); i++) {
        if (i == ShrayRank()) continue;
        *number += numbers[i];
    }
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s n\n", argv[0]);
        ShrayFinalize(EXIT_FAILURE);
    }

    long n = atol(argv[1]);

    long *arr = ShrayMalloc(n, n * sizeof(long));

    for (unsigned int i = ShrayStart(arr); i < ShrayEnd(arr); i++) {
        arr[i] = i + 1;
    }
    ShraySync(arr);

    double duration;
    long sum = 0;

    TIME(duration,
        for (unsigned int i = ShrayStart(arr); i < ShrayEnd(arr); i++) {
            sum += arr[i];
        }

        gasnetBarrier();

        gasnet_sum(&sum);
        );

    if (sum != n * (n + 1) / 2) {
        fprintf(stderr, "Error at rank %d, %ld != %ld\n",
                    ShrayRank(), sum, n * (n + 1) / 2);
    } else {
        fprintf(stderr, "Success at rank %d\n", ShrayRank());
    }

    printf("Achieved bandwidth: %lf MB/s\n", n * sizeof(long) / duration / 1e6);
    ShrayFinalize(EXIT_SUCCESS);
}
