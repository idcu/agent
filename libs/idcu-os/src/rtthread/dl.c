#include "idcu/os/os.h"
#include <rtthread.h>

void* idcu_dl_open(const char* filename, int flags)
{
    return NULL;
}

void* idcu_dl_sym(void* handle, const char* symbol)
{
    return NULL;
}

int idcu_dl_close(void* handle)
{
    return 0;
}

const char* idcu_dl_error(void)
{
    return "Dynamic loading not supported on RT-Thread";
}
