#ifndef SHRAY__GUARD
#define SHRAY__GUARD

#include "shrayInternal.h"

/* True for exactly one node. Useful to output results only once. */
extern bool ShrayOutput;

/** <!--********************************************************************-->
 *
 * @fn void ShrayInit(int *argc, char ***argv)
 *
 *   @brief       First statement in application.
 *
 *   @param argc  Pointer to the number of command line arguments.
 *   @param argv  Pointer to the command line arguments.
 *
 ******************************************************************************/

void ShrayInit(int *argc, char ***argv);

/** <!--********************************************************************-->
 *
 * @fn void *ShrayMalloc(size_t firstDimension, size_t totalSize);
 *
 *   @brief       Allocates a distributed (multidimensional) array.
 *
 *   @param firstDimension Extent of the first dimension of the allocated array.
 *   @param totalSize Total size of the array in bytes.
 *
 *   @return Pointer to the allocation.
 *
 ******************************************************************************/

void *ShrayMalloc(size_t firstDimension, size_t totalSize);

/** <!--********************************************************************-->
 *
 * @fn size_t ShrayStart(size_t firstDimension)
 *
 *   @brief       Determines the start of the first dimension of a distributed 
 *                array we write to.
 *
 *   @param firstDimension Extent of the first dimension of the array.
 *
 *   @return start such that we are allowed to write to (i1, ..., id) for 
 *           start <= i1 < end.
 *
 ******************************************************************************/

size_t ShrayStart(size_t firstDimension);

/** <!--********************************************************************-->
 *
 * @fn size_t ShrayEnd(size_t firstDimension)
 *
 *   @brief       Determines the end of the first dimension of a distributed 
 *                array we write to.
 *
 *   @param firstDimension Extent of the first dimension of the array.
 *
 *   @return end such that we are allowed to write to (i1, ..., id) for 
 *           start <= i1 < end.
 *
 ******************************************************************************/

size_t ShrayEnd(size_t firstDimension);

/** <!--********************************************************************-->
 *
 * @fn void ShraySync(void *array)
 *
 *   @brief         Makes distributed array available for reading, to be called
 *                  after writing is finished.
 *
 *   @param array   Array we have finished writing to.
 *
 ******************************************************************************/

void ShraySync(void *array);

/** <!--********************************************************************-->
 *
 * @fn void ShrayFree(void *array)
 *
 *   @brief         Frees distributed array.
 *
 *   @param array   Array to be freed.
 *
 ******************************************************************************/

void ShrayFree(void *address);

/** <!--********************************************************************-->
 *
 * @fn void ShrayReport(void)
 *
 *   @brief         Prints profiling information, only outputs non-garbage when
 *                  PROFILE is defined.
 *
 ******************************************************************************/

void ShrayReport(void);

/** <!--********************************************************************-->
 *
 * @fn void ShrayReport(void)
 *
 *   @brief         Last statement to be called in an application.
 *
 ******************************************************************************/

void ShrayFinalize(int exit_code);

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
