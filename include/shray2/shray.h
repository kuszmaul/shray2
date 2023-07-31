#ifndef SHRAY__GUARD
#define SHRAY__GUARD

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include <sys/time.h>

extern bool ShrayOutput;

/* Debug declarations */
void ShrayInit_debug(int *argc, char ***argv);
void *ShrayMalloc_debug(size_t firstDimension, size_t totalSize);
__attribute__((pure)) size_t ShrayStart_debug(void *array);
__attribute__((pure)) size_t ShrayEnd_debug(void *array);
void ShraySync_debug(void *unused, ...);
void ShrayFree_debug(void *address);
void ShrayReport_debug(void);
__attribute__((pure)) unsigned int ShrayRank_debug(void);
__attribute__((pure)) unsigned int ShraySize_debug(void);
void ShrayFinalize_debug(int exit_code) __attribute__ ((noreturn));
void * ShrayWriteBuf_debug(void *address, size_t size);
void ShrayCommit_debug(void * buf, void *address, size_t size);
void ShrayUncommit_debug(void *address, size_t size);

/* Profile declarations */
void ShrayInit_profile(int *argc, char ***argv);
void *ShrayMalloc_profile(size_t firstDimension, size_t totalSize);
__attribute__((pure)) size_t ShrayStart_profile(void *array);
__attribute__((pure)) size_t ShrayEnd_profile(void *array);
void ShraySync_profile(void *unused, ...);
void ShrayFree_profile(void *address);
void ShrayReport_profile(void);
__attribute__((pure)) unsigned int ShrayRank_profile(void);
__attribute__((pure)) unsigned int ShraySize_profile(void);
void ShrayFinalize_profile(int exit_code) __attribute__ ((noreturn));
void * ShrayWriteBuf_profile(void *address, size_t size);
void ShrayCommit_profile(void * buf, void *address, size_t size);
void ShrayUncommit_profile(void *address, size_t size);

/* Normal declarations */
void ShrayInit_normal(int *argc, char ***argv);
void *ShrayMalloc_normal(size_t firstDimension, size_t totalSize);
__attribute__((pure)) size_t ShrayStart_normal(void *array);
__attribute__((pure)) size_t ShrayEnd_normal(void *array);
void ShraySync_normal(void *unused, ...);
void ShrayFree_normal(void *address);
void ShrayReport_normal(void);
__attribute__((pure)) unsigned int ShrayRank_normal(void);
__attribute__((pure)) unsigned int ShraySize_normal(void);
void ShrayFinalize_normal(int exit_code) __attribute__ ((noreturn));
void * ShrayWriteBuf_normal(void *address, size_t size);
void ShrayCommit_normal(void * buf, void *address, size_t size);
void ShrayUncommit_normal(void *address, size_t size);

#ifdef DEBUG

#define ShrayInit(argc, argv) ShrayInit_debug(argc, argv)
#define ShrayMalloc(firstDimension, totalSize) ShrayMalloc_debug(firstDimension, totalSize)
#define ShrayStart(array) ShrayStart_debug(array)
#define ShrayEnd(array) ShrayEnd_debug(array)
#define ShraySync(...) ShraySync_debug(NULL, __VA_ARGS__, NULL)
#define ShrayFree(address) ShrayFree_debug(address)
#define ShrayReport() ShrayReport_debug()
#define ShrayRank() ShrayRank_debug()
#define ShraySize() ShraySize_debug()
#define ShrayFinalize(exit_code) ShrayFinalize_debug(exit_code)

#else
#ifdef PROFILE

#define ShrayInit(argc, argv) ShrayInit_profile(argc, argv)
#define ShrayMalloc(firstDimension, totalSize) ShrayMalloc_profile(firstDimension, totalSize)
#define ShrayStart(array) ShrayStart_profile(array)
#define ShrayEnd(array) ShrayEnd_profile(array)
#define ShraySync(...) ShraySync_profile(NULL, __VA_ARGS__, NULL)
#define ShrayFree(address) ShrayFree_profile(address)
#define ShrayReport() ShrayReport_profile()
#define ShrayRank() ShrayRank_profile()
#define ShraySize() ShraySize_profile()
#define ShrayFinalize(exit_code) ShrayFinalize_profile(exit_code)

#else
#define ShrayInit(argc, argv) ShrayInit_normal(argc, argv)
#define ShrayMalloc(firstDimension, totalSize) ShrayMalloc_normal(firstDimension, totalSize)
#define ShrayStart(array) ShrayStart_normal(array)
#define ShrayEnd(array) ShrayEnd_normal(array)
#define ShraySync(...) ShraySync_normal(NULL, __VA_ARGS__, NULL)
#define ShrayFree(address) ShrayFree_normal(address)
#define ShrayReport() ShrayReport_normal()
#define ShrayRank() ShrayRank_normal()
#define ShraySize() ShraySize_normal()
#define ShrayFinalize(exit_code) ShrayFinalize_normal(exit_code)
#endif /* PROFILE */
#endif /* DEBUG */

/** <!--********************************************************************-->
 *
 * @var bool ShrayOutput
 *
 *   @brief       True for exactly one node, useful to output only once.
 *
 ******************************************************************************/

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

/** <!--********************************************************************-->
 *
 * @fn size_t ShrayStart(void *array)
 *
 *   @brief       Determines the start of the first dimension of a distributed
 *                array we write to.
 *
 *   @param array Array to compute the start index for.
 *
 *   @return start such that we are allowed to write to (i1, ..., id) for
 *           start <= i1 < end.
 *
 ******************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn size_t ShrayEnd(void *array)
 *
 *   @brief       Determines the end of the first dimension of a distributed
 *                array we write to.
 *
 *   @param array Array to compute the end index for.
 *
 *   @return end such that we are allowed to write to (i1, ..., id) for
 *           start <= i1 < end.
 *
 ******************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn void ShraySync(void *array, ...)
 *
 *   @brief         Makes distributed array(s) available for reading, to be called
 *                  after writing is finished.
 *
 *   @param array   Array we have finished writing to.
 *
 ******************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn void ShrayFree(void *array)
 *
 *   @brief         Frees distributed array.
 *
 *   @param array   Array to be freed.
 *
 ******************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn void ShrayReport(void)
 *
 *   @brief         Prints profiling information, only outputs non-garbage when
 *                  PROFILE is defined.
 *
 ******************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn size_t ShrayRank(void)
 *
 *   @brief         Returns the current Shray rank.
 *
 ******************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn size_t ShraySize(void)
 *
 *   @brief         Returns the number of Shray nodes.
 *
 ******************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn size_t ShrayFinalize(int exit_code)
 *
 *   @brief         Last statement to be called in an application.
 *
 ******************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn void * ShrayWriteBuf(void *address, size_t size)
 *
 *   @brief         If you want to do gasnet communication on page-aligned
 *                  [address, address + size[ not a subset of the partition you
 *                  own, you can do it on the buffer returned by this call.
 *                  When done, commit your result with a call to
 *                  ShrayCommit(buf, address, size). The memory can be
 *                  freed up after ShrayCommit by ShrayUncommit
 *
 *   @return buf    Buffer you can write to
 *
 ******************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn void ShrayCommit(void * buf, void *address, size_t size)
 *
 *   @brief         See ShrayWriteBuf
 *
 ******************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn void ShrayUncommit(void *address, size_t size)
 *
 *   @brief         See ShrayWriteBuf
 *
 ******************************************************************************/

#endif /* SHRAY__GUARD */
