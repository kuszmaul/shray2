#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <shmem.h>
#include <sys/param.h>

void init(size_t n, double *input)
{
    int my_pe = shmem_my_pe();
    size_t elems = n / shmem_n_pes();

    for (size_t i = 0; i < elems; i++) {
        input[i] = i + my_pe * elems;
    }
}

int find_ceil(int *arr, int r, int l, int h)
{
    int mid;
    while (l < h)
    {
         mid = l + ((h - l) >> 1);
        (r > arr[mid]) ? (l = mid + 1) : (h = mid);
    }
    return (arr[l] >= r) ? l : -1;
}

/*
 * Return an index in the range [0,n) according to a given frequency. Modified
 * from https://www.geeksforgeeks.org/random-number-generator-in-arbitrary-probability-distribution-fashion.
 */
int rand_freq(int *freq, int *prefix, int n)
{
    prefix[0] = freq[0];
    for (int i = 1; i < n; ++i)
        prefix[i] = prefix[i - 1] + freq[i];

    int r = (rand() % prefix[n - 1]) + 1;
    return find_ceil(prefix, r, 0, n - 1);
}


void random_fill(size_t n, int local_prob, double *input, double *output)
{
    /* Fill our portion of the output array */
    int my_pe = shmem_my_pe();
    int n_pes = shmem_n_pes();
    int *freq = malloc(n_pes * sizeof(int));
    int *prefix = malloc(n_pes * sizeof(int));
    if (!freq || !prefix) {
        fprintf(stderr, "allocation for frequency data failed\n");
        shmem_finalize();
        exit(1);
    }

    int other_rank_prob = MAX((100 - local_prob) / MAX(n_pes - 1, 1), 1);
    for (int i = 0; i < n_pes; ++i) {
        freq[i] = i == my_pe ? local_prob : other_rank_prob;
    }

    int elems = n / n_pes;
    for (int i = 0; i < elems; i++) {
        /* Determine a rank depending on the probability */
        int target_pe = rand_freq(freq, prefix, n_pes);

        int target_offset = rand() % elems;
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

    double *input = shmem_malloc(n / p * sizeof(double));
    double *output = shmem_malloc(n / p * sizeof(double));

    init(n, input);

    random_fill(n, local_prob, input, output);

    shmem_free(input);
    shmem_free(output);

    shmem_finalize();

    return EXIT_SUCCESS;
}
