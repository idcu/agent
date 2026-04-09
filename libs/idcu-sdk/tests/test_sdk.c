#include <idcu/testframework/testframework.h>
#include <idcu/sdk/sdk.h>
#include <string.h>

TEST_CASE(sdk_init_destroy) {
    idcu_SdkContext* ctx = NULL;
    int ret = idcu_sdk_init(&ctx);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(ctx != NULL);

    idcu_sdk_destroy(ctx);
    TEST_PASS();
}

TEST_CASE(sdk_module_definition) {
    static int init_called = 0;
    static int start_called = 0;
    static int stop_called = 0;
    static int destroy_called = 0;

    static int test_init(idcu_SdkContext* ctx, void* user_data) {
        (void)ctx;
        (void)user_data;
        init_called++;
        return IDCU_ERR_OK;
    }

    static int test_start(idcu_SdkContext* ctx, void* user_data) {
        (void)ctx;
        (void)user_data;
        start_called++;
        return IDCU_ERR_OK;
    }

    static int test_stop(idcu_SdkContext* ctx, void* user_data) {
        (void)ctx;
        (void)user_data;
        stop_called++;
        return IDCU_ERR_OK;
    }

    static void test_destroy(idcu_SdkContext* ctx, void* user_data) {
        (void)ctx;
        (void)user_data;
        destroy_called++;
    }

    IDCU_SDK_MODULE_DEFINE(
        test_module,
        "1.0.0",
        "Test module",
        test_init,
        test_start,
        test_stop,
        test_destroy,
        NULL
    );

    const idcu_SdkModuleDef* def = IDCU_SDK_GET_MODULE(test_module);
    TEST_ASSERT(def != NULL);
    TEST_ASSERT(strcmp(def->name, "test_module") == 0);
    TEST_ASSERT(strcmp(def->version, "1.0.0") == 0);
    TEST_ASSERT(strcmp(def->description, "Test module") == 0);
    TEST_ASSERT(def->init_fn == test_init);
    TEST_ASSERT(def->start_fn == test_start);
    TEST_ASSERT(def->stop_fn == test_stop);
    TEST_ASSERT(def->destroy_fn == test_destroy);

    TEST_PASS();
}

TEST_CASE(sdk_register_module) {
    idcu_SdkContext* ctx = NULL;
    int ret = idcu_sdk_init(&ctx);
    TEST_ASSERT(ret == IDCU_ERR_OK);

    static int dummy_init(idcu_SdkContext* ctx, void* user_data) {
        (void)ctx;
        (void)user_data;
        return IDCU_ERR_OK;
    }

    IDCU_SDK_MODULE_DEFINE(
        dummy_module,
        "1.0.0",
        "Dummy module",
        dummy_init,
        NULL,
        NULL,
        NULL,
        NULL
    );

    ret = idcu_sdk_register_module(ctx, IDCU_SDK_GET_MODULE(dummy_module));
    TEST_ASSERT(ret == IDCU_ERR_OK);

    idcu_sdk_destroy(ctx);
    TEST_PASS();
}

TEST_CASE(sdk_error_cases) {
    idcu_SdkContext* ctx = NULL;
    int ret = idcu_sdk_init(NULL);
    TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    ret = idcu_sdk_init(&ctx);
    TEST_ASSERT(ret == IDCU_ERR_OK);

    ret = idcu_sdk_register_module(NULL, NULL);
    TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    ret = idcu_sdk_register_module(ctx, NULL);
    TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    const char* str_val = NULL;
    ret = idcu_sdk_get_config_string(NULL, "key", &str_val);
    TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    int int_val = 0;
    ret = idcu_sdk_get_config_int(NULL, "key", &int_val);
    TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    bool bool_val = false;
    ret = idcu_sdk_get_config_bool(NULL, "key", &bool_val);
    TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    idcu_sdk_destroy(NULL);

    idcu_sdk_destroy(ctx);
    TEST_PASS();
}

TEST_MAIN() {
    RUN_TEST(sdk_init_destroy);
    RUN_TEST(sdk_module_definition);
    RUN_TEST(sdk_register_module);
    RUN_TEST(sdk_error_cases);
    return 0;
}
