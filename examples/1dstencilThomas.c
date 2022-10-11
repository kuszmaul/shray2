/* 1d stencil (Jacobi iteration). -ffast-math makes it slower?! Because it does not
 * use fma instructions for some reason. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <shray2/shray.h>
#include <assert.h>
#include <sys/time.h>

#define min(a, b) ((a) < (b)) ? (a) : (b)
#define max(a, b) ((a) < (b)) ? (b) : (a)

/* The coefficients of the three-point stencil. */
const double a = 0.50;
const double b = 0.33;
const double c = 0.25;

/* 80 kB per array, so should fit in a 256 kB L2 cache. */
#define BLOCK 20000

void init(size_t n, double *arr)
{
    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        arr[i] = i;
    }
}

void stencil(size_t n, double **in, double **out, int iterations)
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
    size_t start = ShrayStart(n);
    size_t end = ShrayEnd(n);

    assert((end - start) % BLOCK == 0);

    for (size_t i = max(start, BLOCK); (i < end) && (i < n - BLOCK); i += BLOCK) {

        /* Pack the block into the buffer. */
        memcpy(inBuffer, &in[i - iterations], (BLOCK + 2 * iterations) * sizeof(double));

        /* Perform the stencil. */
        stencil(BLOCK + 2 * iterations, &inBuffer, &outBuffer, iterations);

        /* Copy the result back to out. */
        memcpy(&out[i], &outBuffer[iterations], BLOCK * sizeof(double));
    }


    /* Boundary of the grid */
    if (ShrayStart(n) == 0) {
        memcpy(inBuffer, in, (BLOCK + iterations) * sizeof(double));
        stencil(BLOCK + iterations, &inBuffer, &outBuffer, iterations);
        memcpy(out, outBuffer, BLOCK * sizeof(double));
    }

    if (ShrayEnd(n) == n) {
        memcpy(inBuffer, &in[n - BLOCK - iterations], (BLOCK + iterations) * sizeof(double));
        stencil(BLOCK + iterations, &inBuffer, &outBuffer, iterations);
        memcpy(&out[n - BLOCK], &outBuffer[iterations], BLOCK * sizeof(double));
    }

    free(inBuffer);
    free(outBuffer);
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

    /* Pointers of type double[n], so a dereference [i] adds i * n * sizeof(double)
     * bytes to this pointer and then dereferences, aka i rows further. */
    double *in = ShrayMalloc(n, n * sizeof(double));
    double *out = ShrayMalloc(n, n * sizeof(double));

    init(n, in);
    ShraySync(in);

    double duration;

    TIME(duration, stencilOpt(n, in, out, iterations); ShraySync(out););

    printf("Time %lfs, n %zu, iterations %d, %lf Gflops/s\n", duration, n, 
            iterations, 5.0 * (n - 2) * iterations / 1000000000.0 / duration);

    ShrayReport();

    ShrayFree(in);
    ShrayFree(out);

    ShrayFinalize(0);
}
