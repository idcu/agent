#include "idcu/os/os.h"
#include <dlfcn.h>

static const char* g_last_dlerror = NULL;

void* idcu_dlopen(const char* path)
{
    void* handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        g_last_dlerror = dlerror();
    }
    return handle;
}

void* idcu_dlsym(void* handle, const char* symbol)
{
    void* sym = dlsym(handle, symbol);
    if (!sym) {
        g_last_dlerror = dlerror();
    }
    return sym;
}

int idcu_dlclose(void* handle)
{
    return dlclose(handle);
}

const char* idcu_dlerror(void)
{
    return g_last_dlerror;
}
