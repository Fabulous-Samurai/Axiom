#ifdef __MINGW32__
#include <process.h>
#include <stdlib.h>
#include <time.h>
#ifdef __cplusplus
extern "C" {
// Definitive workaround for missing C11 symbols in some MinGW/UCRT versions
#ifndef _AT_QUICK_EXIT_DEFINED
#define _AT_QUICK_EXIT_DEFINED
inline int at_quick_exit(void (*)(void)) { return 0; }
inline void quick_exit(int status) { _exit(status); }
#endif

#ifndef _TIMESPEC_GET_DEFINED
#define _TIMESPEC_GET_DEFINED
inline int timespec_get(struct timespec* ts, int base) {
  (void)ts;
  (void)base;
  return 0;
}
#endif
}
#endif
#endif
