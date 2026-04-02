#include <stdio.h>
#include "include/module_def.h"
#include "kernel/micro_kernel.h"

static MicroKernel kernel;

int main(void)
{
    printf("idcu/agent hard real-time microkernel start...\n");

    kernel_init(&kernel);
    kernel_start_modules(&kernel);
    kernel_run(&kernel);

    return 0;
}