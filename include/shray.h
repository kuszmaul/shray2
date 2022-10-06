#ifndef SHRAY__GUARD
#define SHRAY__GUARD

#include "shrayInternal.h"

/* True for exactly one node. Useful to output results only once. */
extern bool ShrayOutput;

/* First statement in application. */
void ShrayInit(int *argc, char ***argv, size_t cacheSize);

/* Allocates memory for an array described by sizes and dimension. typeWidth is 
 * sizeof(TYPE) where TYPE is the type of the array. */
void *ShrayMalloc(size_t firstDimension, size_t totalSize);

/* Given the first dimension of an array, returns the inclusive lower bound
 * on the first dimension where we need to start computing. */
size_t ShrayStart(size_t firstDimension);

/* Given the first dimension of an array, returns the exclusive upper bound
 * on the first dimension where we need to end computing. */
size_t ShrayEnd(size_t firstDimension);

/* Makes memory available for reading, to be called after writing the result
 * of an array computation to it. */
void ShraySync(void *array);

/* Frees memory allocated by ShrayMalloc. */
void ShrayFree(void *address);

/* Prints the number of segfaults and barriers to stderr. */
void ShrayReport(void);

/* Last statement in application. */
void ShrayFinalize(void);

#define SHRAY_TIME(fncall)                                      \
    do {                                                        \
/*        gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS); \
        gasnet_barrier_wait(0, GASNET_BARRIERFLAG_ANONYMOUS);   \
        double start = MPI_Wtime();                             \
        fncall;                                                 \
        gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS); \
        gasnet_barrier_wait(0, GASNET_BARRIERFLAG_ANONYMOUS);   \
        double end = MPI_Wtime();                               \
        fprintf(stderr, #fncall " took %lfs.\n", end - start);*/  \
    } while (0)
#endif
