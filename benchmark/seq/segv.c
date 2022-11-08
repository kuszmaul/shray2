#include <stdio.h>
#include <stdlib.h>
/* FIXME I vaguely remember using __USE_GNU was wrong, but not why. */
#define __USE_GNU
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>

size_t counter = 0;

#define TIME(duration, fncalls)                                        \
    {                                                                  \
        struct timeval tv1, tv2;                                       \
        gettimeofday(&tv1, NULL);                                      \
        fncalls                                                        \
        gettimeofday(&tv2, NULL);                                      \
        duration = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +    \
         (double) (tv2.tv_sec - tv1.tv_sec);                           \
    }

#define MREMAP_SAFE(variable, fncall)                                                   \
    {                                                                                   \
        variable = fncall;                                                              \
        if (variable == MAP_FAILED) {                                                   \
            fprintf(stderr, "Line %d: ", __LINE__);                                     \
            perror("mremap failed");                                                    \
            exit(EXIT_FAILURE);                                                         \
        }                                                                               \
    }

#define MMAP_SAFE(variable, fncall)                                                     \
    {                                                                                   \
        variable = fncall;                                                              \
        if (variable == MAP_FAILED) {                                                   \
            fprintf(stderr, "Line %d: ", __LINE__);                                     \
            perror("mmap failed");                                                      \
            exit(EXIT_FAILURE);                                                         \
        }                                                                               \
    }

#define MPROTECT_SAFE(addr, len, prot)                                                  \
    {                                                                                   \
        if (mprotect(addr, len, prot) != 0) {                                           \
            fprintf(stderr, "Line %d: ", __LINE__);                                     \
            perror("mprotect failed");                                                  \
            exit(EXIT_FAILURE);                                                         \
        }                                                                               \
    }

void SegvHandler(int sig, siginfo_t *si, void *unused)
{
    void *address = si->si_addr;
    void *roundedAddress = (void *)((uintptr_t)address / 4096 * 4096);

    MPROTECT_SAFE(address, 4096, PROT_READ | PROT_WRITE);

    counter++;
    return;
}

void registerHandler(void)
{
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset (&sa.sa_mask);
    sa.sa_sigaction = SegvHandler;

    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("Registering SIGSEGV handler failed.\n");
    }
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: number of pages for a large remap\n");
        exit(EXIT_FAILURE);
    }

    size_t numberOfPages = atol(argv[1]) / 2 * 2;

    registerHandler();

    int pagesz = sysconf(_SC_PAGE_SIZE);
    if (pagesz == -1) {
        perror("Querying system page size failed.");
    }

    double *buffer;
    MMAP_SAFE(buffer, mmap(NULL, numberOfPages * pagesz, PROT_NONE,
                MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));

    double duration;

    TIME(duration,
    for (size_t i = 0; i < numberOfPages * pagesz / sizeof(double); i += 4096) {
        buffer[i] *= 2;
    });

    printf("Handling a segfault takes %lf ns\n", duration * 1000000000.0 / counter);

    return EXIT_SUCCESS;
}
