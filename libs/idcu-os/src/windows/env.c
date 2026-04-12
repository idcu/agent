#include "idcu/os.h"
#include <windows.h>
#include <stdlib.h>

static char g_env_buf[32768];

char* idcu_env_get(const char* name)
{
    if (GetEnvironmentVariableA(name, g_env_buf, sizeof(g_env_buf)) == 0) {
        return NULL;
    }
    return g_env_buf;
}

int idcu_env_set(const char* name, const char* value)
{
    return SetEnvironmentVariableA(name, value) ? 0 : -1;
}

int idcu_env_del(const char* name)
{
    return SetEnvironmentVariableA(name, NULL) ? 0 : -1;
}
