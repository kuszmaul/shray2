#ifndef SHRAY__GUARD
#define SHRAY__GUARD

#include <stddef.h>
#include <stdbool.h>
#include <sys/time.h>

typedef struct {
    void *args;
    size_t start;
    size_t end;
} worker_info_t;

extern bool ShrayOutput;
typedef void (*shray_fn)(worker_info_t *info);

#ifdef DEBUG

void ShrayInit_debug(int *argc, char ***argv);
void *ShrayMalloc_debug(size_t firstDimension, size_t totalSize);
size_t ShrayStart_debug(size_t firstDimension);
size_t ShrayEnd_debug(size_t firstDimension);
void ShraySync_debug(void *array);
void ShrayFree_debug(void *address);
void ShrayReport_debug(void);
unsigned int ShrayRank_debug(void);
unsigned int ShraySize_debug(void);
void ShrayFinalize_debug(int exit_code);
void ShrayPrefetch_debug(void *address, size_t size);
void ShrayDiscard_debug(void *address, size_t size);
void ShrayRunWorker_debug(shray_fn fn, size_t n, void *args);
void ShrayBroadcast_debug(void *buffer, size_t size, int root);

#define ShrayInit(argc, argv) ShrayInit_debug(argc, argv)
#define ShrayMalloc(firstDimension, totalSize) ShrayMalloc_debug(firstDimension, totalSize)
#define ShrayStart(firstDimension) ShrayStart_debug(firstDimension)
#define ShrayEnd(firstDimension) ShrayEnd_debug(firstDimension)
#define ShraySync(array) ShraySync_debug(array)
#define ShrayFree(address) ShrayFree_debug(address)
#define ShrayReport() ShrayReport_debug()
#define ShrayRank() ShrayRank_debug()
#define ShraySize() ShraySize_debug()
#define ShrayFinalize(exit_code) ShrayFinalize_debug(exit_code)
#define ShrayPrefetch(address, size) ShrayPrefetch_debug(address, size)
#define ShrayDiscard(address, size) ShrayDiscard_debug(address, size)
#define ShrayRunWorker(fn, n, args) ShrayRunWorker_debug(fn, n, args)
#define ShrayBroadcast(buffer, size, root) ShrayBroadcast_debug(buffer, size, root)

#else

#ifdef PROFILE

void ShrayInit_profile(int *argc, char ***argv);
void *ShrayMalloc_profile(size_t firstDimension, size_t totalSize);
size_t ShrayStart_profile(size_t firstDimension);
size_t ShrayEnd_profile(size_t firstDimension);
void ShraySync_profile(void *array);
void ShrayFree_profile(void *address);
void ShrayReport_profile(void);
unsigned int ShrayRank_profile(void);
unsigned int ShraySize_profile(void);
void ShrayFinalize_profile(int exit_code);
void ShrayPrefetch_profile(void *address, size_t size);
void ShrayDiscard_profile(void *address, size_t size);
void ShrayRunWorker_profile(shray_fn fn, size_t n, void *args);
void ShrayBroadcast_profile(void *buffer, size_t size, int root);

#define ShrayInit(argc, argv) ShrayInit_profile(argc, argv)
#define ShrayMalloc(firstDimension, totalSize) ShrayMalloc_profile(firstDimension, totalSize)
#define ShrayStart(firstDimension) ShrayStart_profile(firstDimension)
#define ShrayEnd(firstDimension) ShrayEnd_profile(firstDimension)
#define ShraySync(array) ShraySync_profile(array)
#define ShrayFree(address) ShrayFree_profile(address)
#define ShrayReport() ShrayReport_profile()
#define ShrayRank() ShrayRank_profile()
#define ShraySize() ShraySize_profile()
#define ShrayFinalize(exit_code) ShrayFinalize_profile(exit_code)
#define ShrayPrefetch(address, size) ShrayPrefetch_profile(address, size)
#define ShrayDiscard(address, size) ShrayDiscard_profile(address, size)
#define ShrayRunWorker(fn, n, args) ShrayRunWorker_profile(fn, n, args)
#define ShrayBroadcast(buffer, size, root) ShrayBroadcast_profile(buffer, size, root)

#else
void ShrayInit_normal(int *argc, char ***argv);
void *ShrayMalloc_normal(size_t firstDimension, size_t totalSize);
size_t ShrayStart_normal(size_t firstDimension);
size_t ShrayEnd_normal(size_t firstDimension);
void ShraySync_normal(void *array);
void ShrayFree_normal(void *address);
void ShrayReport_normal(void);
unsigned int ShrayRank_normal(void);
unsigned int ShraySize_normal(void);
void ShrayFinalize_normal(int exit_code);
void ShrayPrefetch_normal(void *address, size_t size);
void ShrayDiscard_normal(void *address, size_t size);
void ShrayRunWorker_normal(shray_fn fn, size_t n, void *args);
void ShrayBroadcast_normal(void *buffer, size_t size, int root);

#define ShrayInit(argc, argv) ShrayInit_normal(argc, argv)
#define ShrayMalloc(firstDimension, totalSize) ShrayMalloc_normal(firstDimension, totalSize)
#define ShrayStart(firstDimension) ShrayStart_normal(firstDimension)
#define ShrayEnd(firstDimension) ShrayEnd_normal(firstDimension)
#define ShraySync(array) ShraySync_normal(array)
#define ShrayFree(address) ShrayFree_normal(address)
#define ShrayReport() ShrayReport_normal()
#define ShrayRank() ShrayRank_normal()
#define ShraySize() ShraySize_normal()
#define ShrayFinalize(exit_code) ShrayFinalize_normal(exit_code)
#define ShrayPrefetch(address, size) ShrayPrefetch_normal(address, size)
#define ShrayDiscard(address, size) ShrayDiscard_normal(address, size)
#define ShrayRunWorker(fn, n, args) ShrayRunWorker_normal(fn, n, args)
#define ShrayBroadcast(buffer, size, root) ShrayBroadcast_normal(buffer, size, root)
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
 * @fn void ShrayPrefetch(void *address, size_t size);
 *
 *   @brief Asynchronous prefetches subset of [address, address + size[.
 *
 *   @param address   Start of the memory we get (the system will round this up).
 *          size      Number of bytes we get (system will round this down).
 *
 ******************************************************************************/

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

/** <!--********************************************************************-->
 *
 * @fn void ShrayRunWorker(shray_fn fn, size_t n, void *args);
 *
 *   @brief          Executes fn on args with args->start = ShrayStart(n),
 *                   args->end = ShrayEnd(n) using multithreading.
 *
 *   @param fn       Function that writes to distributed array A only on
 *                   indices (i1, ..., id) satisfying
 *                   workerinfo_t->start <= i1 < worker_info_t->end.
 *          n        First dimension of A.
 *          args     Arguments of fn.
 *
 ******************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn void ShrayBroadcast(void *address, size_t size, int root);
 *
 *   @brief Lets node 'root' broadcast information to all other nodes,
 *          mainly useful if root gets some input that other nodes need.
 *
 *   @param buffer
 *          size
 *          root     Node 'root' communicates [buffer, buffer + size[ to
 *                   buffer on all other nodes.
 *
 ******************************************************************************/

/* Example
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
