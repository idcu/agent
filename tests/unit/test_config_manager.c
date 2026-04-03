#include "test/test_framework.h"
#include "utils/config_manager.h"
#include "utils/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static idcu_TestSuite g_suite;
static const char* TEST_CONFIG_FILE = "test_config.cfg";

static void test_config_init_shutdown(void) {
    FILE* fp = fopen(TEST_CONFIG_FILE, "w");
    if (fp) {
        fprintf(fp, "[test]\n");
        fprintf(fp, "key1 = value1\n");
        fprintf(fp, "key2 = 123\n");
        fclose(fp);
    }
    
    int ret = idcu_config_init(TEST_CONFIG_FILE);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK || ret == IDCU_ERR_CONFIG_LOAD, "idcu_config_init should work");
    
    if (idcu_config_is_loaded()) {
        idcu_config_shutdown();
    }
    
    remove(TEST_CONFIG_FILE);
    IDCU_TEST_PASS();
}

static void test_config_get_set_string(void) {
    int ret = idcu_config_init(NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_config_init NULL should work");
    
    const char* val = idcu_config_get_string("test", "key", "default");
    IDCU_TEST_ASSERT(strcmp(val, "default") == 0, "Should return default for non-existent key");
    
    ret = idcu_config_set_string("test", "key", "testvalue");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_config_set_string should succeed");
    
    val = idcu_config_get_string("test", "key", "default");
    IDCU_TEST_ASSERT(strcmp(val, "testvalue") == 0, "Should get the set value");
    
    idcu_config_shutdown();
    IDCU_TEST_PASS();
}

static void test_config_get_set_int(void) {
    int ret = idcu_config_init(NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_config_init NULL should work");
    
    int val = idcu_config_get_int("test", "count", -1);
    IDCU_TEST_ASSERT(val == -1, "Should return default for non-existent key");
    
    ret = idcu_config_set_int("test", "count", 42);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_config_set_int should succeed");
    
    val = idcu_config_get_int("test", "count", -1);
    IDCU_TEST_ASSERT(val == 42, "Should get the set value");
    
    idcu_config_shutdown();
    IDCU_TEST_PASS();
}

static void test_config_has_section_key(void) {
    int ret = idcu_config_init(NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_config_init NULL should work");
    
    IDCU_TEST_ASSERT(idcu_config_has_section("nonexistent") == 0, "Should not have nonexistent section");
    
    idcu_config_set_string("test", "key", "value");
    
    IDCU_TEST_ASSERT(idcu_config_has_section("test") != 0, "Should have test section");
    IDCU_TEST_ASSERT(idcu_config_has_key("test", "key") != 0, "Should have test key");
    IDCU_TEST_ASSERT(idcu_config_has_key("test", "nonexistent") == 0, "Should not have nonexistent key");
    
    idcu_config_shutdown();
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Config Manager Tests");
    
    idcu_test_suite_add_test(&g_suite, "config_init_shutdown", test_config_init_shutdown);
    idcu_test_suite_add_test(&g_suite, "config_get_set_string", test_config_get_set_string);
    idcu_test_suite_add_test(&g_suite, "config_get_set_int", test_config_get_set_int);
    idcu_test_suite_add_test(&g_suite, "config_has_section_key", test_config_has_section_key);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
