#include <stdio.h>
#include "module_def.h"
#include "micro_kernel.h"
#include "idcu/log/log.h"

static idcu_MicroKernel g_kernel;

int main(void)
{
    printf("idcu/agent hard real-time microkernel start...\n");

    int log_ret = idcu_log_init(NULL, IDCU_LOG_INFO);
    if (log_ret != IDCU_ERR_SUCCESS) {
        fprintf(stderr, "warning: failed to initialize log system, using console output fallback\n");
        idcu_log_set_level(IDCU_LOG_INFO);
    }

    idcu_kernel_init(&g_kernel);
    
    idcu_kernel_set_signal_handler(&g_kernel);
    
    idcu_kernel_start_modules(&g_kernel);
    
    idcu_kernel_run(&g_kernel);
    idcu_kernel_stop(&g_kernel);
    idcu_log_shutdown();
    printf("idcu/agent hard real-time microkernel stopped.\n");
    return 0;
}
