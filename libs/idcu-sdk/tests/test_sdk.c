#include <idcu/testframework/testframework.h>
#include <idcu/sdk/sdk.h>
#include <string.h>

static int test_sdk_init_called = 0;
static int test_sdk_start_called = 0;
static int test_sdk_stop_called = 0;
static int test_sdk_destroy_called = 0;

static int test_sdk_test_init(idcu_SdkContext* ctx, void* user_data) {
    (void)ctx;
    (void)user_data;
    test_sdk_init_called++;
    return IDCU_ERR_OK;
}

static int test_sdk_test_start(idcu_SdkContext* ctx, void* user_data) {
    (void)ctx;
    (void)user_data;
    test_sdk_start_called++;
    return IDCU_ERR_OK;
}

static int test_sdk_test_stop(idcu_SdkContext* ctx, void* user_data) {
    (void)ctx;
    (void)user_data;
    test_sdk_stop_called++;
    return IDCU_ERR_OK;
}

static void test_sdk_test_destroy(idcu_SdkContext* ctx, void* user_data) {
    (void)ctx;
    (void)user_data;
    test_sdk_destroy_called++;
}

static int test_sdk_dummy_init(idcu_SdkContext* ctx, void* user_data) {
    (void)ctx;
    (void)user_data;
    return IDCU_ERR_OK;
}

IDCU_SDK_MODULE_DEFINE(
    test_module,
    "1.0.0",
    "Test module",
    test_sdk_test_init,
    test_sdk_test_start,
    test_sdk_test_stop,
    test_sdk_test_destroy,
    NULL
);

IDCU_SDK_MODULE_DEFINE(
    dummy_module,
    "1.0.0",
    "Dummy module",
    test_sdk_dummy_init,
    NULL,
    NULL,
    NULL,
    NULL
);

IDCU_TEST_CASE(sdk, init_destroy) {
    idcu_SdkContext* ctx = NULL;
    int ret = idcu_sdk_init(&ctx);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(ctx != NULL);

    idcu_sdk_destroy(ctx);
}

IDCU_TEST_CASE(sdk, module_definition) {
    test_sdk_init_called = 0;
    test_sdk_start_called = 0;
    test_sdk_stop_called = 0;
    test_sdk_destroy_called = 0;

    const idcu_SdkModuleDef* def = IDCU_SDK_GET_MODULE(test_module);
    IDCU_TEST_ASSERT(def != NULL);
    IDCU_TEST_ASSERT(strcmp(def->name, "test_module") == 0);
    IDCU_TEST_ASSERT(strcmp(def->version, "1.0.0") == 0);
    IDCU_TEST_ASSERT(strcmp(def->description, "Test module") == 0);
    IDCU_TEST_ASSERT(def->init_fn == test_sdk_test_init);
    IDCU_TEST_ASSERT(def->start_fn == test_sdk_test_start);
    IDCU_TEST_ASSERT(def->stop_fn == test_sdk_test_stop);
    IDCU_TEST_ASSERT(def->destroy_fn == test_sdk_test_destroy);
}

IDCU_TEST_CASE(sdk, register_module) {
    idcu_SdkContext* ctx = NULL;
    int ret = idcu_sdk_init(&ctx);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);

    ret = idcu_sdk_register_module(ctx, IDCU_SDK_GET_MODULE(dummy_module));
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);

    idcu_sdk_destroy(ctx);
}

IDCU_TEST_CASE(sdk, error_cases) {
    idcu_SdkContext* ctx = NULL;
    int ret = idcu_sdk_init(NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    ret = idcu_sdk_init(&ctx);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);

    ret = idcu_sdk_register_module(NULL, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    ret = idcu_sdk_register_module(ctx, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    const char* str_val = NULL;
    ret = idcu_sdk_get_config_string(NULL, "key", &str_val);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    int int_val = 0;
    ret = idcu_sdk_get_config_int(NULL, "key", &int_val);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    bool bool_val = false;
    ret = idcu_sdk_get_config_bool(NULL, "key", &bool_val);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    idcu_sdk_destroy(NULL);

    idcu_sdk_destroy(ctx);
}

int main(void) {
    return idcu_test_run_all();
}
