#include <stdio.h>
#include <stdlib.h>
/* FIXME I vaguely remember using __USE_GNU was wrong, but not why. */
#define __USE_GNU
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>

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



int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: number of pages for a large remap\n");
        exit(EXIT_FAILURE);
    }

    size_t numberOfPages = atol(argv[1]) / 2 * 2;

    int pagesz = sysconf(_SC_PAGE_SIZE);
    if (pagesz == -1) {
        perror("Querying system page size failed.");
    }

    void *buffer1;
    void *buffer2;
    MMAP_SAFE(buffer1, mmap(NULL, numberOfPages * pagesz, PROT_READ | PROT_WRITE, 
                MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));
    MMAP_SAFE(buffer2, mmap(NULL, numberOfPages * pagesz, PROT_NONE, 
                MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));

    double duration1;
    double duration2;

    /* We remap this back and forth between buffer1 and buffer2 */
    void *page = buffer1;

    TIME(duration1,
    for (size_t i = 0; i < numberOfPages / 2; i++) {
        MREMAP_SAFE(page, mremap(page, pagesz, pagesz, MREMAP_MAYMOVE | MREMAP_FIXED,
                    buffer2));
        MREMAP_SAFE(page, mremap(page, pagesz, pagesz, MREMAP_MAYMOVE | MREMAP_FIXED,
                    buffer1));
    });

    TIME(duration2, 
    for (size_t i = 0; i < numberOfPages / 2; i++) {
        MREMAP_SAFE(page, mremap(page, pagesz * numberOfPages, pagesz * numberOfPages, 
                    MREMAP_MAYMOVE | MREMAP_FIXED, buffer2));
        MREMAP_SAFE(page, mremap(page, pagesz * numberOfPages, pagesz * numberOfPages, 
                    MREMAP_MAYMOVE | MREMAP_FIXED, buffer1));
    });

    printf("Remapping one page %zu times took %lf ns. per page (%d bytes).\n", 
            numberOfPages, 1000000000.0 * duration1 / numberOfPages, pagesz);
    printf("Remapping %zu pages %zu times took %lf ns. per page (%d bytes).\n", 
            numberOfPages, numberOfPages, 
            1000000000.0 * duration2 / numberOfPages / numberOfPages, pagesz);

    return EXIT_SUCCESS;
}
