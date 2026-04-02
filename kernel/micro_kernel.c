#include "micro_kernel.h"
#include <stdio.h>
#include <string.h>

// 为避免 Windows/MinGW 链接问题，我们手动声明几个模块
// 在实际项目中，您可以使用其他方式处理模块加载
extern const ModuleInterface __module_base_log;
extern const ModuleInterface __module_biz_collect;
extern const ModuleInterface __module_core_module;

static const ModuleInterface* modules[] = {
    &__module_base_log,
    &__module_biz_collect,
    &__module_core_module,
    NULL
};

void kernel_init(MicroKernel *k)
{
    memset(k, 0, sizeof(MicroKernel));
    msg_bus_init(&k->msg);
    ctx_init(&k->global, 0, 0);
}

void kernel_start_modules(MicroKernel *k)
{
    uint32_t cnt = 0;
    const ModuleInterface** mod = modules;

    while (*mod && cnt < 16) {
        printf("[kernel] module %s init\n", (*mod)->name);
        (*mod)->init();
        k->sandbox[cnt].module_id = cnt;
        k->sandbox[cnt].perm = PERM_SEND | PERM_RECV | PERM_RUN;
        cnt++;
        mod++;
    }
    k->sb_cnt = cnt;
}

void kernel_run(MicroKernel *k)
{
    while (1) {
        coro_sched_run(&k->coro);

        StackContext ctx;
        for (uint32_t i = 0; i < k->sb_cnt; ++i) {
            Sandbox *sb = &k->sandbox[i];
            if (sandbox_perm_check(sb, PERM_RECV) == 0) {
                msg_recv(&k->msg, i, &ctx);
            }
        }
    }
}