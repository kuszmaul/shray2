/* 1d stencil (Jacobi iteration). -ffast-math makes it slower?! Because it does not
 * use fma instructions for some reason. */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>

/* The coefficients of the three-point stencil. */
const double a = 0.50;
const double b = 0.33;
const double c = 0.25;

/* 80 kB per array, so should fit in a 256 kB L2 cache. */
#define BLOCK 10000

void init(size_t n, double *arr)
{
    for (size_t i = 0; i < n; i++) {
        arr[i] = i;
    }
}

void stencil(size_t n, double *in, double *out, int iterations)
{
    double *temp;

    out[0] = in[0];
    out[n - 1] = in[n - 1];

    for (int t = 1; t <= iterations; t++) {
        for (size_t i = 1; i < n - 1; i++) {
            out[i] = in[i - 1] * a + in[i] * b + in[i + 1] * c;
        }

        temp = in;
        in = out;
        out = temp;
    }

    /* We buffer swapped, so copy in to out in case iterations is even. */
    if (iterations % 2 == 0) {
        memcpy(out, in, n * sizeof(double));
    }
}

void stencil2(size_t n, double **in, double **out, int iterations)
{
    double *temp;

    (*out)[0] = (*in)[0];
    (*out)[n - 1] = (*in)[n - 1];

    for (int t = 1; t <= iterations; t++) {
        for (size_t i = 1; i < n - 1; i++) {
            (*out)[i] = (*in)[i - 1] * a + (*in)[i] * b + (*in)[i + 1] * c;
        }

        temp = *in;
        *in = *out;
        *out = temp;
    }

    temp = *in;
    *in = *out;
    *out = temp;
}

/* Idea: we calculate the stencil one block at a time. 
 * For that we take a slice of the input around the block with an 
 * extra boundary of thickness t. */
void stencilOpt(size_t n, double *in, double *out, int iterations)
{
    double *inBuffer = malloc((BLOCK + 2 * iterations) * sizeof(double));
    double *outBuffer = malloc((BLOCK + 2 * iterations) * sizeof(double));

    /* Inner part of the grid */
    assert(n % BLOCK == 0);
    for (size_t i = BLOCK; i < n - BLOCK; i += BLOCK) {

        /* Pack the block into the buffer. */
        memcpy(inBuffer, &in[i - iterations], (BLOCK + 2 * iterations) * sizeof(double));

        /* Perform the stencil. */
        stencil(BLOCK + 2 * iterations, inBuffer, outBuffer, iterations);

        /* Copy the result back to out. */
        memcpy(&out[i], &outBuffer[iterations], BLOCK * sizeof(double));
    }


    /* Boundary of the grid */
    memcpy(inBuffer, in, (BLOCK + iterations) * sizeof(double));
    stencil(BLOCK + iterations, inBuffer, outBuffer, iterations);
    memcpy(out, outBuffer, BLOCK * sizeof(double));

    memcpy(inBuffer, &in[n - BLOCK - iterations], (BLOCK + iterations) * sizeof(double));
    stencil(BLOCK + iterations, inBuffer, outBuffer, iterations);
    memcpy(&out[n - BLOCK], &outBuffer[iterations], BLOCK * sizeof(double));

    free(inBuffer);
    free(outBuffer);
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
    if (argc != 3) {
        printf("Please specify 2 arguments (n, iterations).\n");
        exit(1);
    }

    size_t n = atoll(argv[1]);
    int iterations = atoi(argv[2]);

    /* Pointers of type double[n], so a dereference [i] adds i * n * sizeof(double)
     * bytes to this pointer and then dereferences, aka i rows further. */
    double *in = malloc(n * sizeof(double));
    double *out = malloc(n * sizeof(double));
    double *out2 = malloc(n * sizeof(double));

    init(n, in);

    time_t start = clock();
    stencil(n, in, out, iterations);
    time_t end  = clock();
    double duration = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Time %lfs, n %zu, iterations %d, %lf Gflops/s\n", duration, n, iterations, 
            5.0 * (n - 2) * iterations / 1000000000.0 / duration);

    init(n, in);

    start = clock();
    stencil2(n, &in, &out2, iterations);
//    stencilOpt(n, in, out2, iterations);
    end = clock();
    duration = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Optimised: time %lfs, n %zu, iterations %d, %lf Gflops/s\n", duration, n, 
            iterations, 5.0 * (n - 2) * iterations / 1000000000.0 / duration);

    if (testEquality(out, out2, 0, n, 0.01)) {
        printf("Success!\n");
    } else {
        printf("Failure!\n");
    }

    free(in);
    free(out);
    free(out2);
}
