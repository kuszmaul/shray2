#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <ga.h>
#include <mpi.h>
#include <sys/param.h>

void init(int g_a)
{
	int rank = GA_Nodeid();
	int lo[1], hi[1], ld[1];
	double *a;

	NGA_Distribution(g_a, rank, lo, hi);
	NGA_Access(g_a, lo, hi, &a, ld);

	for (int i = lo[0]; i <= hi[0]; ++i) {
		a[i] = i;
	}

	NGA_Release(g_a, lo, hi);
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

void random_fill(size_t n, int local_prob, int g_input, int g_output)
{
    int rank = GA_Nodeid();
    int lo_out[1], hi_out[1], ld_out[1];
    double *output;

    int lo_in[1], hi_in[1], ld_in[1];
    double input;

    int other_rank_prob = MAX((100 - local_prob) / 2, 1);
    int freq[3] = { other_rank_prob, local_prob, other_rank_prob };
    int prefix[3] = { 0, 0, 0 };

    NGA_Distribution(g_output, rank, lo_out, hi_out);
    NGA_Access(g_output, lo_out, hi_out, &output, ld_out);

    /* Fill our portion of the output array */
    int ga_start = lo_out[0];
    int ga_end = hi_out[0];
    for (int i = ga_start; i <= ga_end; i++) {
        /* Generate a random index with a certain probability. */
        int r = rand_n_prob(n, ga_start, ga_end, freq, prefix);

        lo_in[0] = r;
        hi_in[0] = r;
        ld_in[0] = 1;
        NGA_Get(g_input, lo_in, hi_in, &input, ld_in);

        /* Do an operation on that random index */
        output[i] = input;
    }

    NGA_Release(g_output, lo_out, hi_out);
}

int main(int argc, char **argv)
{
    int heap = 3000000;
    int stack = 3000000;

    MPI_Init(&argc,&argv);
    GA_Initialize();

    if (argc != 3) {
        GA_Error("Usage: n, probability local access\n", 1);
    }

    size_t n = atoll(argv[1]);
    size_t local_prob = atoll(argv[2]);
    if (local_prob > 100) {
        GA_Error("Probability can not be greater than 100\n", 1);
    }

    srand(time(NULL));

    int nprocs = GA_Nnodes();

    int dimensions[1] = { n };
    int chunks[1] = { n / nprocs };

    int g_input = NGA_Create(C_DBL, 1, dimensions, "input", chunks);
    if (!g_input) {
        GA_Error("Could not allocate input array", 1);
    }

    int g_output = NGA_Duplicate(g_input, "output");
    if (!g_output) {
        GA_Error("Could not allocate output", 1);
    }

    init(g_input);
    GA_Sync();

    random_fill(n, local_prob, g_input, g_output);
    GA_Sync();

    GA_Destroy(g_input);
    GA_Destroy(g_output);
    GA_Terminate();
    MPI_Finalize();

    return EXIT_SUCCESS;
}
