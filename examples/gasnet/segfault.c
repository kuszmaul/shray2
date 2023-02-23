#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>
#include <gasnet.h>
#include <gasnet_coll.h>
#include <shray2/shray.h>
#include "../util/time.h"

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
    for (size_t i = 0; i < n / p; i++) {
        arr[i] = 1;
    }
}

/* Every node gets the same address */
void *coll_malloc(size_t size)
{
    void *result;

    /* For the segfault handler, we need the start of each allocation to be
     * ShrayPagesz-aligned. We cheat a little by making it possible for this to be multiple
     * system-pages. So we mmap an extra page at the start and end, and then move the
     * pointer up. */
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

    if (gasnet_mynode() == 0) {
        double *buffer = malloc(4096);

        /* Local part */
        for (size_t i = 0; i < n / p; i++) {
            result += A[i];
        }

        /* Remote parts */
        for (int processor = 1; processor < p; processor++) {
            for (size_t page = 0; page < n / p / 512; page++) {
                gasnet_get(buffer, processor, A + page * 512, 4096);

                for (int i = 0; i < 512; i++) {
                    result += buffer[i];
                }
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

    if (argc != 2) {
        printf("Usage: lenght of the vector you reduce\n");
        gasnet_exit(1);
    }

    int p = gasnet_nodes();
    size_t n = atoll(argv[1]) / (p * 4096) * (p * 4096);
    double *A = coll_malloc(n / p * sizeof(double));
    init(A, n, p);
    gasnetBarrier();

    double duration, result;
    TIME(duration, result = reduce(A, n);)

    if (gasnet_mynode() == 0) {
        double microsPerPage = 1000000.0 * duration / (n / 512);

        fprintf(stderr, "We reduced an array on %d processors at %lf microseconds per page:\n"
                "That is a bandwidth of %lf GB/s (result is %lf)\n",
                p, microsPerPage, 4096.0 / microsPerPage / 1000.0, result);
        printf("%lf", microsPerPage);
    }

    gasnetBarrier();

    gasnet_exit(0);
}
