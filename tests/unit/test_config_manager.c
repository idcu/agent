#include "test_framework.h"
#include "config_manager.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static TestSuite g_suite;
static const char* TEST_CONFIG_FILE = "test_config.cfg";

static void test_config_init_shutdown(void) {
    FILE* fp = fopen(TEST_CONFIG_FILE, "w");
    if (fp) {
        fprintf(fp, "[test]\n");
        fprintf(fp, "key1 = value1\n");
        fprintf(fp, "key2 = 123\n");
        fclose(fp);
    }
    
    int ret = config_init(TEST_CONFIG_FILE);
    TEST_ASSERT(ret == ERR_OK || ret == ERR_CONFIG_LOAD, "config_init should work");
    
    if (config_is_loaded()) {
        config_shutdown();
    }
    
    remove(TEST_CONFIG_FILE);
    TEST_PASS();
}

static void test_config_get_set_string(void) {
    int ret = config_init(NULL);
    TEST_ASSERT(ret == ERR_OK, "config_init NULL should work");
    
    const char* val = config_get_string("test", "key", "default");
    TEST_ASSERT(strcmp(val, "default") == 0, "Should return default for non-existent key");
    
    ret = config_set_string("test", "key", "testvalue");
    TEST_ASSERT(ret == ERR_OK, "config_set_string should succeed");
    
    val = config_get_string("test", "key", "default");
    TEST_ASSERT(strcmp(val, "testvalue") == 0, "Should get the set value");
    
    config_shutdown();
    TEST_PASS();
}

static void test_config_get_set_int(void) {
    int ret = config_init(NULL);
    TEST_ASSERT(ret == ERR_OK, "config_init NULL should work");
    
    int val = config_get_int("test", "count", -1);
    TEST_ASSERT(val == -1, "Should return default for non-existent key");
    
    ret = config_set_int("test", "count", 42);
    TEST_ASSERT(ret == ERR_OK, "config_set_int should succeed");
    
    val = config_get_int("test", "count", -1);
    TEST_ASSERT(val == 42, "Should get the set value");
    
    config_shutdown();
    TEST_PASS();
}

static void test_config_has_section_key(void) {
    int ret = config_init(NULL);
    TEST_ASSERT(ret == ERR_OK, "config_init NULL should work");
    
    TEST_ASSERT(config_has_section("nonexistent") == 0, "Should not have nonexistent section");
    
    config_set_string("test", "key", "value");
    
    TEST_ASSERT(config_has_section("test") != 0, "Should have test section");
    TEST_ASSERT(config_has_key("test", "key") != 0, "Should have test key");
    TEST_ASSERT(config_has_key("test", "nonexistent") == 0, "Should not have nonexistent key");
    
    config_shutdown();
    TEST_PASS();
}

int main(void) {
    log_init(NULL, LOG_INFO);
    
    test_suite_init(&g_suite, "Config Manager Tests");
    
    test_suite_add_test(&g_suite, "config_init_shutdown", test_config_init_shutdown);
    test_suite_add_test(&g_suite, "config_get_set_string", test_config_get_set_string);
    test_suite_add_test(&g_suite, "config_get_set_int", test_config_get_set_int);
    test_suite_add_test(&g_suite, "config_has_section_key", test_config_has_section_key);
    
    test_suite_run(&g_suite);
    test_suite_print_summary(&g_suite);
    
    int failures = test_suite_get_failures(&g_suite);
    log_shutdown();
    
    return failures > 0 ? 1 : 0;
}
