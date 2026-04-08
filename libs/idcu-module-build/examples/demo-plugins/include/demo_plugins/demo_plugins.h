#ifndef DEMO_PLUGINS_H
#define DEMO_PLUGINS_H

#ifdef _WIN32
#ifdef DEMO_PLUGINS_SHARED
#define DEMO_PLUGINS_API __declspec(dllexport)
#else
#define DEMO_PLUGINS_API __declspec(dllimport)
#endif
#else
#define DEMO_PLUGINS_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    DEMO_PLUGINS_API int         demo_plugins_add(int a, int b);
    DEMO_PLUGINS_API int         demo_plugins_multiply(int a, int b);
    DEMO_PLUGINS_API const char* demo_plugins_get_version(void);

#ifdef __cplusplus
}
#endif

#endif
