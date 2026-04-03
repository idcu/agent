#include "kernel/micro_kernel.h"
#include "module/module_registry.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <unistd.h>
#endif

static idcu_MicroKernel *g_kernel = NULL;
static idcu_ModuleRegistry g_module_registry;

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
    idcu_module_registry_init(&g_module_registry);
    idcu_module_registry_discover_modules(&g_module_registry);
    g_kernel = k;
}

void idcu_kernel_start_modules(idcu_MicroKernel *k)
{
    uint32_t cnt = 0;
    int module_count = idcu_module_registry_get_count(&g_module_registry);

    for (int i = 0; i < module_count && cnt < 16; i++) {
        const idcu_RegisteredModule* reg_mod = idcu_module_registry_get_at(&g_module_registry, i);
        if (!reg_mod) {
            continue;
        }
        
        const idcu_ModuleInterface* mod = reg_mod->iface;
        printf("[kernel] module %s init\n", mod->name);
        if (mod->init) {
            int ret = mod->init();
            if (ret != 0) {
                printf("[kernel] warning: module %s init failed with code %d\n", mod->name, ret);
            }
        }
        
        k->sandbox[cnt].module_id = cnt;
        k->sandbox[cnt].perm = IDCU_PERM_SEND | IDCU_PERM_RECV | IDCU_PERM_RUN;
        
        k->tracked_modules[cnt].iface = mod;
        k->tracked_modules[cnt].state = IDCU_MOD_STATE_RUNNING;
        
        cnt++;
    }
    k->sb_cnt = cnt;
    k->tracked_cnt = cnt;
}

void idcu_kernel_stop(idcu_MicroKernel *k)
{
    printf("[kernel] stopping all modules...\n");
    
    if (k->tracked_cnt == 0) {
        printf("[kernel] no modules to stop\n");
        return;
    }
    
    int stop_failures = 0;
    
    for (int i = (int)k->tracked_cnt - 1; i >= 0; i--) {
        idcu_TrackedModule *tracked = &k->tracked_modules[i];
        
        if (tracked->state != IDCU_MOD_STATE_RUNNING) {
            printf("[kernel] module %s is not running, skipping stop\n", tracked->iface->name);
            continue;
        }
        
        printf("[kernel] stopping module %s\n", tracked->iface->name);
        
        if (tracked->iface->stop) {
            int ret = tracked->iface->stop();
            if (ret != 0) {
                printf("[kernel] warning: module %s stop failed with code %d\n", tracked->iface->name, ret);
                stop_failures++;
            }
        } else {
            printf("[kernel] module %s has no stop function\n", tracked->iface->name);
        }
        
        tracked->state = IDCU_MOD_STATE_STOPPED;
    }
    
    if (stop_failures > 0) {
        printf("[kernel] %d module(s) reported errors during stop\n", stop_failures);
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
