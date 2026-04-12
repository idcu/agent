#include "idcu/os/os.h"
#include <signal.h>
#include <string.h>

static idcu_signal_handler_t g_handlers[NSIG];

static void signal_wrapper(int signum)
{
    if (signum >= 0 && signum < NSIG && g_handlers[signum]) {
        g_handlers[signum](signum);
    }
}

int idcu_signal_register(int signum, idcu_signal_handler_t handler)
{
    if (signum < 0 || signum >= NSIG) {
        return -1;
    }
    
    g_handlers[signum] = handler;
    
    if (handler) {
        signal(signum, signal_wrapper);
    } else {
        signal(signum, SIG_DFL);
    }
    
    return 0;
}
