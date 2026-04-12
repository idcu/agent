#include "idcu/os/os.h"
#include <dlfcn.h>

void* idcu_dlopen(const char* path)
{
    return dlopen(path, RTLD_NOW | RTLD_LOCAL);
}

void* idcu_dlsym(void* handle, const char* symbol)
{
    return dlsym(handle, symbol);
}

int idcu_dlclose(void* handle)
{
    return dlclose(handle) == 0 ? 0 : -1;
}

const char* idcu_dlerror(void)
{
    return dlerror();
}
