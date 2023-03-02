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

void init(double *input)
{
    for (size_t i = ShrayStart(input); i < ShrayEnd(input); i++) {
        input[i] = 1.0;
    }
}

/* Returns an integer in [0, n] that is in [local_start, local_end[ with probability
 * localProp %. */
int rand_n_prob(size_t local_start, size_t local_end, int localProp, size_t size)
{
    int n_pes = ShraySize();

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

    size_t shray_start = ShrayStart(input);
    size_t shray_end = ShrayEnd(input);

    for (size_t i = 0; i < n; i++) {
        size_t r = rand_n_prob(shray_start, shray_end, local_prob, size);
        result += input[r];
    }

    return result;
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 4) {
        fprintf(stderr, "Usage: array size, number of accesses, probability local access."
                " Use SHRAY_CACHEsize=4096\n");
        ShrayFinalize(1);
    }

    size_t size = atoll(argv[1]);
    size_t n = atoll(argv[2]);
    int local_prob = atoi(argv[3]);

    if (local_prob > 100) {
        fprintf(stderr, "Probability can not be greater than 100\n");
        ShrayFinalize(1);
    }

    srand(time(NULL));

    double *input = (double *)ShrayMalloc(size, size * sizeof(double));

    init(input);
    ShraySync(input);

    double duration;
    double result;
    TIME(duration,
    result = random_reduce(n, local_prob, input, size);
    );

    if (ShrayOutput) printf("%lf\n", duration);

    if (result == (double)n) {
        fprintf(stderr, "Success!\n");
    } else {
        fprintf(stderr, "Failure! Result = %lf\n", result);
    }

    ShrayReport();

    ShrayFree(input);

    ShrayFinalize(0);
}
