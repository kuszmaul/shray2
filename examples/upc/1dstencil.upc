#include <upc.h>
#include <upc_tick.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define TIME(duration, fncalls)                                        \
    {                                                                  \
        upc_barrier;                                                   \
        upc_tick_t start = upc_ticks_now();                            \
        fncalls                                                        \
        upc_barrier;                                                   \
        upc_tick_t end = upc_ticks_now();                              \
        duration = upc_ticks_to_ns(end - start) / 1000000000.0;        \
    }

#define T float

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define BLOCK 10000
#define TIMEBLOCK 100

/* The coefficients of the five-point stencil. */
const T a = 0.50;
const T b = 0.33;
const T c = 0.25;

void init(shared T *arr, size_t n)
{
    for (size_t i = MYTHREAD * n / THREADS; i < (MYTHREAD + 1) * n / THREADS; i++) {
            arr[i] = i;
    }
    upc_barrier;
}

void left(size_t n, int iterations, T **in, T **out)
{
    (*out)[0] = (*in)[0];

    for (int t = 0; t < iterations; t++) {
        for (size_t i = 1; i < n + iterations - 1 - t; i++) {
            (*out)[i] = a * (*in)[i - 1] + b * (*in)[i] + c * (*in)[i + 1];
        }

        /* Buffer swap to save memory. */
        if (t != iterations - 1) {
            T *temp = *in;
            *in = *out;
            *out = temp;
        }
    }
}

/* Computes a stencil on inBuffer of size n + 2 * iterations, writing the result to outBuffer,
 * however only indices iterations, ..., iterations + n - 1 have the proper output. */
void middle(size_t n, int iterations, T **in, T **out)
{
    for (int t = 0; t < iterations; t++) {
        for (size_t i = 1 + t; i < n + 2 * iterations - 1 - t; i++) {
            (*out)[i] = a * (*in)[i - 1] + b * (*in)[i] + c * (*in)[i + 1];
        }

        /* Buffer swap to save memory. */
        if (t != iterations - 1) {
            T *temp = *in;
            *in = *out;
            *out = temp;
        }
    }
}

void right(size_t n, int iterations, T **in, T **out)
{
    (*out)[n + iterations - 1] = (*in)[n + iterations - 1];

    for (int t = 0; t < iterations; t++) {
        for (size_t i = 1 + t; i < n + iterations - 1; i++) {
            (*out)[i] = a * (*in)[i - 1] + b * (*in)[i] + c * (*in)[i + 1];
        }

        /* Buffer swap to save memory. */
        if (t != iterations - 1) {
            T *temp = *in;
            *in = *out;
            *out = temp;
        }
    }
}

/* We use iterations as blocksize. This increases the number of flops performed by a
 * factor 2. */
void StencilBlocked(size_t n, shared T **in, shared T **out, int iterations)
{
    T *inBuffer = malloc((BLOCK + 2 * iterations) * sizeof(T));
    T *outBuffer = malloc((BLOCK + 2 * iterations) * sizeof(T));

    /* We have n / BLOCK blocks, that are distributed blockwise (I know, unfortunate naming)
     * over THREADS threads. */
    size_t distributionBlockSize = (n / BLOCK + THREADS - 1) / THREADS;
    size_t start = distributionBlockSize * MYTHREAD;
    size_t end = MIN(distributionBlockSize * (MYTHREAD + 1), n / BLOCK);
    for (size_t row = start; row < end; row++) {
        if (row == 0) {
            memcpy(inBuffer, (T *)*in, (BLOCK + iterations) * sizeof(T));
            left(BLOCK, iterations, &inBuffer, &outBuffer);
            memcpy((T *)*out, outBuffer, BLOCK * sizeof(T));
        } else if (row == n / BLOCK - 1) {
            memcpy(inBuffer, (T *)*in + row * BLOCK - iterations,
                    (BLOCK + iterations) * sizeof(T));
            right(BLOCK, iterations, &inBuffer, &outBuffer);
            memcpy((T *)*out + row * BLOCK, outBuffer + iterations, BLOCK * sizeof(T));
        } else {
            memcpy(inBuffer, (T *)*in + row * BLOCK - iterations,
                    (BLOCK + 2 * iterations) * sizeof(T));
            middle(BLOCK, iterations, &inBuffer, &outBuffer);
            memcpy((T *)*out + row * BLOCK, outBuffer + iterations, BLOCK * sizeof(T));
        }
    }

    upc_barrier;

    free(inBuffer);
    free(outBuffer);
}

void Stencil(size_t n, shared T **in, shared T **out, int iterations)
{
    for (int t = TIMEBLOCK; t <= iterations; t += TIMEBLOCK) {
        StencilBlocked(n, in, out, TIMEBLOCK);
        shared T *temp = *out;
        *out = *in;
        *in = temp;
    }
    if (iterations % TIMEBLOCK != 0) {
        StencilBlocked(n, in, out, iterations % TIMEBLOCK);
    } else {
        /* We did one buffer swap too many */
        shared T *temp = *out;
        *out = *in;
        *in = temp;
    }
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Please specify 2 arguments (n, iterations).\n");
        exit(EXIT_FAILURE);
    }

    size_t n = atoll(argv[1]);
    int iterations = atoi(argv[2]);

    if (n % BLOCK != 0) {
        printf("This is a prototype, please make sure n divides blocksize %d and "
                "number of threads. Suggestion: n = %zu\n",
                BLOCK, n / (BLOCK * THREADS) * (BLOCK * THREADS));
        exit(EXIT_FAILURE);
    }

    if (n % THREADS != 0) {
        printf("Please make sure the number of processors divides n. Suggestion: n = %zu.\n",
                n / THREADS * THREADS);
    }

    /* Easy way to visualize a blocking algorithm is to reshape into
     * a matrix, and view each row as a block. We take iterations as block size. */
    /* FIXME don't we want to have blocksize n / THREADS * sizeof(T) ? */
    shared T *in = upc_all_alloc(THREADS, n * sizeof(T));
    shared T *out = upc_all_alloc(THREADS, n * sizeof(T));
    init(in, n);

    double duration;
    TIME(duration, Stencil(n, &in, &out, iterations););

    if (MYTHREAD == 0) {
        printf("%lf\n", 5.0 * (n - 2) * iterations / 1000000000.0 / duration);
        upc_free(in);
        upc_free(out);
    }

    exit(EXIT_SUCCESS);
}
