#include "idcu/os/os.h"
#include <rtthread.h>

int idcu_signal_register(int sig, idcu_signal_handler_t handler)
{
    return 0;
}

int idcu_signal_raise(int sig)
{
    return 0;
}

int idcu_signal_block(int sig)
{
    return 0;
}

int idcu_signal_unblock(int sig)
{
    return 0;
}
