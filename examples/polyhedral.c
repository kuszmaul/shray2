/* 1d stencil (Jacobi iteration). -ffast-math makes it slower?! Because it does not
 * use fma instructions for some reason. */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>

size_t flops = 0;

/* The coefficients of the three-point stencil. */
const double a = 0.50;
const double b = 0.33;
const double c = 0.25;

/* 80 kB per array, so should fit in a 256 kB L2 cache. */
#define BLOCK 10

void init(size_t n, double *arr)
{
    for (size_t i = 0; i < n; i++) {
        arr[i] = i;
    }
}

void stencil(size_t n, double **in, double **out, unsigned int iterations)
{
    double *temp;

    (*out)[0] = (*in)[0];
    (*out)[n - 1] = (*in)[n - 1];

    for (unsigned int t = 1; t <= iterations; t++) {
        for (size_t i = 1; i < n - 1; i++) {
            (*out)[i] = (*in)[i - 1] * a + (*in)[i] * b + (*in)[i + 1] * c;
            fprintf(stderr, "Real out[%d][%zu] = %lf\n", t, i, (*out)[i]);
        }

        temp = *in;
        *in = *out;
        *out = temp;
    }

    temp = *in;
    *in = *out;
    *out = temp;
}

/* Afterwards out contains partial stencils at the sides, and the full stencil 
 * in the middle. */
void kernelTrapezoid(size_t n, double **in, double **out, unsigned int iterations)
{
    double *temp;

    (*out)[0] = (*in)[0];
    (*out)[n - 1] = (*in)[n - 1];

    for (unsigned int t = 1; t <= iterations; t++) {
        for (size_t i = t; i < n - t; i++) {
            (*out)[i] = (*in)[i - 1] * a + (*in)[i] * b + (*in)[i + 1] * c;
//            fprintf(stderr, "Fake out[%d][%zu] = %lf\n", t, i, (*out)[i]);
        }

        /* We need to preserve the partial stencil on the sides. */
        (*in)[t] = (*out)[t];
        (*in)[n - t - 1] = (*out)[n - t - 1];

        /* The output of this iteration is the input of the next iteration. */
        temp = *in;
        *in = *out;
        *out = temp;
    }

    /* There is no next iteration, so swap back. */
    temp = *in;
    *in = *out;
    *out = temp;
}

/* We have overwritten part of the boundary in the trapezoid part, that 
 * we need here. Need to rethink algorithm. */
void kernelTriangle(double **in, double **out, unsigned int iterations)
{
    double *temp;

    for (unsigned int t = 1; t <= iterations; t++) {
        for (size_t i = t; i < 2 * iterations + 2 - t; i++) {
            (*out)[i] = (*in)[i - 1] * a + (*in)[i] * b + (*in)[i + 1] * c;
            printf("Triangle out[%d][%zu] = %lf\n", t, i, (*out)[i]);
        }

        /* The output of this iteration is the input of the next iteration. */
        temp = *in;
        *in = *out;
        *out = temp;
    }

    /* There is no next iteration, so swap back. */
    temp = *in;
    *in = *out;
    *out = temp;
}

/* We use the polyhedral method, so if the y-axis is time, 
 *
 *   _    _    _    _    _    _    _    _    _    _    _    _ 
 *  / \  / \  / \  / \  / \  / \  / \  / \  / \  / \  / \  / \
 * /   \/   \/   \/   \/   \/   \/   \/   \/   \/   \/   \/   \
 *
 * where we first calculate the upwards facing trapezoids, and then 
 * the downwards facing triangles. */
void stencilOpt(size_t n, double *in, double *out, unsigned int iterations)
{
    double *inBuffer = malloc(BLOCK * sizeof(double));
    double *outBuffer = malloc(BLOCK * sizeof(double));

    if (inBuffer == NULL || outBuffer == NULL) {
        fprintf(stderr, "One of the swap buffers was NULL\n");
    }

    assert(n % BLOCK == 0);
    assert(BLOCK > 2 * iterations);

    /* Trapezoids */
    for (size_t block = 0; block < n / BLOCK; block++) {
        memcpy(inBuffer, in + block * BLOCK, BLOCK * sizeof(double));
        kernelTrapezoid(BLOCK, &inBuffer, &outBuffer, iterations);
        memcpy(out + block * BLOCK, outBuffer, BLOCK * sizeof(double));
    }

    printf("Trapezoid done\n");

    /* Triangle */
    for (size_t block = 1; block < n / BLOCK - 1; block++) {
        memcpy(inBuffer, out + block * BLOCK - iterations - 1, 
                2 * (iterations + 1) * sizeof(double));
        memcpy(outBuffer, out + block * BLOCK - iterations - 1, 
                2 * (iterations + 1) * sizeof(double));
        kernelTriangle(&inBuffer, &outBuffer, iterations);
        memcpy(out + block * BLOCK - iterations, outBuffer + 1, 2 * iterations * sizeof(double));
    }

    /* First and last triangle */

    free(inBuffer);
    free(outBuffer);
    printf("Frees done\n");
}

int testEquality(double *arr1, double *arr2, size_t startIndex, size_t endIndex, double epsilon)
{
    for (size_t i = startIndex; i < endIndex; i++) {
        if ((arr1[i] - arr2[i]) * (arr1[i] - arr2[i]) > epsilon) {
            printf("Position %ld: %lf != %lf\n", i, arr1[i], arr2[i]);
//            return 0;
        }
    }

    return 1;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Please specify 2 arguments (n, iterations).\n");
        exit(1);
    }

    size_t n = atoll(argv[1]);
    unsigned int iterations = atoi(argv[2]);

    /* Pointers of type double[n], so a dereference [i] adds i * n * sizeof(double)
     * bytes to this pointer and then dereferences, aka i rows further. */
    double *in = malloc(n * sizeof(double));
    double *out = malloc(n * sizeof(double));
    double *out2 = malloc(n * sizeof(double));

    init(n, in);

    time_t start = clock();
    stencil(n, &in, &out, iterations);
    time_t end  = clock();
    double duration = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Time %lfs, n %zu, iterations %d, %lf Gflops/s\n", duration, n, iterations, 
            5.0 * (n - 2) * iterations / 1000000000.0 / duration);

    init(n, in);

    start = clock();
    stencilOpt(n, in, out2, iterations);
    end = clock();
    duration = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Optimised: time %lfs, n %zu, iterations %d, %lf Gflops/s\n", duration, n, 
            iterations, 5.0 * (n - 2) * iterations / 1000000000.0 / duration);

    if (testEquality(out, out2, 0, n, 0.01)) {
        printf("Success!\n");
    } else {
        printf("Failure!\n");
    }

    printf("%zu flops\n", flops);
    free(in);
    free(out);
    free(out2);
}
