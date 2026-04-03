#include <stdio.h>
#include "module/module_def.h"
#include "kernel/micro_kernel.h"
#include "utils/log.h"

static idcu_MicroKernel kernel;

int main(void)
{
    printf("idcu/agent hard real-time microkernel start...\n");

    int log_ret = idcu_log_init(NULL, IDCU_LOG_INFO);
    if (log_ret != IDCU_ERR_SUCCESS) {
        fprintf(stderr, "warning: failed to initialize log system\n");
    }

    idcu_kernel_init(&kernel);
    idcu_kernel_set_signal_handler(&kernel);
    idcu_kernel_start_modules(&kernel);
    idcu_kernel_run(&kernel);

    idcu_log_shutdown();
    printf("idcu/agent hard real-time microkernel stopped.\n");
    return 0;
}
