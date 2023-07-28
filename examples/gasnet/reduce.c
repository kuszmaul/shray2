#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>
#include <gasnet.h>
#include <gasnet_coll.h>
#include <math.h>
#include "../util/time.h"

size_t pagesz;

#define GASNET_SAFE(fncall)                                                              \
    {                                                                                    \
        int retval;                                                                      \
        if ((retval = fncall) != GASNET_OK) {                                            \
            printf("Error during GASNet call\n");                                        \
            gasnet_exit(1);                                                              \
        }                                                                                \
    }

#define MMAP_SAFE(variable, address, length, prot)                                      \
    {                                                                                   \
        variable = mmap(address, length, prot, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);     \
        if (variable == MAP_FAILED) {                                                   \
            fprintf(stderr, "mmap failed");                                             \
            gasnet_exit(1);                                                             \
        }                                                                               \
    }

#define MMAP_FIXED_SAFE(address, length, prot)                                          \
    {                                                                                   \
        void *success = mmap(address, length, prot,                                     \
                MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);                        \
        if (success == MAP_FAILED) {                                                    \
            printf("mmap failed");                                                      \
            gasnet_exit(1);                                                             \
        }                                                                               \
    }

void gasnetBarrier()
{
    gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0, GASNET_BARRIERFLAG_ANONYMOUS);
}

void init(double *arr, size_t n, int p)
{
    for (size_t i = 0; i < (n + p - 1) / p; i++) {
        arr[i] = 1;
    }
}

/* Every node gets the same address */
void *coll_malloc(size_t size)
{
    void *result;

    /* For the segfault handler, we need the start of each allocation to be
     * ShrayPagesz-aligned. We cheat a little by making it possible for this to
     * be multiple system-pages. So we mmap an extra page at the start and end,
     * and then move the pointer up. */
    if (gasnet_mynode() == 0) {
        MMAP_SAFE(result, NULL, size, PROT_READ | PROT_WRITE);
    }

    gasnet_coll_broadcast(gasnete_coll_team_all, &result, 0, &result,
            sizeof(void *), GASNET_COLL_DST_IN_SEGMENT);

    if (gasnet_mynode() != 0) {
        MMAP_FIXED_SAFE(result, size, PROT_READ | PROT_WRITE);
    }

    return result;
}

double reduce(double *A, size_t n)
{
    double result = 0.0;
    int p = gasnet_nodes();
    size_t double_per_page = pagesz / sizeof(double);
    size_t blocksize = (n + p - 1) / p;

    if (gasnet_mynode() == 0) {
        double *buffer = malloc(pagesz);

        /* Remote parts */
        for (size_t j = blocksize; j < n; j += double_per_page) {
            int processor = j / blocksize;
            gasnet_get(buffer, processor, A + j, pagesz);
            for (size_t i = 0; i < double_per_page; i++) {
                result += buffer[i];
            }
        }

        free(buffer);
    }

    return result;
}

int main(int argc, char **argv)
{
    GASNET_SAFE(gasnet_init(&argc, &argv));

    GASNET_SAFE(gasnet_attach(NULL, 0, 4096, 0));

    if (argc != 4) {
        printf("Usage: n, NITER, pages per get\n");
        gasnet_exit(1);
    }

    int p = gasnet_nodes();
    size_t n = atol(argv[1]);
    int niter = atoi(argv[2]);
    pagesz = 4096 * atoi(argv[3]);
    n = n / (p * pagesz) * (p * pagesz);

    /* Wellfords algorithm may be overkill, but numerical stability is
       nice. */
    int count = 0;
    double mean = 0;
    double m2 = 0;

    double *A = coll_malloc(n * sizeof(double));
    for (int t = 0; t < niter; t++) {
        init(A, n, p);
        gasnetBarrier();

        double duration, result;
        double bandwidth = 0;

        if (gasnet_mynode() == 0) {
            TIME(duration, result = reduce(A, n);)
            bandwidth = 8.0 * (n - (n + p - 1) / p) / 1e6 / duration;
            fprintf(stderr, "Sum is %lf\n", result);
        }
        count++;
        double delta_pre = bandwidth - mean;
        mean += delta_pre / count;
        double delta_post = bandwidth - mean;
        m2 += delta_pre * delta_post;

    }

    if (gasnet_mynode() == 0) {
        printf(" & %lf & %lf & ", mean, sqrt(m2 / count));
    }

    gasnetBarrier();

    gasnet_exit(0);
}
