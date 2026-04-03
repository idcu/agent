#include <stdio.h>
#include "module/module_def.h"
#include "kernel/micro_kernel.h"

static idcu_MicroKernel kernel;

int main(void)
{
    printf("idcu/agent hard real-time microkernel start...\n");

    idcu_kernel_init(&kernel);
    idcu_kernel_set_signal_handler(&kernel);
    idcu_kernel_start_modules(&kernel);
    idcu_kernel_run(&kernel);

    printf("idcu/agent hard real-time microkernel stopped.\n");
    return 0;
}
