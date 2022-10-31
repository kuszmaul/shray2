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

void random_fill(size_t n, int local_prob, double *input, double *output)
{
    size_t size = ShraySize();
    size_t rank = ShrayRank();
    int *freq = malloc(size * sizeof(int));
    int *prefix = malloc(size * sizeof(int));
    if (!freq || !prefix) {
        fprintf(stderr, "allocation for frequency data failed\n");
        ShrayFinalize(1);
    }

    int other_rank_prob = MAX((100 - local_prob) / MAX(size - 1, 1), 1);
    for (size_t i = 0; i < size; ++i) {
        freq[i] = i == rank ? local_prob : other_rank_prob;
    }

    /* Fill our portion of the output array */
    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        /* Determine a rank depending on the probability */
        int target_rank = rand_freq(freq, prefix, size);

        /* Determine the actual index */
        size_t start = ShrayStartRank(n, target_rank);
        size_t end = ShrayEndRank(n, target_rank);
        size_t index = rand() % (end - start);
        output[i] = input[start + index];
    }

    free(freq);
    free(prefix);
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
