#ifndef IDCU_MICROKERNEL_KERNEL_H
#define IDCU_MICROKERNEL_KERNEL_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/microkernel/types.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_kernel_init(idcu_MicroKernel** kernel, const idcu_KernelConfig* config);
void idcu_kernel_destroy(idcu_MicroKernel* kernel);

int idcu_kernel_start(idcu_MicroKernel* kernel);
int idcu_kernel_stop(idcu_MicroKernel* kernel);
int idcu_kernel_run(idcu_MicroKernel* kernel);

idcu_KernelState idcu_kernel_get_state(const idcu_MicroKernel* kernel);

#ifdef __cplusplus
}
#endif

#endif
