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

/* The coefficients of the five-point stencil. */
const double a = 0.50;
const double b = 0.33;
const double c = 0.25;
const double d = 0.25;
const double e = 0.25;

void init(size_t n, double *arr)
{
    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        for (size_t j = 0; j < n; j++) {
            arr[i * n + j] = i + j;
        }
    }
}

void relax(size_t n, double **in, double **out)
{
    size_t start = MAX(ShrayStart(n), 1);
    size_t end = MIN(ShrayEnd(n), n - 1);
    ShrayGet((*in) + start * n, n * sizeof(double));
    ShrayGet((*in) + (end - 1) * n, n * sizeof(double));

    /* Inner part */
    for (size_t i = start + 1; i < end - 1; i++) {
        for (size_t j = 1; j < n - 1; j++) {
            (*out)[i * n + j] = a * (*in)[(i - 1) * n + j] +
                             b * (*in)[i * n + j - 1] +
                             c * (*in)[i * n + j] +
                             d * (*in)[i * n + j + 1] +
                             e * (*in)[(i + 1) * n + j];
        }
    }

    ShrayGetComplete((*in) + start * n);
    ShrayGetComplete((*in) + (end - 1) * n);

    for (size_t j = 1; j < n - 1; j++) {
        (*out)[start * n + j] = a * (*in)[(start - 1) * n + j] +
                         b * (*in)[start * n + j - 1] +
                         c * (*in)[start * n + j] +
                         d * (*in)[start * n + j + 1] +
                         e * (*in)[(start + 1) * n + j];
    }

    for (size_t j = 1; j < n - 1; j++) {
        (*out)[start * n + j] = a * (*in)[(end - 1 - 1) * n + j] +
                         b * (*in)[(end - 1) * n + j - 1] +
                         c * (*in)[(end - 1) * n + j] +
                         d * (*in)[(end - 1) * n + j + 1] +
                         e * (*in)[(end - 1 + 1) * n + j];
    }

    ShrayGetFree((*in) + start * n);
    ShrayGetFree((*in) + (end - 1) * n);
}

void stencil(size_t n, double **in, double **out, int iterations)
{
    /* First and last row. */
    if (ShrayStart(n) == 0) {
        for (size_t j = 0; j < n; j++) {
            (*out)[j] = (*in)[j];
        }
    }
    if (ShrayStart(n) == n - 1) {
        for (size_t j = 0; j < n; j++) {
            (*out)[(n - 1) * n + j] = (*in)[(n - 1) * n + j];
        }
    }

    /* First and last column */
    for (size_t i = MAX(ShrayStart(n), 1); i < MIN(ShrayEnd(n), n - 1); i++) {
        (*out)[i * n] = (*in)[i * n];
        (*out)[i * n + n - 1] = (*in)[i * n + n - 1];
    }

    /* Inner part */
    for (int t = 1; t < iterations; t++) {
        relax(n, in, out);
        ShraySync(*out);

        /* Switch buffers. This is allowed because every processor is done writing to
         * out at this point, hence does not need to read from in anymore. */
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

    double *in = ShrayMalloc(n, n * n * sizeof(double));
    double *out = ShrayMalloc(n, n * n * sizeof(double));

    init(n, in);
    ShraySync(in);

    double duration;
    TIME(duration, stencil(n, &in, &out, iterations););

    if (ShrayOutput) {
        printf("Time %lfs, %lf Gflops/s\n", duration,
                9.0 * (n - 2) * (n - 2) * iterations / 1000000000.0 / duration);
    }

    ShrayFree(in);
    ShrayFree(out);

    ShrayReport();
    ShrayFinalize(0);
}
