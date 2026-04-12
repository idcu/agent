#include "idcu/os.h"
#include <windows.h>

static char g_last_dlerror[256];

void* idcu_dlopen(const char* path)
{
    HMODULE handle = LoadLibraryA(path);
    if (!handle) {
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0,
                      g_last_dlerror, sizeof(g_last_dlerror), NULL);
    }
    return (void*)handle;
}

void* idcu_dlsym(void* handle, const char* symbol)
{
    FARPROC sym = GetProcAddress((HMODULE)handle, symbol);
    if (!sym) {
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0,
                      g_last_dlerror, sizeof(g_last_dlerror), NULL);
    }
    return (void*)sym;
}

int idcu_dlclose(void* handle)
{
    return FreeLibrary((HMODULE)handle) ? 0 : -1;
}

const char* idcu_dlerror(void)
{
    return g_last_dlerror;
}
