#include "kernel/micro_kernel.h"
#include <stdio.h>
#include <string.h>

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

void idcu_kernel_init(idcu_MicroKernel *k)
{
    memset(k, 0, sizeof(idcu_MicroKernel));
    idcu_msg_bus_init(&k->msg);
    idcu_ctx_init(&k->global, 0, 0);
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

void idcu_kernel_run(idcu_MicroKernel *k)
{
    while (1) {
        idcu_coro_sched_run(&k->coro);

        idcu_Message msg;
        for (uint32_t i = 0; i < k->sb_cnt; ++i) {
            idcu_Sandbox *sb = &k->sandbox[i];
            if (idcu_sandbox_perm_check(sb, IDCU_PERM_RECV) == 0) {
                idcu_msg_recv(&k->msg, i, &msg);
            }
        }
    }
}
