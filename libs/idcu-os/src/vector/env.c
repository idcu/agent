#include "idcu/os/os.h"
#include <stdlib.h>
#include <string.h>

char* idcu_env_get(const char* name)
{
    return getenv(name);
}

int idcu_env_set(const char* name, const char* value)
{
    return setenv(name, value, 1) == 0 ? 0 : -1;
}

int idcu_env_del(const char* name)
{
    return unsetenv(name) == 0 ? 0 : -1;
}
