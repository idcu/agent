#include <idcu/testframework/testframework.h>
#include <idcu/microkernel/kernel.h>
#include <string.h>

IDCU_TEST_CASE(microkernel, init_destroy) {
    idcu_KernelConfig config = {
        .config_path = NULL,
        .enable_logging = false,
        .log_level = 0,
        .max_modules = 32
    };

    idcu_MicroKernel* kernel = NULL;
    int ret = idcu_kernel_init(&kernel, &config);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(kernel != NULL);
    IDCU_TEST_ASSERT(idcu_kernel_get_state(kernel) == IDCU_KERNEL_STOPPED);

    idcu_kernel_destroy(kernel);
}

IDCU_TEST_CASE(microkernel, lifecycle) {
    idcu_KernelConfig config = {
        .config_path = NULL,
        .enable_logging = false,
        .log_level = 0,
        .max_modules = 32
    };

    idcu_MicroKernel* kernel = NULL;
    int ret = idcu_kernel_init(&kernel, &config);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);

    IDCU_TEST_ASSERT(idcu_kernel_get_state(kernel) == IDCU_KERNEL_STOPPED);

    ret = idcu_kernel_start(kernel);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_kernel_get_state(kernel) == IDCU_KERNEL_RUNNING);

    ret = idcu_kernel_stop(kernel);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_kernel_get_state(kernel) == IDCU_KERNEL_STOPPED);

    idcu_kernel_destroy(kernel);
}

IDCU_TEST_CASE(microkernel, error_cases) {
    idcu_KernelConfig config = {
        .config_path = NULL,
        .enable_logging = false,
        .log_level = 0,
        .max_modules = 32
    };

    int ret = idcu_kernel_init(NULL, &config);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    idcu_MicroKernel* kernel = NULL;
    ret = idcu_kernel_init(&kernel, &config);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);

    ret = idcu_kernel_start(NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    ret = idcu_kernel_stop(NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    ret = idcu_kernel_run(NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    IDCU_TEST_ASSERT(idcu_kernel_get_state(NULL) == IDCU_KERNEL_STOPPED);

    idcu_kernel_destroy(NULL);

    idcu_kernel_destroy(kernel);
}

int main(void) {
    return idcu_test_run_all();
}
