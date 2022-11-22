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
    int my_pe = shmem_my_pe();
    size_t elems = n / shmem_n_pes();

    for (size_t i = 0; i < elems; i++) {
        input[i] = i + my_pe * elems;
    }
}

/* Returns an integer in [0, n] that is in [local_start, local_end[ with probability
 * localProp %. */
int rand_n_prob(size_t n, size_t local_start, size_t local_end, int localProp)
{
    int n_pes = shmem_n_pes();

    bool local = (rand() % 100 < localProp);

    size_t result;

    if (local) {
        result = (size_t)(rand() % (n / n_pes) + local_start);
    } else {
        size_t randomInt = (size_t)(rand() % (n - (n / n_pes)));
        result = (randomInt >= local_start) ? randomInt + local_end - local_start : randomInt;
    }

    return result;
}

void random_fill(size_t n, int local_prob, double *input, double *output)
{
    int my_pe = shmem_my_pe();
    int n_pes = shmem_n_pes();
    int elems = n / n_pes;

    for (int i = 0; i < elems; i++) {
        /* Generate a random index with a certain probability. */
        int r = rand_n_prob(n, my_pe * elems,  (my_pe + 1) * elems, local_prob);

        /* Determine where it is stored. */
        int target_pe = r / elems;
        int target_offset = r % elems;
        shmem_getmem(output + i, input + target_offset, sizeof(double), target_pe);
    }
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: n, probability local access\n");
        return EXIT_FAILURE;
    }

    size_t n = atoll(argv[1]);
    size_t local_prob = atoll(argv[2]);
    if (local_prob > 100) {
        fprintf(stderr, "Probability can not be greater than 100\n");
        return EXIT_FAILURE;
    }

    shmem_init();

    int p = shmem_n_pes();
    srand(time(NULL));

    double *input = shmem_malloc(n / p * sizeof(double));
    double *output = shmem_malloc(n / p * sizeof(double));

    init(n, input);

    shmem_barrier_all();

    double duration;

    TIME(duration,
    random_fill(n, local_prob, input, output);
    shmem_barrier_all();)

    printf("%lf\n", duration);

    shmem_free(input);
    shmem_free(output);

    shmem_finalize();

    return EXIT_SUCCESS;
}
