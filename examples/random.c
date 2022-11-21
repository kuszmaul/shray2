#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <shray2/shray.h>

#define MAX(a, b) ((a) > (b)) ? (a) : (b)
#define MIN(a, b) ((a) > (b)) ? (b) : (a)

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
    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        input[i] = i;
    }
}

/* Returns an integer in [0, n] that is in [local_start, local_end[ with probability
 * localProp %. */
int rand_n_prob(size_t n, size_t local_start, size_t local_end, int localProp)
{
    int n_pes = ShraySize();

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
    /* Fill our portion of the output array */
    size_t shray_start = ShrayStart(n);
    size_t shray_end = ShrayEnd(n);
    for (size_t i = shray_start; i < shray_end; i++) {
        /* Generate a random index with a certain probability. */
        int r = rand_n_prob(n, shray_start, shray_end, local_prob);

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

    double duration;
    TIME(duration,
    random_fill(n, local_prob, input, output);
    ShraySync(output);
    );

    printf("This took %lfs.\n", duration);

    ShrayReport();

    ShrayFree(input);
    ShrayFree(output);

    ShrayFinalize(0);
}
