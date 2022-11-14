#ifndef SHRAY__GUARD
#define SHRAY__GUARD

#include <stddef.h>
#include <stdbool.h>
#include <sys/time.h>

/** <!--********************************************************************-->
 *
 * @var bool ShrayOutput
 *
 *   @brief       True for exactly one node, useful to output only once.
 *
 ******************************************************************************/

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
 * @fn void ShrayRealloc(void *array);
 *
 *   @brief Allows a distributed buffer to be reused for a different distributed
 *          array of the same size.
 *
 *   @param array   buffer we want to reallocate
 *
 ******************************************************************************/

void ShrayRealloc(void *array);

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
 * @fn size_t ShrayRank(void)
 *
 *   @brief         Returns the current Shray rank.
 *
 ******************************************************************************/

unsigned int ShrayRank(void);


/** <!--********************************************************************-->
 *
 * @fn size_t ShraySize(void)
 *
 *   @brief         Returns the number of Shray nodes.
 *
 ******************************************************************************/

unsigned int ShraySize(void);

/** <!--********************************************************************-->
 *
 * @fn size_t ShrayFinalize(int exit_code)
 *
 *   @brief         Last statement to be called in an application.
 *
 ******************************************************************************/

void ShrayFinalize(int exit_code);

/** <!--********************************************************************-->
 *
 * @fn void ShrayPrefetch(void *address, size_t size);
 *
 *   @brief Asynchronous prefetches subset of [address, address + size[.
 *
 *   @param address   Start of the memory we get (the system will round this up).
 *          size      Number of bytes we get (system will round this down).
 *
 ******************************************************************************/

void ShrayPrefetch(void *address, size_t size);

/** <!--********************************************************************-->
 *
 * @fn void ShrayDiscard(void *address, size_t size);
 *
 *   @brief Hint to free prefetched memory [address, address + size[. 
 *
 *   @param address  start of memory we free. 
 *          size     number of bytes starting from address to free.
 *
 ******************************************************************************/

void ShrayDiscard(void *address, size_t size);

/* The elipses are a function call block. Example usage:
 *
 * double duration;
 * TIME(duration, f(in, out); ShraySync(out););
 * printf("Executing f took %lf seconds.\n", duration);
 *
 * */
#define TIME(duration, fncalls)                                        \
    {                                                                  \
        struct timeval tv1, tv2;                                       \
        gettimeofday(&tv1, NULL);                                      \
        fncalls                                                        \
        gettimeofday(&tv2, NULL);                                      \
        duration = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +    \
         (double) (tv2.tv_sec - tv1.tv_sec);                           \
    }

#endif
