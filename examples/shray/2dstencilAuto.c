/* A five-point stencil. Not optimised, so heavily memory-bound.
 * MKL contain an optimised version, but it is not possible to
 * specify the number of iterations, it just continues until
 * convergence. */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <shray2/shray.h>
#include "../util/time.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) > (b)) ? (b) : (a))

/* The coefficients of the five-point stencil. */
const double a = 0.50;
const double b = 0.33;
const double c = 0.25;
const double d = 0.25;
const double e = 0.25;

void init(size_t n, double *arr)
{
    for (size_t i = ShrayStart(arr); i < ShrayEnd(arr); i++) {
        for (size_t j = 0; j < n; j++) {
            arr[i * n + j] = i + j;
        }
    }
}

void relax(size_t n, double **in, double **out)
{
    size_t start = MAX(ShrayStart(*in), 1);
    size_t end = MIN(ShrayEnd(*in), n - 1);

    for (size_t i = start; i < end; i++) {
        for (size_t j = 1; j < n - 1; j++) {
            (*out)[i * n + j] = a * (*in)[(i - 1) * n + j] +
                             b * (*in)[i * n + j - 1] +
                             c * (*in)[i * n + j] +
                             d * (*in)[i * n + j + 1] +
                             e * (*in)[(i + 1) * n + j];
        }
    }
}

void stencil(size_t n, double **in, double **out, int iterations)
{
    /* First and last row. */
    if (ShrayStart(*in) == 0) {
        for (size_t j = 0; j < n; j++) {
            (*out)[j] = (*in)[j];
        }
    }

    if (ShrayEnd(*in) == n - 1) {
        for (size_t j = 0; j < n; j++) {
            (*out)[(n - 1) * n + j] = (*in)[(n - 1) * n + j];
        }
    }

    /* First and last column */
    for (size_t i = MAX(ShrayStart(*in), 1); i < MIN(ShrayEnd(*in), n - 1); i++) {
        (*out)[i * n] = (*in)[i * n];
        (*out)[i * n + n - 1] = (*in)[i * n + n - 1];
    }

    /* Inner part */
    for (int t = 1; t < iterations; t++) {
        relax(n, in, out);
        ShraySync(*out);

        /* Switch buffers. */
        double *temp = *in;
        *in = *out;
        *out = temp;
    }

    /* No buffer swap after the last iteration. */
    relax(n, in, out);
    ShraySync(*out);
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

    double *in = (double *)ShrayMalloc(n, n * n * sizeof(double));
    double *out = (double *)ShrayMalloc(n, n * n * sizeof(double));

    init(n, in);
    ShraySync(in);

    double duration;
    TIME(duration, stencil(n, &in, &out, iterations););

    if (ShrayOutput) {
        printf("%lf\n", 9.0 * (n - 2) * (n - 2) * iterations / 1000000000.0 / duration);
    }

    ShrayFree(in);
    ShrayFree(out);

    ShrayReport();
    ShrayFinalize(0);
}
