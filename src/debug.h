/**************************************************
 * Debugging
 **************************************************/

//-DBUG_OFF
#ifdef DEBUG
    #define DBUG_PRINT(fmt, ...)                                                        \
        fprintf(stderr, "\t[node %d]: " fmt "\n", Shray_rank, __VA_ARGS__);
#else
    #define DBUG_PRINT(fmt, ...)
#endif

/**************************************************
 * Profiling
 **************************************************/

#ifdef PROFILE
    #define BARRIERCOUNT barrierCounter++;
    #define SEGFAULTCOUNT segfaultCounter++;
    #define PREFETCHMISS prefetchMissCounter++;
#else
    #define BARRIERCOUNT
    #define SEGFAULTCOUNT
    #define PREFETCHMISS
#endif
