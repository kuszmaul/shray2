#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <shmem.h>

void init(size_t n, size_t *input)
{
    int my_pe = shmem_my_pe();
    size_t elems = n / shmem_n_pes();

    for (size_t i = 0; i < elems; i++) {
        input[i] = i + my_pe * elems;
    }
}

void random_fill(size_t n, size_t local_prob, size_t *input, size_t *output)
{
    /* Fill our portion of the output array */
    int my_pe = shmem_my_pe();
    size_t elems = n / shmem_n_pes();

    for (size_t i = 0; i < elems; i++) {
        // TODO: Get the randomness based on a distribution by `local_prob`.
        size_t index = rand() % n;

        int target_pe = index / elems;
        int target_offset = index % elems;
        shmem_getmem(output + i, input + target_offset, sizeof(size_t), target_pe);
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

    size_t *input = shmem_malloc(n / p * sizeof(size_t));
    size_t *output = shmem_malloc(n / p * sizeof(size_t));

    init(n, input);

    random_fill(n, local_prob, input, output);

    shmem_free(input);
    shmem_free(output);

    shmem_finalize();

    return EXIT_SUCCESS;
}
