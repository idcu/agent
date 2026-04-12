#include "idcu/os/os.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#if IDCU_OS_HARMONY

static char g_last_error[256] = {0};

void* idcu_dlopen(const char* path)
{
    if (!path) {
        strncpy(g_last_error, "Path is NULL", sizeof(g_last_error) - 1);
        return NULL;
    }
    
    void* handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    
    if (!handle) {
        const char* err = dlerror();
        if (err) {
            strncpy(g_last_error, err, sizeof(g_last_error) - 1);
        } else {
            strncpy(g_last_error, "Unknown dlopen error", sizeof(g_last_error) - 1);
        }
        
        char private_path[1024];
        const char* data_dir = getenv("DATA_DIR");
        if (data_dir) {
            snprintf(private_path, sizeof(private_path), "%s/%s", data_dir, path);
            handle = dlopen(private_path, RTLD_NOW | RTLD_LOCAL);
            if (handle) {
                g_last_error[0] = '\0';
                return handle;
            }
        }
    }
    
    if (handle) {
        g_last_error[0] = '\0';
    }
    
    return handle;
}

void* idcu_dlsym(void* handle, const char* symbol)
{
    if (!handle || !symbol) {
        strncpy(g_last_error, "Invalid arguments", sizeof(g_last_error) - 1);
        return NULL;
    }
    
    void* sym = dlsym(handle, symbol);
    
    if (!sym) {
        const char* err = dlerror();
        if (err) {
            strncpy(g_last_error, err, sizeof(g_last_error) - 1);
        } else {
            strncpy(g_last_error, "Unknown dlsym error", sizeof(g_last_error) - 1);
        }
    } else {
        g_last_error[0] = '\0';
    }
    
    return sym;
}

int idcu_dlclose(void* handle)
{
    if (!handle) {
        strncpy(g_last_error, "Handle is NULL", sizeof(g_last_error) - 1);
        return -1;
    }
    
    int result = dlclose(handle);
    
    if (result != 0) {
        const char* err = dlerror();
        if (err) {
            strncpy(g_last_error, err, sizeof(g_last_error) - 1);
        } else {
            strncpy(g_last_error, "Unknown dlclose error", sizeof(g_last_error) - 1);
        }
    } else {
        g_last_error[0] = '\0';
    }
    
    return result;
}

const char* idcu_dlerror(void)
{
    if (g_last_error[0] != '\0') {
        return g_last_error;
    }
    
    const char* err = dlerror();
    if (err) {
        strncpy(g_last_error, err, sizeof(g_last_error) - 1);
        return g_last_error;
    }
    
    return NULL;
}

#endif
