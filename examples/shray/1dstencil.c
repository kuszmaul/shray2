#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <shray2/shray.h>

#define T float

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) < (b)) ? (b) : (a))

#define BLOCK 10000
#define TIMEBLOCK 100
//#define CHECK

/* The coefficients of the five-point stencil. */
const T a = 0.50;
const T b = 0.33;
const T c = 0.25;

void init2d(T *arr, size_t n2)
{
    for (size_t i = ShrayStart(arr); i < ShrayEnd(arr); i++) {
        for (size_t j = 0; j < n2; j++) {
            arr[i * n2 + j] = i * n2 + j;
        }
    }
    ShraySync(arr);
}

void init(T *arr)
{
    for (size_t i = ShrayStart(arr); i < ShrayEnd(arr); i++) {
            arr[i] = i;
    }
    ShraySync(arr);
}

void naive(size_t n, int iterations, T **in, T **out)
{
    (*out)[0] = (*in)[0];
    (*out)[n - 1] = (*in)[n - 1];

    for (int t = 0; t < iterations; t++) {
        for (size_t i = MAX(ShrayStart(*in), 1); i < MIN(ShrayEnd(*in), n - 1); i++) {
            (*out)[i] = a * (*in)[i - 1] + b * (*in)[i] + c * (*in)[i + 1];
        }

        ShraySync(*out);

        /* Buffer swap to save memory. */
        if (t != iterations - 1) {
            T *temp = *in;
            *in = *out;
            *out = temp;
        }
    }
}

void left(size_t n, int iterations, T **in, T **out)
{
    (*out)[0] = (*in)[0];

    for (int t = 1; t <= iterations; t++) {
        for (size_t i = 1; i < n + iterations - t; i++) {
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
    for (int t = 1; t <= iterations; t++) {
        for (size_t i = t; i < n + 2 * iterations - t; i++) {
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

    for (int t = 1; t <= iterations; t++) {
        for (size_t i = t; i < n + iterations - 1; i++) {
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
void StencilBlocked(size_t n, T **in, T **out, int iterations)
{
    T *inBuffer = (T*)malloc((BLOCK + 2 * iterations) * sizeof(T));
    T *outBuffer = (T*)malloc((BLOCK + 2 * iterations) * sizeof(T));

    for (size_t row = ShrayStart(*in); row < ShrayEnd(*in); row++) {
        if (row == 0) {
            memcpy(inBuffer, *in, (BLOCK + iterations) * sizeof(T));
            left(BLOCK, iterations, &inBuffer, &outBuffer);
            memcpy(*out, outBuffer, BLOCK * sizeof(T));
        } else if (row == n / BLOCK - 1) {
            memcpy(inBuffer, *in + row * BLOCK - iterations,
                    (BLOCK + iterations) * sizeof(T));
            right(BLOCK, iterations, &inBuffer, &outBuffer);
            memcpy(*out + row * BLOCK, outBuffer + iterations, BLOCK * sizeof(T));
        } else {
            memcpy(inBuffer, *in + row * BLOCK - iterations,
                    (BLOCK + 2 * iterations) * sizeof(T));
            middle(BLOCK, iterations, &inBuffer, &outBuffer);
            memcpy(*out + row * BLOCK, outBuffer + iterations, BLOCK * sizeof(T));
        }
    }

    ShraySync(*out);

    free(inBuffer);
    free(outBuffer);
}

void Stencil(size_t n, T **in, T **out, int iterations)
{
    for (int t = TIMEBLOCK; t <= iterations; t += TIMEBLOCK) {
        StencilBlocked(n, in, out, TIMEBLOCK);
        T *temp = *out;
        *out = *in;
        *in = temp;
    }
    if (iterations % TIMEBLOCK != 0) {
        StencilBlocked(n, in, out, iterations % TIMEBLOCK);
    } else {
        /* We did one buffer swap too many */
        T *temp = *out;
        *out = *in;
        *in = temp;
    }
}

bool equal(T *x, T *y, size_t n, T error)
{
    for (size_t i = 0; i < n; i++) {
        if (MAX(x[i], y[i]) / MIN(x[i], y[i]) > error) {
            printf("Index %zu: %lf != %lf\n", i, x[i], y[i]);
            return false;
        }
    }

    return true;
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 3) {
        printf("Please specify 2 arguments (n, iterations).\n");
        ShrayFinalize(1);
    }

    size_t n = atoll(argv[1]);
    int iterations = atoi(argv[2]);

    if (n % BLOCK != 0) {
        printf("This is a prototype, please make sure n | %d. Suggestion: n = %zu\n",
                BLOCK, n / BLOCK * BLOCK);
        ShrayFinalize(1);
    }

    /* Easy way to visualize a blocking algorithm is to reshape into
     * a matrix, and view each row as a block. */
    T *in = (T*)ShrayMalloc(n / BLOCK, n * sizeof(T));
    T *out = (T*)ShrayMalloc(n / BLOCK, n * sizeof(T));
    init2d(in, BLOCK);

#ifdef CHECK
    T *in2 = ShrayMalloc(n, n * sizeof(T));
    T *out2 = ShrayMalloc(n, n * sizeof(T));
    init(in2);
#endif

    double duration;
    TIME(duration, Stencil(n, &in, &out, iterations););

    if (ShrayOutput) {
        printf("%lf\n", 5.0 * (n - 2) * iterations / 1000000000.0 / duration);
    }

#ifdef CHECK
    naive(n, iterations, &in2, &out2);

    if (equal(out, out2, n, 1.00001)) {
        printf("SUCCESS on processor %d!\n", ShrayRank());
    }
    ShrayFree(in2);
    ShrayFree(out2);
#endif

    ShrayFree(in);
    ShrayFree(out);

    ShrayReport();
    ShrayFinalize(0);
}
