#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <shmem.h>
#include <sys/param.h>


#define TIME(duration, fncalls)                                        \
    {                                                                  \
        struct timeval tv1, tv2;                                       \
        gettimeofday(&tv1, NULL);                                      \
        fncalls                                                        \
        gettimeofday(&tv2, NULL);                                      \
        duration = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +    \
         (double) (tv2.tv_sec - tv1.tv_sec);                           \
    }

void init(size_t n, double *input)
{
    size_t elems = n / shmem_n_pes();

    for (size_t i = 0; i < elems; i++) {
        input[i] = 1.0;
    }
}

/* Returns an integer in [0, n] that is in [local_start, local_end[ with probability
 * localProp %. */
int rand_n_prob(size_t local_start, size_t local_end, int localProp, size_t size)
{
    int n_pes = shmem_n_pes();

    bool local = (rand() % 100 < localProp);

    size_t result;

    if (local || n_pes == 1) {
        result = (size_t)(rand() % (size / n_pes) + local_start);
    } else {
        size_t randomInt = (size_t)(rand() % (size - (size / n_pes)));
        result = (randomInt < local_start) ? randomInt : randomInt + local_end - local_start;
    }

    return result;
}

double random_reduce(size_t n, int local_prob, double *input, size_t size)
{
    double result = 0.0;
    int my_pe = shmem_my_pe();
    int n_pes = shmem_n_pes();
    int elems = size / n_pes;

    for (size_t i = 0; i < n; i++) {
        /* Generate a random index with a certain probability. */
        int r = rand_n_prob(my_pe * elems,  (my_pe + 1) * elems, local_prob, size);

        /* Determine where it is stored. */
        int target_pe = r / elems;
        int target_offset = r % elems;
        double randomDouble;
        shmem_getmem(&randomDouble, input + target_offset, sizeof(double), target_pe);

        result += randomDouble;
    }

    return result;
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "Usage: array size, number of accesses, probability local access.\n");
        exit(EXIT_FAILURE);
    }

    size_t size = atoll(argv[1]);
    size_t n = atoll(argv[2]);
    int local_prob = atoi(argv[3]);

    if (local_prob > 100) {
        fprintf(stderr, "Probability can not be greater than 100\n");
        exit(EXIT_FAILURE);
    }

    shmem_init();

    srand(time(NULL));

    double *input = shmem_malloc(size * sizeof(double));

    init(size, input);

    shmem_barrier_all();

    double duration;
    double result;

    TIME(duration,
    result = random_reduce(n, local_prob, input, size);
    shmem_barrier_all();)

    if (shmem_my_pe() == 0) printf("%lf\n", duration);

    if (result == (double)n) {
        fprintf(stderr, "Success!\n");
    } else {
        fprintf(stderr, "Failure! Result = %lf\n", result);
    }

    shmem_free(input);

    shmem_finalize();

    return EXIT_SUCCESS;
}
