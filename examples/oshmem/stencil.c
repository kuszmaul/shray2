/* A five-point stencil. Not optimised, so heavily memory-bound. 
 * MKL contain an optimised version, but it is not possible to 
 * specify the number of iterations, it just continues until 
 * convergence. */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <shmem.h>

/* The coefficients of the five-point stencil. */
const double a = 0.50;
const double b = 0.33;
const double c = 0.25;
const double d = 0.25;
const double e = 0.25;

void init(size_t n, double *arr)
{
    for (size_t i = 0; i < n / shmem_n_pes(); i++) {
        for (size_t j = 0; j < n; j++) {
            arr[i * n + j] = i + j;
        }
    }
}

void relax(size_t n, double **in, double **out, double *firstBuffer, double *lastBuffer)
{
    int p = shmem_n_pes();

    /* Local part */
    for (size_t i = 1; i < n / p - 1; i++) {
        for (size_t j = 1; j < n - 1; j++) {
            (*out)[i * n + j] = a * (*in)[(i - 1) * n + j] + 
                             b * (*in)[i * n + j - 1] + 
                             c * (*in)[i * n + j] + 
                             d * (*in)[i * n + j + 1] + 
                             e * (*in)[(i + 1) * n + j];
        }
    }

    if (shmem_my_pe() != p - 1) {
        /* Last row */
        shmem_get(lastBuffer, (*in), n, shmem_my_pe() + 1);
        for (size_t j = 1; j < n - 1; j++) {
           (*out)[(n / p - 1) * n + j] = a * (*in)[(n / p - 2) * n + j] + 
                            b * (*in)[(n / p - 1) * n + j - 1] + 
                            c * (*in)[(n / p - 1) * n + j] + 
                            d * (*in)[(n / p - 1) * n + j + 1] + 
                            e * lastBuffer[j];
        }
    }

    if (shmem_my_pe() != 0) {
        /* First row */
        shmem_get(firstBuffer, *in + n * (n / p - 1), n, shmem_my_pe() - 1);
        for (size_t j = 1; j < n - 1; j++) {
           (*out)[j] = a * firstBuffer[j] + 
                            b * (*in)[j - 1] + 
                            c * (*in)[j] + 
                            d * (*in)[j + 1] + 
                            e * (*in)[n + j];
        }
    }
}

void stencil(size_t n, double **in, double **out, int iterations)
{
    int p = shmem_n_pes();

    double *firstBuffer = malloc(n * sizeof(double));
    double *lastBuffer = malloc(n * sizeof(double));

    /* First and last row. */
    if (shmem_my_pe() == 0) {
        for (size_t j = 0; j < n; j++) {
            (*out)[j] = (*in)[j];
        }
    }
    if (shmem_my_pe() == p - 1) {
        for (size_t j = 0; j < n; j++) {
            (*out)[(n / p - 1) * n + j] = (*in)[(n / p - 1) * n + j];
        }
    }

    /* First and last column */
    for (size_t i = 0; i < n / p; i++) {
        (*out)[i * n] = (*in)[i * n];
        (*out)[i * n + n / p - 1] = (*in)[i * n + n / p - 1];
    }
 
    /* Inner part */
    for (int t = 1; t < iterations; t++) {
        relax(n, in, out, firstBuffer, lastBuffer);
        shmem_barrier_all();

        /* Switch buffers. This is allowed because every processor is done writing to 
         * out at this point, hence does not need to read from in anymore. */
        double *temp = *in;
        *in = *out;
        *out = temp;
    }

    /* No buffer swap after the last iteration. */
    relax(n, in, out, firstBuffer, lastBuffer);
    shmem_barrier_all();

    free(firstBuffer);
    free(lastBuffer);
}

int main(int argc, char **argv)
{
    shmem_init();

    if (argc != 3) {
        printf("Please specify 2 arguments (n, iterations).\n");
        exit(1);
    }

    size_t n = atoll(argv[1]);
    int iterations = atoi(argv[2]);

    double *in = shmem_malloc(n / shmem_n_pes() * n * sizeof(double));
    double *out = shmem_malloc(n / shmem_n_pes() * n * sizeof(double));

    init(n, in);
    shmem_barrier_all();

    stencil(n, &in, &out, iterations);

    shmem_free(in);
    shmem_free(out);

    shmem_finalize();
}
