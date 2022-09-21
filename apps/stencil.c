/* This is a tiled implementation of a 5-point stencil. */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Size of small stencil. */
#define KERNELN 120
/* Iterations of small stencil. Must be odd. */
#define KERNELIT 11

#define WEIGHT1 0.25
#define WEIGHT2 0.25
#define WEIGHT3 0.25
#define WEIGHT4 0.25
#define WEIGHT5 0.25

/* Does a 5-point stencil of KERNELIT iterations on 
 * a KERNELN x KERNELN grid in, but only calculates the 
 * output for the inner (KERNELN - KERNELIT) x (KERNELN - KERNELIT)
 * grid. If KERNENLIT is odd, writes
 * the output to buffer, if even, writes it to in. This 
 * function is destructive, e.g. the input is gone afterwards. */
void kernel(double *in, double *buffer)
{
    for (int t = 1; t <= KERNELIT; t++) {
        for (size_t i = t; i < KERNELN - t; i++) {
            for (size_t j = t; j < KERNELN - t; j++) {
                buffer[i * KERNELN + j] = WEIGHT1 * in[(i - 1) * KERNELN + j] + 
                        WEIGHT2 * in[i * KERNELN + j - 1] + 
                        WEIGHT3 * in[i * KERNELN + j] + 
                        WEIGHT4 * in[i * KERNELN + j + 1] + 
                        WEIGHT5 * in[(i + 1) * KERNELN + j];
            }
        }
    }
}

/* Packs the KERNELN x KERNELN subsquare of in, with left-top corner
 * (i - KERNELIT, j - KERNELIT) into contiguous buffer kernelIn. */
void pack(size_t i, size_t j, double *in, double *kernelIn, size_t n)
{
    for (size_t row = i - KERNELIT; row < i - KERNELIT + KERNELN; row++) {
        memcpy(&kernelIn[row * KERNELN], &in[(i - KERNELIT) * n + j - KERNELIT],
                KERNELN * sizeof(double));
    }
}

/* Unpacks the inner (KERNELN - KERNELIT) x (KERNELN - KERNELIT) subsquare of 
 * KERNELN x KERNELN buffer kernelBuffer into the subsquare of buffer that has 
 * with left-top corner (i, j). */
void unpack(size_t i, size_t j, double *buffer, double *kernelBuffer, size_t n)
{
    for (size_t row = i - KERNELIT; row < i - KERNELIT + KERNELN; row++) {
        memcpy(&kernelBuffer[row * KERNELN], &buffer[(i - KERNELIT) * n + j - KERNELIT],
                KERNELN * sizeof(double));
    }
}


/* Does a KERNELIT iterations stencil on n x n grid in. Assumes KERNELIT
 * is odd, and writes the output to buffer. Is destructive.
 * For now, assume KERNELN | n. */
void stencilFixed(double *in, double *buffer, size_t n)
{
    double *kernelIn = malloc(KERNELN * KERNELN * sizeof(double));
    double *kernelBuffer = malloc(KERNELN * KERNELN * sizeof(double));

    /* Inner part of the grid. */
    size_t tileSize = KERNELN - 2 * KERNELIT;
    size_t start = KERNELN - KERNELIT;
    size_t end = n - (KERNELN - KERNELIT);
    for (size_t tilei = start; tilei < end; tilei += tileSize) {
        for (size_t tilej = start; tilej < end; tilej += tileSize) {
            pack(tilei, tilej, in, kernelIn, n);

            /* Do the kernel. */
            kernel(kernelIn, kernelBuffer);

            unpack(tilei, tilej, buffer, kernelBuffer, n);
        }
    }

    /* Boundary of the grid. */

    free(kernelIn);
    free(kernelBuffer);
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: size, iterations.\n");
        exit(EXIT_FAILURE);
    }

    size_t n = atol(argv[1]);
    size_t iterations = atol(argv[2]);

    double *in = malloc(n * n * sizeof(double));
    double *buffer = malloc(n * n * sizeof(double));

    time_t start = clock();
    stencilFixed(in, buffer, n);
    time_t end  = clock();
    double duration = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Time %lfs, n %zu, iterations %d, %lf Gflops/s\n", duration, n, KERNELIT, 
            9.0 * (n - 2) * (n - 2) * KERNELIT / 1000000000.0 / duration);

    free(in);
    free(buffer);

    return 0;
}
