/* basic 1d stencil with shray. */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include <shray2/shray.h>

/* The coefficients of the three-point stencil. */
const double a = 0.50;
const double b = 0.33;
const double c = 0.25;

void init(size_t n, double *arr)
{
    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        arr[i] = i;
    }
}

void stencil_alloc(size_t n, double **in, double **out, int iterations)
{
    for (int t = 1; t <= iterations; t++) {
        *out = ShrayMalloc(n, n * sizeof(double));

        /* set boundary values */
        size_t start = ShrayStart(n);
        size_t end = ShrayEnd(n);
        if (start == 0) {
            (*out)[start] = (*in)[start];
        }
        if (end == n) {
            (*out)[end - 1] = (*in)[end - 1];
        }

        for (size_t i = ShrayStartOffset(n, 1); i < ShrayEndOffset(n, n - 1); i++) {
            (*out)[i] = (*in)[i - 1] * a + (*in)[i] * b + (*in)[i + 1] * c;
        }

        ShraySync(*out);

        ShrayFree(*in);
        *in = *out;
    }

    *out = *in;
}

void stencil_reuse(size_t n, double **in, double **out, int iterations)
{
    double *tmp;

    /* set boundary values */
    size_t start = ShrayStart(n);
    size_t end = ShrayEnd(n);
    if (start == 0) {
        (*out)[start] = (*in)[start];
    }
    if (end == n) {
        (*out)[end - 1] = (*in)[end - 1];
    }
    ShraySync(*out);

    for (int t = 1; t <= iterations; t++) {
        for (size_t i = ShrayStartOffset(n, 1); i < ShrayEndOffset(n, n - 1); i++) {
            (*out)[i] = (*in)[i - 1] * a + (*in)[i] * b + (*in)[i + 1] * c;
        }

        ShraySync(*out);
        tmp = *in;
        *in = *out;
        *out = tmp;
    }

    tmp = *in;
    *in = *out;
    *out = tmp;
}

int testEquality(double *arr1, double *arr2, size_t startIndex, size_t endIndex, double epsilon)
{
    for (size_t i = startIndex; i < endIndex; i++) {
        if ((arr1[i] - arr2[i]) * (arr1[i] - arr2[i]) > epsilon) {
            printf("Position %ld: %lf != %lf\n", i, arr1[i], arr2[i]);
            return 0;
        }
    }

    return 1;
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 3) {
        printf("Please specify 2 arguments (n, iterations).\n");
        exit(1);
    }

    size_t n = atoll(argv[1]);
    int iterations = atoi(argv[2]);

    double *in = ShrayMalloc(n, n * sizeof(double));
    double *out = ShrayMalloc(n, n * sizeof(double));
    double *out2 = ShrayMalloc(n, n * sizeof(double));

    /* reuse method */
    init(n, in);
    ShraySync(in);
    time_t start = clock();
    stencil_reuse(n, &in, &out, iterations);
    time_t end  = clock();
    double duration = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Time reuse %lfs, n %zu, iterations %d, %lf Gflops/s\n", duration, n, iterations,
            5.0 * (n - 2) * iterations / 1000000000.0 / duration);

    /* allocation method */
    init(n, in);
    ShraySync(in);
    start = clock();
    stencil_alloc(n, &in, &out2, iterations);
    end  = clock();
    duration = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Time alloc %lfs, n %zu, iterations %d, %lf Gflops/s\n", duration, n, iterations,
            5.0 * (n - 2) * iterations / 1000000000.0 / duration);

    /* validate end result */
    if (ShrayStart(n) == 0) {
        if (testEquality(out, out2, 0, n, 0.01)) {
            printf("Success!\n");
        } else {
            printf("Failure!\n");
        }
    }

    ShrayFree(out);
    ShrayFree(out2);

    ShrayFinalize(0);
}
