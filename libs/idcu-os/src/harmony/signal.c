#include "idcu/os/os.h"
#include <signal.h>
#include <string.h>
#include <errno.h>

#if IDCU_OS_HARMONY

static idcu_signal_handler_t g_signal_handlers[NSIG];

static int harmony_signal_map(int signum)
{
    static const struct {
        int std_signal;
        int harmony_signal;
    } signal_map[] = {
        {SIGINT, 46},
        {SIGTERM, 47},
        {SIGUSR1, 48},
        {SIGUSR2, 49},
        {SIGALRM, 50},
        {SIGCHLD, 51},
        {SIGPIPE, 52},
        {SIGHUP, 53},
        {SIGQUIT, 54},
        {SIGILL, 55},
        {SIGFPE, 56},
        {SIGSEGV, 57},
        {SIGABRT, 58},
        {0, 0}
    };
    
    for (int i = 0; signal_map[i].std_signal != 0; i++) {
        if (signal_map[i].std_signal == signum) {
            return signal_map[i].harmony_signal;
        }
    }
    
    return signum;
}

static void harmony_signal_trampoline(int signum)
{
    for (int i = 0; i < NSIG; i++) {
        if (g_signal_handlers[i]) {
            int mapped = harmony_signal_map(i);
            if (mapped == signum) {
                g_signal_handlers[i](i);
                return;
            }
        }
    }
}

int idcu_signal_register(int signum, idcu_signal_handler_t handler)
{
    if (signum < 0 || signum >= NSIG) {
        return -EINVAL;
    }
    
    int harmony_signum = harmony_signal_map(signum);
    
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    
    if (handler) {
        g_signal_handlers[signum] = handler;
        sa.sa_handler = harmony_signal_trampoline;
    } else {
        g_signal_handlers[signum] = NULL;
        sa.sa_handler = SIG_DFL;
    }
    
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(harmony_signum, &sa, NULL) == -1) {
        return -errno;
    }
    
    return 0;
}

#endif
