#include <idcu/microkernel/kernel.h>
#include <idcu/module_system/module_system.h>
#include <idcu/coroutine/coroutine.h>
#include <idcu/msgbus/msgbus.h>
#include <idcu/common/lock.h>
#include <stdlib.h>
#include <string.h>

struct idcu_MicroKernel {
    idcu_KernelState state;
    idcu_KernelConfig config;
    idcu_ModuleSystem module_system;
    idcu_CoroutineScheduler* coro_scheduler;
    idcu_MsgBus* msgbus;
    idcu_Mutex mutex;
    volatile int should_stop;
};

int idcu_kernel_init(idcu_MicroKernel** kernel, const idcu_KernelConfig* config) {
    if (!kernel) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_MicroKernel* k = (idcu_MicroKernel*)malloc(sizeof(idcu_MicroKernel));
    if (!k) {
        return IDCU_ERR_MEMORY;
    }

    memset(k, 0, sizeof(idcu_MicroKernel));
    k->state = IDCU_KERNEL_STOPPED;
    k->should_stop = 0;

    if (config) {
        k->config = *config;
    }

    int ret = idcu_mutex_init(&k->mutex);
    if (ret != IDCU_ERR_OK) {
        free(k);
        return ret;
    }

    ret = idcu_module_system_init(&k->module_system);
    if (ret != IDCU_ERR_OK) {
        idcu_mutex_destroy(&k->mutex);
        free(k);
        return ret;
    }

    ret = idcu_msgbus_init(&k->msgbus);
    if (ret != IDCU_ERR_OK) {
        idcu_module_system_destroy(&k->module_system);
        idcu_mutex_destroy(&k->mutex);
        free(k);
        return ret;
    }

    ret = idcu_coro_scheduler_init(&k->coro_scheduler);
    if (ret != IDCU_ERR_OK) {
        idcu_msgbus_destroy(k->msgbus);
        idcu_module_system_destroy(&k->module_system);
        idcu_mutex_destroy(&k->mutex);
        free(k);
        return ret;
    }

    *kernel = k;
    return IDCU_ERR_OK;
}

void idcu_kernel_destroy(idcu_MicroKernel* kernel) {
    if (!kernel) {
        return;
    }

    idcu_mutex_lock(&kernel->mutex);

    if (kernel->state == IDCU_KERNEL_RUNNING) {
        kernel->should_stop = 1;
        idcu_mutex_unlock(&kernel->mutex);
        idcu_kernel_stop(kernel);
        idcu_mutex_lock(&kernel->mutex);
    }

    if (kernel->coro_scheduler) {
        idcu_coro_scheduler_destroy(kernel->coro_scheduler);
        kernel->coro_scheduler = NULL;
    }

    if (kernel->msgbus) {
        idcu_msgbus_destroy(kernel->msgbus);
        kernel->msgbus = NULL;
    }

    idcu_module_system_destroy(&kernel->module_system);

    kernel->state = IDCU_KERNEL_STOPPED;

    idcu_mutex_unlock(&kernel->mutex);
    idcu_mutex_destroy(&kernel->mutex);

    free(kernel);
}

int idcu_kernel_start(idcu_MicroKernel* kernel) {
    if (!kernel) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&kernel->mutex);

    if (kernel->state != IDCU_KERNEL_STOPPED) {
        idcu_mutex_unlock(&kernel->mutex);
        return IDCU_ERR_INVALID_ARG;
    }

    kernel->state = IDCU_KERNEL_STARTING;
    kernel->should_stop = 0;
    idcu_mutex_unlock(&kernel->mutex);

    int ret = idcu_module_system_init_all(&kernel->module_system);
    if (ret != IDCU_ERR_OK) {
        idcu_mutex_lock(&kernel->mutex);
        kernel->state = IDCU_KERNEL_STOPPED;
        idcu_mutex_unlock(&kernel->mutex);
        return ret;
    }

    ret = idcu_module_system_start_all(&kernel->module_system);
    if (ret != IDCU_ERR_OK) {
        idcu_module_system_stop_all(&kernel->module_system);
        idcu_module_system_destroy_all(&kernel->module_system);
        idcu_mutex_lock(&kernel->mutex);
        kernel->state = IDCU_KERNEL_STOPPED;
        idcu_mutex_unlock(&kernel->mutex);
        return ret;
    }

    idcu_mutex_lock(&kernel->mutex);
    kernel->state = IDCU_KERNEL_RUNNING;
    idcu_mutex_unlock(&kernel->mutex);

    return IDCU_ERR_OK;
}

int idcu_kernel_stop(idcu_MicroKernel* kernel) {
    if (!kernel) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&kernel->mutex);

    if (kernel->state != IDCU_KERNEL_RUNNING && kernel->state != IDCU_KERNEL_STARTING) {
        idcu_mutex_unlock(&kernel->mutex);
        return IDCU_ERR_OK;
    }

    kernel->state = IDCU_KERNEL_STOPPING;
    kernel->should_stop = 1;
    idcu_mutex_unlock(&kernel->mutex);

    idcu_coro_scheduler_stop(kernel->coro_scheduler);

    idcu_module_system_stop_all(&kernel->module_system);
    idcu_module_system_destroy_all(&kernel->module_system);

    idcu_mutex_lock(&kernel->mutex);
    kernel->state = IDCU_KERNEL_STOPPED;
    idcu_mutex_unlock(&kernel->mutex);

    return IDCU_ERR_OK;
}

int idcu_kernel_run(idcu_MicroKernel* kernel) {
    if (!kernel) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&kernel->mutex);
    if (kernel->state != IDCU_KERNEL_RUNNING) {
        idcu_mutex_unlock(&kernel->mutex);
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_unlock(&kernel->mutex);

    while (1) {
        idcu_mutex_lock(&kernel->mutex);
        if (kernel->should_stop || kernel->state != IDCU_KERNEL_RUNNING) {
            idcu_mutex_unlock(&kernel->mutex);
            break;
        }
        idcu_mutex_unlock(&kernel->mutex);

        idcu_msgbus_process(kernel->msgbus);
        idcu_coro_scheduler_run(kernel->coro_scheduler);
    }

    return IDCU_ERR_OK;
}

idcu_KernelState idcu_kernel_get_state(const idcu_MicroKernel* kernel) {
    if (!kernel) {
        return IDCU_KERNEL_STOPPED;
    }
    return kernel->state;
}
