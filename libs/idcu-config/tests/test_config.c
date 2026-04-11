#include <idcu/testframework/testframework.h>
#include <idcu/config/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* test_config_file = "test_config.ini";

IDCU_TEST_CASE(config, init_shutdown) {
    FILE* f = fopen(test_config_file, "w");
    IDCU_TEST_ASSERT(f != NULL);
    if (f) {
        fprintf(f, "[test]\n");
        fprintf(f, "string_key=test_value\n");
        fprintf(f, "int_key=42\n");
        fclose(f);
    }

    int ret = idcu_config_init(test_config_file);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT(idcu_config_is_loaded() != 0);
    
    idcu_config_shutdown();
    IDCU_TEST_ASSERT(idcu_config_is_loaded() == 0);
    
    remove(test_config_file);
}

IDCU_TEST_CASE(config, get_string) {
    FILE* f = fopen(test_config_file, "w");
    IDCU_TEST_ASSERT(f != NULL);
    if (f) {
        fprintf(f, "[test]\n");
        fprintf(f, "key1=hello world\n");
        fprintf(f, "key2=test123\n");
        fclose(f);
    }

    int ret = idcu_config_init(test_config_file);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    const char* val1 = idcu_config_get_string("test", "key1", "default");
    IDCU_TEST_ASSERT_STRING_EQUAL("hello world", val1);
    
    const char* val2 = idcu_config_get_string("test", "key2", "default");
    IDCU_TEST_ASSERT_STRING_EQUAL("test123", val2);
    
    const char* val3 = idcu_config_get_string("test", "nonexistent", "default");
    IDCU_TEST_ASSERT_STRING_EQUAL("default", val3);
    
    idcu_config_shutdown();
    remove(test_config_file);
}

IDCU_TEST_CASE(config, get_int) {
    FILE* f = fopen(test_config_file, "w");
    IDCU_TEST_ASSERT(f != NULL);
    if (f) {
        fprintf(f, "[numbers]\n");
        fprintf(f, "positive=100\n");
        fprintf(f, "negative=-42\n");
        fprintf(f, "zero=0\n");
        fclose(f);
    }

    int ret = idcu_config_init(test_config_file);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    int val1 = idcu_config_get_int("numbers", "positive", -1);
    IDCU_TEST_ASSERT_EQUAL(100, val1);
    
    int val2 = idcu_config_get_int("numbers", "negative", -1);
    IDCU_TEST_ASSERT_EQUAL(-42, val2);
    
    int val3 = idcu_config_get_int("numbers", "zero", -1);
    IDCU_TEST_ASSERT_EQUAL(0, val3);
    
    int val4 = idcu_config_get_int("numbers", "nonexistent", 999);
    IDCU_TEST_ASSERT_EQUAL(999, val4);
    
    idcu_config_shutdown();
    remove(test_config_file);
}

IDCU_TEST_CASE(config, get_bool) {
    FILE* f = fopen(test_config_file, "w");
    IDCU_TEST_ASSERT(f != NULL);
    if (f) {
        fprintf(f, "[flags]\n");
        fprintf(f, "flag1=true\n");
        fprintf(f, "flag2=1\n");
        fprintf(f, "flag3=false\n");
        fprintf(f, "flag4=0\n");
        fprintf(f, "flag5=yes\n");
        fprintf(f, "flag6=no\n");
        fclose(f);
    }

    int ret = idcu_config_init(test_config_file);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    IDCU_TEST_ASSERT(idcu_config_get_bool("flags", "flag1", 0) != 0);
    IDCU_TEST_ASSERT(idcu_config_get_bool("flags", "flag2", 0) != 0);
    IDCU_TEST_ASSERT(idcu_config_get_bool("flags", "flag3", 1) == 0);
    IDCU_TEST_ASSERT(idcu_config_get_bool("flags", "flag4", 1) == 0);
    IDCU_TEST_ASSERT(idcu_config_get_bool("flags", "flag5", 0) != 0);
    IDCU_TEST_ASSERT(idcu_config_get_bool("flags", "flag6", 1) == 0);
    
    idcu_config_shutdown();
    remove(test_config_file);
}

IDCU_TEST_CASE(config, set_and_get) {
    FILE* f = fopen(test_config_file, "w");
    IDCU_TEST_ASSERT(f != NULL);
    if (f) {
        fprintf(f, "[test]\n");
        fclose(f);
    }

    int ret = idcu_config_init(test_config_file);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    ret = idcu_config_set_string("test", "new_key", "new_value");
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    const char* val = idcu_config_get_string("test", "new_key", "default");
    IDCU_TEST_ASSERT_STRING_EQUAL("new_value", val);
    
    ret = idcu_config_set_int("test", "count", 123);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    int count = idcu_config_get_int("test", "count", 0);
    IDCU_TEST_ASSERT_EQUAL(123, count);
    
    idcu_config_shutdown();
    remove(test_config_file);
}

IDCU_TEST_CASE(config, save_config) {
    FILE* f = fopen(test_config_file, "w");
    IDCU_TEST_ASSERT(f != NULL);
    if (f) {
        fprintf(f, "[test]\n");
        fclose(f);
    }

    int ret = idcu_config_init(test_config_file);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    idcu_config_set_string("test", "key1", "value1");
    idcu_config_set_int("test", "key2", 456);
    idcu_config_set_bool("test", "key3", 1);
    
    ret = idcu_config_save("test_config_saved.ini");
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    idcu_config_shutdown();
    
    ret = idcu_config_init("test_config_saved.ini");
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_STRING_EQUAL("value1", idcu_config_get_string("test", "key1", "default"));
    IDCU_TEST_ASSERT_EQUAL(456, idcu_config_get_int("test", "key2", 0));
    IDCU_TEST_ASSERT(idcu_config_get_bool("test", "key3", 0) != 0);
    
    idcu_config_shutdown();
    
    remove(test_config_file);
    remove("test_config_saved.ini");
}

int main(void) {
    return idcu_test_run_all();
}
