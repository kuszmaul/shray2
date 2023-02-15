#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <shray2/shray.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) < (b)) ? (b) : (a))

#define BLOCK 10000
#define TIMEBLOCK 100
//#define CHECK

/* The coefficients of the five-point stencil. */
const double a = 0.50;
const double b = 0.33;
const double c = 0.25;

void init2d(double *arr, size_t n2)
{
    for (size_t i = ShrayStart(arr); i < ShrayEnd(arr); i++) {
        for (size_t j = 0; j < n2; j++) {
            arr[i * n2 + j] = i * n2 + j;
        }
    }
    ShraySync(arr);
}

void init(double *arr)
{
    for (size_t i = ShrayStart(arr); i < ShrayEnd(arr); i++) {
            arr[i] = i;
    }
    ShraySync(arr);
}

void naive(size_t n, int iterations, double **in, double **out)
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
            double *temp = *in;
            *in = *out;
            *out = temp;
        }
    }
}

void left(size_t n, int iterations, double **in, double **out)
{
    (*out)[0] = (*in)[0];

    for (int t = 0; t < iterations; t++) {
        for (size_t i = 1; i < n + iterations - 1 - t; i++) {
            (*out)[i] = a * (*in)[i - 1] + b * (*in)[i] + c * (*in)[i + 1];
        }

        /* Buffer swap to save memory. */
        if (t != iterations - 1) {
            double *temp = *in;
            *in = *out;
            *out = temp;
        }
    }
}

/* Computes a stencil on inBuffer of size n + 2 * iterations, writing the result to outBuffer,
 * however only indices iterations, ..., iterations + n - 1 have the proper output. */
void middle(size_t n, int iterations, double **in, double **out)
{
    for (int t = 0; t < iterations; t++) {
        for (size_t i = 1 + t; i < n + 2 * iterations - 1 - t; i++) {
            (*out)[i] = a * (*in)[i - 1] + b * (*in)[i] + c * (*in)[i + 1];
        }

        /* Buffer swap to save memory. */
        if (t != iterations - 1) {
            double *temp = *in;
            *in = *out;
            *out = temp;
        }
    }
}

void right(size_t n, int iterations, double **in, double **out)
{
    (*out)[n + iterations - 1] = (*in)[n + iterations - 1];

    for (int t = 0; t < iterations; t++) {
        for (size_t i = 1 + t; i < n + iterations - 1; i++) {
            (*out)[i] = a * (*in)[i - 1] + b * (*in)[i] + c * (*in)[i + 1];
        }

        /* Buffer swap to save memory. */
        if (t != iterations - 1) {
            double *temp = *in;
            *in = *out;
            *out = temp;
        }
    }
}

/* We use iterations as blocksize. This increases the number of flops performed by a
 * factor 2. */
void StencilBlocked(size_t n, double **in, double **out, int iterations)
{
    double *inBuffer = malloc((BLOCK + 2 * iterations) * sizeof(double));
    double *outBuffer = malloc((BLOCK + 2 * iterations) * sizeof(double));

    for (size_t row = ShrayStart(*in); row < ShrayEnd(*in); row++) {
        if (row == 0) {
            memcpy(inBuffer, *in, (BLOCK + iterations) * sizeof(double));
            left(BLOCK, iterations, &inBuffer, &outBuffer);
            memcpy(*out, outBuffer, BLOCK * sizeof(double));
        } else if (row == n / BLOCK - 1) {
            memcpy(inBuffer, *in + row * BLOCK - iterations,
                    (BLOCK + iterations) * sizeof(double));
            right(BLOCK, iterations, &inBuffer, &outBuffer);
            memcpy(*out + row * BLOCK, outBuffer + iterations, BLOCK * sizeof(double));
        } else {
            memcpy(inBuffer, *in + row * BLOCK - iterations,
                    (BLOCK + 2 * iterations) * sizeof(double));
            middle(BLOCK, iterations, &inBuffer, &outBuffer);
            memcpy(*out + row * BLOCK, outBuffer + iterations, BLOCK * sizeof(double));
        }
    }

    ShraySync(*out);

    free(inBuffer);
    free(outBuffer);
}

void Stencil(size_t n, double **in, double **out, int iterations)
{
    for (int t = TIMEBLOCK; t <= iterations; t += TIMEBLOCK) {
        if (ShrayRank() == 0) printf("%d iterations\n", TIMEBLOCK);
        StencilBlocked(n, in, out, TIMEBLOCK);
        double *temp = *out;
        *out = *in;
        *in = temp;
    }
    if (iterations % TIMEBLOCK != 0) {
        if (ShrayRank() == 0) printf("%d iterations\n", iterations % TIMEBLOCK);
        StencilBlocked(n, in, out, iterations % TIMEBLOCK);
    } else {
        /* We did one buffer swap too many */
        double *temp = *out;
        *out = *in;
        *in = temp;
    }
}

bool equal(double *x, double *y, size_t n, double error)
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
     * a matrix, and view each row as a block. We take iterations as block size*/
    double *in = ShrayMalloc(n / BLOCK, n * sizeof(double));
    double *out = ShrayMalloc(n / BLOCK, n * sizeof(double));
    init2d(in, BLOCK);

#ifdef CHECK
    double *in2 = ShrayMalloc(n, n * sizeof(double));
    double *out2 = ShrayMalloc(n, n * sizeof(double));
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
