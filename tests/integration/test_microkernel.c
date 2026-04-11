#include <idcu/testframework/testframework.h>
#include <idcu/microkernel/kernel.h>
#include <idcu/microkernel/types.h>
#include <stdio.h>
#include <string.h>

IDCU_TEST_CASE(microkernel, init_destroy_simple) {
    idcu_MicroKernel* kernel = NULL;
    idcu_KernelConfig config = {
        .config_path = NULL,
        .enable_logging = false,
        .log_level = 0,
        .max_modules = 16
    };
    
    int ret = idcu_kernel_init(&kernel, &config);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT(kernel != NULL);
    
    idcu_KernelState state = idcu_kernel_get_state(kernel);
    IDCU_TEST_ASSERT_EQUAL(IDCU_KERNEL_STOPPED, state);
    
    idcu_kernel_destroy(kernel);
}

IDCU_TEST_CASE(microkernel, init_destroy_null_config) {
    idcu_MicroKernel* kernel = NULL;
    int ret = idcu_kernel_init(&kernel, NULL);
    IDCU_TEST_ASSERT_NOT_EQUAL(IDCU_ERR_OK, ret);
}

IDCU_TEST_CASE(microkernel, init_destroy_null_kernel_ptr) {
    idcu_KernelConfig config = {
        .config_path = NULL,
        .enable_logging = false,
        .log_level = 0,
        .max_modules = 16
    };
    
    int ret = idcu_kernel_init(NULL, &config);
    IDCU_TEST_ASSERT_NOT_EQUAL(IDCU_ERR_OK, ret);
}

IDCU_TEST_CASE(microkernel, start_stop) {
    idcu_MicroKernel* kernel = NULL;
    idcu_KernelConfig config = {
        .config_path = NULL,
        .enable_logging = false,
        .log_level = 0,
        .max_modules = 16
    };
    
    idcu_kernel_init(&kernel, &config);
    
    idcu_KernelState state = idcu_kernel_get_state(kernel);
    IDCU_TEST_ASSERT_EQUAL(IDCU_KERNEL_STOPPED, state);
    
    int ret = idcu_kernel_start(kernel);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    state = idcu_kernel_get_state(kernel);
    IDCU_TEST_ASSERT(state == IDCU_KERNEL_RUNNING || state == IDCU_KERNEL_STARTING);
    
    ret = idcu_kernel_stop(kernel);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    state = idcu_kernel_get_state(kernel);
    IDCU_TEST_ASSERT(state == IDCU_KERNEL_STOPPED || state == IDCU_KERNEL_STOPPING);
    
    idcu_kernel_destroy(kernel);
}

IDCU_TEST_CASE(microkernel, run) {
    idcu_MicroKernel* kernel = NULL;
    idcu_KernelConfig config = {
        .config_path = NULL,
        .enable_logging = false,
        .log_level = 0,
        .max_modules = 16
    };
    
    idcu_kernel_init(&kernel, &config);
    
    int ret = idcu_kernel_start(kernel);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    ret = idcu_kernel_run(kernel);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    idcu_kernel_stop(kernel);
    idcu_kernel_destroy(kernel);
}

IDCU_TEST_CASE(microkernel, double_destroy) {
    idcu_MicroKernel* kernel = NULL;
    idcu_KernelConfig config = {
        .config_path = NULL,
        .enable_logging = false,
        .log_level = 0,
        .max_modules = 16
    };
    
    idcu_kernel_init(&kernel, &config);
    
    idcu_kernel_destroy(kernel);
    idcu_kernel_destroy(kernel);
    
    IDCU_TEST_PASS();
}

IDCU_TEST_CASE(microkernel, with_logging) {
    idcu_MicroKernel* kernel = NULL;
    idcu_KernelConfig config = {
        .config_path = NULL,
        .enable_logging = true,
        .log_level = 1,
        .max_modules = 16
    };
    
    int ret = idcu_kernel_init(&kernel, &config);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    idcu_kernel_start(kernel);
    idcu_kernel_stop(kernel);
    idcu_kernel_destroy(kernel);
}

IDCU_TEST_CASE(microkernel, custom_max_modules) {
    idcu_MicroKernel* kernel = NULL;
    idcu_KernelConfig config = {
        .config_path = NULL,
        .enable_logging = false,
        .log_level = 0,
        .max_modules = 64
    };
    
    int ret = idcu_kernel_init(&kernel, &config);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    idcu_kernel_destroy(kernel);
}

IDCU_TEST_CASE(microkernel, get_state_null) {
    idcu_KernelState state = idcu_kernel_get_state(NULL);
    IDCU_TEST_PASS();
}

IDCU_TEST_CASE(microkernel, start_null) {
    int ret = idcu_kernel_start(NULL);
    IDCU_TEST_ASSERT_NOT_EQUAL(IDCU_ERR_OK, ret);
}

IDCU_TEST_CASE(microkernel, stop_null) {
    int ret = idcu_kernel_stop(NULL);
    IDCU_TEST_ASSERT_NOT_EQUAL(IDCU_ERR_OK, ret);
}

IDCU_TEST_CASE(microkernel, run_null) {
    int ret = idcu_kernel_run(NULL);
    IDCU_TEST_ASSERT_NOT_EQUAL(IDCU_ERR_OK, ret);
}

IDCU_TEST_CASE(microkernel, full_lifecycle) {
    idcu_MicroKernel* kernel = NULL;
    idcu_KernelConfig config = {
        .config_path = NULL,
        .enable_logging = false,
        .log_level = 0,
        .max_modules = 32
    };
    
    int ret = idcu_kernel_init(&kernel, &config);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    idcu_KernelState state = idcu_kernel_get_state(kernel);
    IDCU_TEST_ASSERT_EQUAL(IDCU_KERNEL_STOPPED, state);
    
    ret = idcu_kernel_start(kernel);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    state = idcu_kernel_get_state(kernel);
    IDCU_TEST_ASSERT(state == IDCU_KERNEL_RUNNING || state == IDCU_KERNEL_STARTING);
    
    ret = idcu_kernel_run(kernel);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    ret = idcu_kernel_stop(kernel);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    state = idcu_kernel_get_state(kernel);
    IDCU_TEST_ASSERT(state == IDCU_KERNEL_STOPPED || state == IDCU_KERNEL_STOPPING);
    
    idcu_kernel_destroy(kernel);
}

int main(void) {
    return idcu_test_run_all();
}
