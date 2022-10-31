#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <shray2/shray.h>

void init(size_t n, double *input)
{
    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        input[i] = i;
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
    int other_rank_prob = MAX((100 - local_prob) / 2, 1);
    int freq[3] = { other_rank_prob, local_prob, other_rank_prob };
    int prefix[3] = { 0, 0, 0 };

    /* Fill our portion of the output array */
    size_t shray_start = ShrayStart(n);
    size_t shray_end = ShrayEnd(n);
    for (size_t i = shray_start; i < shray_end; i++) {
        /* Generate a random index with a certain probability. */
        int r = rand_n_prob(n, shray_start, shray_end, freq, prefix);

        /* Do an operation on that random index */
        output[i] = input[r];
    }
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 3) {
        fprintf(stderr, "Usage: n, probability local access\n");
        ShrayFinalize(1);
    }

    size_t n = atoll(argv[1]);
    size_t local_prob = atoll(argv[2]);
    if (local_prob > 100) {
        fprintf(stderr, "Probability can not be greater than 100\n");
        ShrayFinalize(1);
    }

    srand(time(NULL));

    double *input = ShrayMalloc(n, n * sizeof(double));
    double *output = ShrayMalloc(n, n * sizeof(double));

    init(n, input);
    ShraySync(input);

    random_fill(n, local_prob, input, output);
    ShraySync(output);

    ShrayReport();

    ShrayFree(input);
    ShrayFree(output);

    ShrayFinalize(0);
}
