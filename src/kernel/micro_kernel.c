#include "kernel/micro_kernel.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <unistd.h>
#endif

static idcu_MicroKernel *g_kernel = NULL;

// 为避免 Windows/MinGW 链接问题，我们手动声明几个模块
// 在实际项目中，您可以使用其他方式处理模块加载
extern const idcu_ModuleInterface __idcu_module_base_log;
extern const idcu_ModuleInterface __idcu_module_biz_collect;
extern const idcu_ModuleInterface __idcu_module_core_module;

static const idcu_ModuleInterface* modules[] = {
    &__idcu_module_base_log,
    &__idcu_module_biz_collect,
    &__idcu_module_core_module,
    NULL
};

#ifdef _WIN32
static BOOL WINAPI windows_signal_handler(DWORD fdwCtrlType)
{
    if (fdwCtrlType == CTRL_C_EVENT) {
        printf("[kernel] received Ctrl+C, initiating graceful shutdown...\n");
        if (g_kernel) {
            g_kernel->should_exit = 1;
        }
        return TRUE;
    }
    return FALSE;
}
#else
static void unix_signal_handler(int sig)
{
    if (sig == SIGINT || sig == SIGTERM) {
        printf("[kernel] received signal %d, initiating graceful shutdown...\n", sig);
        if (g_kernel) {
            g_kernel->should_exit = 1;
        }
    }
}
#endif

void idcu_kernel_init(idcu_MicroKernel *k)
{
    memset(k, 0, sizeof(idcu_MicroKernel));
    idcu_msg_bus_init(&k->msg);
    idcu_ctx_init(&k->global, 0, 0);
    g_kernel = k;
}

void idcu_kernel_start_modules(idcu_MicroKernel *k)
{
    uint32_t cnt = 0;
    const idcu_ModuleInterface** mod = modules;

    while (*mod && cnt < 16) {
        printf("[kernel] module %s init\n", (*mod)->name);
        (*mod)->init();
        k->sandbox[cnt].module_id = cnt;
        k->sandbox[cnt].perm = IDCU_PERM_SEND | IDCU_PERM_RECV | IDCU_PERM_RUN;
        cnt++;
        mod++;
    }
    k->sb_cnt = cnt;
}

void idcu_kernel_stop(idcu_MicroKernel *k)
{
    printf("[kernel] stopping all modules...\n");
    
    uint32_t cnt = 0;
    const idcu_ModuleInterface** mod = modules;

    while (*mod && cnt < k->sb_cnt) {
        if ((*mod)->stop) {
            printf("[kernel] stopping module %s\n", (*mod)->name);
            (*mod)->stop();
        }
        cnt++;
        mod++;
    }
    
    printf("[kernel] all modules stopped\n");
}

void idcu_kernel_run(idcu_MicroKernel *k)
{
    while (!k->should_exit) {
        idcu_coro_sched_run(&k->coro);

        idcu_Message msg;
        for (uint32_t i = 0; i < k->sb_cnt; ++i) {
            idcu_Sandbox *sb = &k->sandbox[i];
            if (idcu_sandbox_perm_check(sb, IDCU_PERM_RECV) == 0) {
                idcu_msg_recv(&k->msg, i, &msg);
            }
        }
    }
    
    idcu_kernel_stop(k);
}

void idcu_kernel_set_signal_handler(idcu_MicroKernel *k)
{
    (void)k;
#ifdef _WIN32
    if (!SetConsoleCtrlHandler(windows_signal_handler, TRUE)) {
        printf("[kernel] warning: failed to set Windows console handler\n");
    }
#else
    struct sigaction sa;
    sa.sa_handler = unix_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGINT, &sa, NULL) != 0) {
        printf("[kernel] warning: failed to set SIGINT handler\n");
    }
    if (sigaction(SIGTERM, &sa, NULL) != 0) {
        printf("[kernel] warning: failed to set SIGTERM handler\n");
    }
#endif
}
