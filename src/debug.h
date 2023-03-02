/**************************************************
 * Debugging
 **************************************************/

//-DBUG_OFF
#ifdef DEBUG
    #define DBUG_PRINT(fmt, ...)                                                        \
        fprintf(stderr, "\t[node %d] (%s): " fmt "\n", Shray_rank, __func__, __VA_ARGS__);
#else
    #define DBUG_PRINT(fmt, ...)
#endif

/**************************************************
 * Profiling
 **************************************************/

#ifdef PROFILE
    #define BARRIERCOUNT ShrayBarrierCounter++;
    #define SEGFAULTCOUNT ShraySegfaultCounter++;
    #define PREFETCHMISS ShrayPrefetchMissCounter++;
#else
    #define BARRIERCOUNT
    #define SEGFAULTCOUNT
    #define PREFETCHMISS
#endif
