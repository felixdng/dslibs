#ifndef __COMMON_H
#define __COMMON_H

#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__
#define CDECL   __cdecl
#define STDCALL __stdcall
#define PUBLIC  __declspec(dllexport) STDCALL

#else /* !__WINDOWS__ */
#define CDECL
#define STDCALL
#define PUBLIC __attribute__((visibility("default")))

#endif /* __WINDOWS__ */

#define RUN_TIME_START          \
do {                            \
	clock_t start_t, finish_t;  \
	double use_t = 0.0;         \
	printf(">>>>> run time start... <<<<<\n"); \
	start_t = clock();
#define RUN_TIME_END            \
	finish_t = clock();         \
	use_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC; \
	printf(">>>>> total run time:%.06f Sec <<<<<\n", use_t); \
} while (0);

#ifdef __cplusplus
}
#endif
#endif /* __COMMON_H */
