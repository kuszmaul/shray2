#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <shray2/shray.h>

#define BLOCK 2000

/* The coefficients of the five-point stencil. */
const double a = 0.50;
const double b = 0.33;
const double c = 0.25;

void init(size_t n, double *arr)
{
    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
            arr[i] = i;
    }
}

/* Computes a stencil on inBuffer of size n + 2 * iterations, writing the result to outBuffer,
 * however only indices iterations, ..., iterations + n - 1 have the proper output. */
void middle(size_t n, int iterations, double **in, double **out)
{
    (*out)[0] = (*in)[0];
    (*out)[n + 2 * iterations - 1] = (*in)[n + 2 * iterations - 1];

    for (int t = 0; t < iterations; t++) {
        for (size_t i = 1 + t; i < n - 1 - t; i++) {
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

void left(size_t n, int iterations, double **in, double **out)
{
    (*out)[0] = (*in)[0];
    (*out)[n + iterations - 1] = (*in)[n + iterations - 1];

    for (int t = 0; t < iterations; t++) {
        for (size_t i = 1; i < n - 1 - t; i++) {
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
    (*out)[0] = (*in)[0];
    (*out)[n + iterations - 1] = (*in)[n + iterations - 1];

    for (int t = 0; t < iterations; t++) {
        for (size_t i = 1 + t; i < n - 1; i++) {
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
    double *inBuffer = malloc((iterations + 2 * iterations) * sizeof(double));
    double *outBuffer = malloc((iterations + 2 * iterations) * sizeof(double));

    for (size_t row = ShrayStart(n / iterations); row < ShrayEnd(n / iterations); row++) {
        if (row == 0) {
            memcpy(inBuffer, *in, (iterations + iterations) * sizeof(double));
            left(iterations, iterations, &inBuffer, &outBuffer);
        } else if (row == n / iterations - 1) {
            memcpy(inBuffer, *in + row * iterations - iterations,
                    (iterations + iterations) * sizeof(double));
            right(iterations, iterations, &inBuffer, &outBuffer);
        } else {
            memcpy(inBuffer, *in + row * iterations - iterations,
                    (iterations + 2 * iterations) * sizeof(double));
            middle(iterations, iterations, &inBuffer, &outBuffer);
        }

        memcpy(*out + row * iterations, outBuffer + iterations, iterations * sizeof(double));
    }

    ShraySync(*out);

    free(inBuffer);
    free(outBuffer);
}

void Stencil(size_t n, double **in, double **out, int iterations)
{
    for (int t = 0; t < iterations; t += BLOCK) {
        StencilBlocked(n, in, out, BLOCK);
    }
    if (iterations % BLOCK != 0) {
        StencilBlocked(n, in, out, iterations % BLOCK);
    }
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
    }

    /* Easy way to visualize a blocking algorithm is to reshape into
     * a matrix, and view each row as a block. We take iterations as block size*/
    double *in = ShrayMalloc(n / BLOCK, n * sizeof(double));
    double *out = ShrayMalloc(n / BLOCK, n * sizeof(double));

    init(n, in);
    ShraySync(in);

    double duration;
    TIME(duration, Stencil(n, &in, &out, iterations););

    if (ShrayOutput) {
        printf("%lf\n", 5.0 * (n - 2) * iterations / 1000000000.0 / duration);
    }

    ShrayFree(in);
    ShrayFree(out);

    ShrayReport();
    ShrayFinalize(0);
}
