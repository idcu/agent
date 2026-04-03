#include "test_framework.h"
#include "json_parser.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

static TestSuite g_suite;

static void test_json_parse_simple_object(void) {
    const char* json_str = "{\"name\": \"test\", \"value\": 42, \"enabled\": true}";
    JsonValue result;
    
    int ret = json_parse(json_str, &result);
    TEST_ASSERT(ret == ERR_OK, "Parse should succeed");
    TEST_ASSERT(result.type == JSON_TYPE_OBJECT, "Should be object");
    
    JsonValue* name_val = json_object_get(result.data.object_val, "name");
    TEST_ASSERT(name_val != NULL, "Name should exist");
    TEST_ASSERT(name_val->type == JSON_TYPE_STRING, "Name should be string");
    
    const char* str_val;
    ret = json_get_string(name_val, &str_val);
    TEST_ASSERT(ret == ERR_OK, "Get string should succeed");
    TEST_ASSERT(strcmp(str_val, "test") == 0, "Name should be 'test'");
    
    JsonValue* value_val = json_object_get(result.data.object_val, "value");
    TEST_ASSERT(value_val != NULL, "Value should exist");
    
    int64_t int_val;
    ret = json_get_int(value_val, &int_val);
    TEST_ASSERT(ret == ERR_OK, "Get int should succeed");
    TEST_ASSERT(int_val == 42, "Value should be 42");
    
    JsonValue* enabled_val = json_object_get(result.data.object_val, "enabled");
    TEST_ASSERT(enabled_val != NULL, "Enabled should exist");
    
    int bool_val;
    ret = json_get_bool(enabled_val, &bool_val);
    TEST_ASSERT(ret == ERR_OK, "Get bool should succeed");
    TEST_ASSERT(bool_val == 1, "Enabled should be true");
    
    json_free(&result);
    TEST_PASS();
}

static void test_json_parse_array(void) {
    const char* json_str = "[1, 2, 3, 4, 5]";
    JsonValue result;
    
    int ret = json_parse(json_str, &result);
    TEST_ASSERT(ret == ERR_OK, "Parse array should succeed");
    TEST_ASSERT(result.type == JSON_TYPE_ARRAY, "Should be array");
    
    TEST_ASSERT(result.data.array_val->count == 5, "Array should have 5 elements");
    
    JsonValue* elem = json_array_get(result.data.array_val, 2);
    TEST_ASSERT(elem != NULL, "Element 2 should exist");
    
    int64_t int_val;
    ret = json_get_int(elem, &int_val);
    TEST_ASSERT(ret == ERR_OK, "Get int should succeed");
    TEST_ASSERT(int_val == 3, "Element 2 should be 3");
    
    json_free(&result);
    TEST_PASS();
}

static void test_json_parse_null(void) {
    const char* json_str = "{\"data\": null}";
    JsonValue result;
    
    int ret = json_parse(json_str, &result);
    TEST_ASSERT(ret == ERR_OK, "Parse null should succeed");
    
    JsonValue* data_val = json_object_get(result.data.object_val, "data");
    TEST_ASSERT(data_val != NULL, "Data should exist");
    TEST_ASSERT(data_val->type == JSON_TYPE_NULL, "Data should be null");
    
    json_free(&result);
    TEST_PASS();
}

int main(void) {
    log_init(NULL, LOG_INFO);
    
    test_suite_init(&g_suite, "JSON Parser Tests");
    
    test_suite_add_test(&g_suite, "json_parse_simple_object", test_json_parse_simple_object);
    test_suite_add_test(&g_suite, "json_parse_array", test_json_parse_array);
    test_suite_add_test(&g_suite, "json_parse_null", test_json_parse_null);
    
    test_suite_run(&g_suite);
    test_suite_print_summary(&g_suite);
    
    int failures = test_suite_get_failures(&g_suite);
    log_shutdown();
    
    return failures > 0 ? 1 : 0;
}
