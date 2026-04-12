#include "idcu/os/os.h"
#include <windows.h>

static char g_dlerror_buf[1024];

void* idcu_dlopen(const char* path)
{
    HMODULE h = LoadLibraryA(path);
    if (!h) {
        DWORD err = GetLastError();
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, g_dlerror_buf, sizeof(g_dlerror_buf), NULL);
    }
    return h;
}

void* idcu_dlsym(void* handle, const char* symbol)
{
    return (void*)GetProcAddress((HMODULE)handle, symbol);
}

int idcu_dlclose(void* handle)
{
    return FreeLibrary((HMODULE)handle) ? 0 : -1;
}

const char* idcu_dlerror(void)
{
    return g_dlerror_buf[0] ? g_dlerror_buf : NULL;
}
