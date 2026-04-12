#include "idcu/os/os.h"
#include <rtthread.h>
#include <string.h>

char* idcu_env_get(const char* name)
{
    return rt_getenv(name);
}

int idcu_env_set(const char* name, const char* value)
{
    return rt_setenv(name, value, 1) == 0 ? 0 : -1;
}

int idcu_env_del(const char* name)
{
    rt_unsetenv(name);
    return 0;
}
