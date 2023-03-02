#ifndef HOST__GUARD
#define HOST__GUARD

#ifdef __cplusplus
extern "C" {
#endif

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

/**
 * Print host name.
 */
void hostname_print(void);

#ifdef __cplusplus
}
#endif

#endif
