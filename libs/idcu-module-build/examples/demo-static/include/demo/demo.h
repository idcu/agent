#ifndef DEMO_H
#define DEMO_H

#define DEMO_API

#ifdef __cplusplus
extern "C"
{
#endif

    DEMO_API int         demo_subtract(int a, int b);
    DEMO_API int         demo_divide(int a, int b);
    DEMO_API const char* demo_get_version(void);

#ifdef __cplusplus
}
#endif

#endif
