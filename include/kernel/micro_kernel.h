#ifndef IDCU_KERNEL_MICRO_KERNEL_H
#define IDCU_KERNEL_MICRO_KERNEL_H

#include "scheduler/coroutine.h"
#include "scheduler/msg_bus.h"
#include "security/sandbox.h"
#include "module/module_def.h"

typedef struct {
    CoroScheduler coro;
    MessageBus    msg;
    Sandbox       sandbox[16];
    uint32_t      sb_cnt;
    StackContext  global;
} MicroKernel;

void kernel_init(MicroKernel *k);
void kernel_start_modules(MicroKernel *k);
void kernel_run(MicroKernel *k);

#endif // IDCU_KERNEL_MICRO_KERNEL_H