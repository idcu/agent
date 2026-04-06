#ifndef DEMO_H
#define DEMO_H

#ifdef _WIN32
    #ifdef DEMO_SHARED
        #define DEMO_API __declspec(dllexport)
    #else
        #define DEMO_API __declspec(dllimport)
    #endif
#else
    #define DEMO_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

DEMO_API int demo_add(int a, int b);
DEMO_API int demo_multiply(int a, int b);
DEMO_API const char* demo_get_version(void);

#ifdef __cplusplus
}
#endif

#endif
