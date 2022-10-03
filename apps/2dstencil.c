/* A five-point stencil. Not optimised, so heavily memory-bound. */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "../include/shray.h"

/* The coefficients of the five-point stencil. */
const double a = 0.50;
const double b = 0.33;
const double c = 0.25;
const double d = 0.25;
const double e = 0.25;

void init(size_t n, double *arr)
{
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            arr[i * n + j] = i + j;
        }
    }
}

void relax(size_t n, double **in, double **out)
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
    printf("Copied first and last column / rows\n");

    /* Inner part */
    for (size_t i = MAX(ShrayStart(n), 1); i < MIN(ShrayEnd(n), n - 1); i++) {
        for (size_t j = 1; j < n - 1; j++) {
            (*out)[i * n + j] = a * (*in)[(i - 1) * n + j] + 
                             b * (*in)[i * n + j - 1] + 
                             c * (*in)[i * n + j] + 
                             d * (*in)[i * n + j + 1] + 
                             e * (*in)[(i - 1) * n + j];
        }
    }
}

void stencil(size_t n, double **in, double **out, int iterations)
{
    for (int t = 1; t < iterations; t++) {
        relax(n, in, out);
        printf("in: %p, out: %p\n", *in, *out);
        ShraySync(out);

        /* Switch buffers */
        double *temp = *in;
        *in = *out;
        *out = temp;
        printf("in: %p, out: %p\n", *in, *out);
    }

    /* No buffer swap after the last iteration. */
    relax(n, in, out);
    ShraySync(out);
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv, 1000000);

    if (argc != 3) {
        printf("Please specify 2 arguments (n, iterations).\n");
        exit(1);
    }

    size_t n = atoll(argv[1]);
    int iterations = atoi(argv[2]);

    double *in = ShrayMalloc(n, n * n * sizeof(double));
    double *out = ShrayMalloc(n, n * n * sizeof(double));

    printf("Allocation succesfull\n");

    init(n, in);
    ShraySync(in);

    printf("Init succesfull\n");

    time_t start = clock();
    stencil(n, &in, &out, iterations);
    time_t end  = clock();
    double duration = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Time %lfs, n %zu, iterations %d, %lf Gflops/s\n", duration, n, iterations, 
            5.0 * (n - 2) * iterations / 1000000000.0 / duration);

    ShrayFree(in);
    ShrayFree(out);

    ShrayReport();
    ShrayFinalize();
}
