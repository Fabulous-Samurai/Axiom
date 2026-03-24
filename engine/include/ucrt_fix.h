#pragma once

#ifdef __cplusplus
#include <process.h>

// Force define these in global namespace before cstdlib/ctime are included
extern "C" {
    #ifndef _AT_QUICK_EXIT_DEFINED
    #define _AT_QUICK_EXIT_DEFINED
    inline int at_quick_exit(void (*)(void)) { return 0; }
    inline void quick_exit(int status) { _exit(status); }
    #endif
    
    #ifndef _TIMESPEC_GET_DEFINED
    #define _TIMESPEC_GET_DEFINED
    #define TIME_UTC 1
    struct timespec;
    inline int timespec_get(struct timespec* ts, int base) { return 0; }
    #endif
}

#endif
