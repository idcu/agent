#include "micro_kernel.h"
#include <stdio.h>

void kernel_init(MicroKernel *k)
{
    memset(k, 0, sizeof(MicroKernel));
    msg_bus_init(&k->msg);
    ctx_init(&k->global, 0, 0);
}

extern const ModuleInterface __start_modules[];
extern const ModuleInterface __stop_modules[];

void kernel_start_modules(MicroKernel *k)
{
    const ModuleInterface **mod = (const ModuleInterface**)&__start_modules;
    uint32_t cnt = 0;

    while (mod < (const ModuleInterface**)&__stop_modules && cnt < 16) {
        if (*mod) {
            printf("[kernel] module %s init\n", (*mod)->name);
            (*mod)->init();
            k->sandbox[cnt].module_id = cnt;
            k->sandbox[cnt].perm = PERM_SEND | PERM_RECV | PERM_RUN;
            cnt++;
        }
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