#include <idcu/testframework/testframework.h>
#include <idcu/microkernel/kernel.h>
#include <string.h>

TEST_CASE(microkernel_init_destroy) {
    idcu_KernelConfig config = {
        .config_path = NULL,
        .enable_logging = false,
        .log_level = 0,
        .max_modules = 32
    };

    idcu_MicroKernel* kernel = NULL;
    int ret = idcu_kernel_init(&kernel, &config);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(kernel != NULL);
    TEST_ASSERT(idcu_kernel_get_state(kernel) == IDCU_KERNEL_STOPPED);

    idcu_kernel_destroy(kernel);
    TEST_PASS();
}

TEST_CASE(microkernel_lifecycle) {
    idcu_KernelConfig config = {
        .config_path = NULL,
        .enable_logging = false,
        .log_level = 0,
        .max_modules = 32
    };

    idcu_MicroKernel* kernel = NULL;
    int ret = idcu_kernel_init(&kernel, &config);
    TEST_ASSERT(ret == IDCU_ERR_OK);

    TEST_ASSERT(idcu_kernel_get_state(kernel) == IDCU_KERNEL_STOPPED);

    ret = idcu_kernel_start(kernel);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(idcu_kernel_get_state(kernel) == IDCU_KERNEL_RUNNING);

    ret = idcu_kernel_stop(kernel);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(idcu_kernel_get_state(kernel) == IDCU_KERNEL_STOPPED);

    idcu_kernel_destroy(kernel);
    TEST_PASS();
}

TEST_CASE(microkernel_error_cases) {
    idcu_KernelConfig config = {
        .config_path = NULL,
        .enable_logging = false,
        .log_level = 0,
        .max_modules = 32
    };

    int ret = idcu_kernel_init(NULL, &config);
    TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    idcu_MicroKernel* kernel = NULL;
    ret = idcu_kernel_init(&kernel, &config);
    TEST_ASSERT(ret == IDCU_ERR_OK);

    ret = idcu_kernel_start(NULL);
    TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    ret = idcu_kernel_stop(NULL);
    TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    ret = idcu_kernel_run(NULL);
    TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    TEST_ASSERT(idcu_kernel_get_state(NULL) == IDCU_KERNEL_STOPPED);

    idcu_kernel_destroy(NULL);

    idcu_kernel_destroy(kernel);
    TEST_PASS();
}

TEST_MAIN() {
    RUN_TEST(microkernel_init_destroy);
    RUN_TEST(microkernel_lifecycle);
    RUN_TEST(microkernel_error_cases);
    return 0;
}
