#include "../util/time.h"
#include "../util/host.h"

#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <ga.h>
#include <mpi.h>
#include <sys/param.h>
#include <macdecls.h>

#define BLOCK 10000
#define TIMEBLOCK 100

/* The coefficients of the five-point stencil. */
const float a = 0.50;
const float b = 0.33;
const float c = 0.25;

void left(size_t n, int iterations, float **in, float **out)
{
    (*out)[0] = (*in)[0];

    for (int t = 1; t <= iterations; t++) {
        for (size_t i = 1; i < n + iterations - t; i++) {
            (*out)[i] = a * (*in)[i - 1] + b * (*in)[i] + c * (*in)[i + 1];
        }

        /* Buffer swap to save memory. */
        if (t != iterations - 1) {
            float *temp = *in;
            *in = *out;
            *out = temp;
        }
    }
}

/* Computes a stencil on inBuffer of size n + 2 * iterations, writing the result to outBuffer,
 * however only indices iterations, ..., iterations + n - 1 have the proper output. */
void middle(size_t n, int iterations, float **in, float **out)
{
    for (int t = 1; t <= iterations; t++) {
        for (size_t i = t; i < n + 2 * iterations - t; i++) {
            (*out)[i] = a * (*in)[i - 1] + b * (*in)[i] + c * (*in)[i + 1];
        }

        /* Buffer swap to save memory. */
        if (t != iterations - 1) {
            float *temp = *in;
            *in = *out;
            *out = temp;
        }
    }
}

void right(size_t n, int iterations, float **in, float **out)
{
    (*out)[n + iterations - 1] = (*in)[n + iterations - 1];

    for (int t = 1; t <= iterations; t++) {
        for (size_t i = t; i < n + iterations - 1; i++) {
            (*out)[i] = a * (*in)[i - 1] + b * (*in)[i] + c * (*in)[i + 1];
        }

        /* Buffer swap to save memory. */
        if (t != iterations - 1) {
            float *temp = *in;
            *in = *out;
            *out = temp;
        }
    }
}

/* We use iterations as blocksize. This increases the number of flops performed by a
 * factor 2. */
void StencilBlocked(size_t n, int g_in, int g_out, int iterations)
{
    float *inBuffer = malloc((BLOCK + 2 * iterations) * sizeof(float));
    float *outBuffer = malloc((BLOCK + 2 * iterations) * sizeof(float));

    int lo[1], hi[1], ld[1];
    int s = GA_Nodeid();
    int p = GA_Nnodes();
    size_t distributionBlockSize = (n / BLOCK + p - 1) / p;
    size_t start = distributionBlockSize * s;
    size_t end = MIN(distributionBlockSize * (s + 1), n / BLOCK);
    for (size_t row = start; row < end; row++) {
        if (row == 0) {
            lo[0] = 0;
            hi[0] = BLOCK + iterations - 1;
			NGA_Get(g_in, lo, hi, inBuffer, ld);
            left(BLOCK, iterations, &inBuffer, &outBuffer);
            ld[0] = BLOCK;
            NGA_Put(g_out, lo, hi, outBuffer, ld);
        } else if (row == n / BLOCK - 1) {
            lo[0] = row * BLOCK - iterations;
            hi[0] = (row + 1) * BLOCK - 1;
			NGA_Get(g_in, lo, hi, inBuffer, ld);
            right(BLOCK, iterations, &inBuffer, &outBuffer);
            ld[0] = BLOCK;
            NGA_Put(g_out, lo, hi, outBuffer + iterations, ld);
        } else {
            lo[0] = row * BLOCK - iterations;
            hi[0] = (row + 1) * BLOCK + iterations - 1;
			NGA_Get(g_in, lo, hi, inBuffer, ld);
            middle(BLOCK, iterations, &inBuffer, &outBuffer);
            ld[0] = BLOCK;
            NGA_Put(g_out, lo, hi, outBuffer + iterations, ld);
        }
    }

    GA_Sync();

    free(inBuffer);
    free(outBuffer);
}

void Stencil(size_t n, int g_in, int g_out, int iterations)
{
    for (int t = TIMEBLOCK; t <= iterations; t += TIMEBLOCK) {
        StencilBlocked(n, g_in, g_out, TIMEBLOCK);
        int temp = g_out;
        g_out = g_in;
        g_in = temp;
    }
    if (iterations % TIMEBLOCK != 0) {
        StencilBlocked(n, g_in, g_out, iterations % TIMEBLOCK);
    } else {
        /* We did one buffer swap too many */
        int temp = g_out;
        g_out = g_in;
        g_in = temp;
    }
}

int main(int argc, char **argv)
{
	int heap = 3000000;
	int stack = 3000000;

	MPI_Init(&argc, &argv);
	GA_Initialize();

	if (argc != 3) {
		GA_Error("Usage: n iterations\n", 1);
	}

	/* Initialize the global allocator */
	if (!MA_init(C_DBL, stack, heap)) {
		GA_Error("MA_init failed", 1);
	}

    size_t n = atoll(argv[1]);
	int iterations = atoi(argv[2]);

	int v_dimensions[1] = {n};
	int g_in = NGA_Create(C_FLOAT, 1, v_dimensions, "input vector", NULL);
	if (!g_in) {
		GA_Error("Could not allocate input array", 1);
	}

	int g_out = NGA_Duplicate(g_in, "output vector");
	if (!g_out) {
		GA_Error("Could not allocate output array", 1);
	}

	double one = 1.0;
	GA_Fill(g_in, &one);
	GA_Sync();

	double duration;
	TIME(duration, Stencil(n, g_in, g_out, iterations););

	if (GA_Nodeid() == 0) {
        printf("%lf\n", 5.0 * (n - 2) * iterations / 1000000000.0 / duration);
	}

	hostname_print();

	GA_Destroy(g_in);
	GA_Destroy(g_out);
	GA_Terminate();
	MPI_Finalize();

	return EXIT_SUCCESS;
}
