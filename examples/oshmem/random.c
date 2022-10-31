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

int rand_n_prob(size_t n, size_t local_start, size_t local_end, int *freq, int *prefix)
{
    int r = rand();
    int pi = rand_freq(freq, prefix, 3);

    if (pi == 0) {
        return local_start == 0
            ? r % local_end /* there is no previous range */
            : r % MIN(local_start, local_start - 1);
    } else if (pi == 1) {
        return local_start + (r % (local_end - local_start));
    } else {
        return local_end == n
            ? local_start + (r % (local_end - local_start)) /* there is no range after */
            : local_end + (r % (n - local_end));
    }
}

void random_fill(size_t n, int local_prob, double *input, double *output)
{
    int my_pe = shmem_my_pe();
    int n_pes = shmem_n_pes();
    int elems = n / n_pes;

    int other_rank_prob = MAX((100 - local_prob) / 2, 1);
    int freq[3] = { other_rank_prob, local_prob, other_rank_prob };
    int prefix[3] = { 0, 0, 0 };

    for (int i = 0; i < elems; i++) {
        /* Generate a random index with a certain probability. */
        int r = rand_n_prob(n, my_pe * elems,  (my_pe + 1) * elems, freq, prefix);

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

    random_fill(n, local_prob, input, output);

    shmem_free(input);
    shmem_free(output);

    shmem_finalize();

    return EXIT_SUCCESS;
}
