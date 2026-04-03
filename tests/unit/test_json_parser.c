#include "test/test_framework.h"
#include "utils/json_parser.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static idcu_TestSuite g_suite;

static void test_json_parse_simple_object(void) {
    const char* json_str = "{\"name\": \"test\", \"value\": 42, \"enabled\": true}";
    idcu_JsonValue result;
    
    int ret = idcu_json_parse(json_str, &result);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Parse should succeed");
    IDCU_TEST_ASSERT(idcu_json_get_type(&result) == IDCU_JSON_TYPE_OBJECT, "Should be object");
    
    idcu_JsonValue* name_val = idcu_json_object_get(result.data.object_val, "name");
    IDCU_TEST_ASSERT(name_val != NULL, "Name should exist");
    IDCU_TEST_ASSERT(idcu_json_get_type(name_val) == IDCU_JSON_TYPE_STRING, "Name should be string");
    
    const char* str_val;
    ret = idcu_json_get_string(name_val, &str_val);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Get string should succeed");
    IDCU_TEST_ASSERT(strcmp(str_val, "test") == 0, "Name should be 'test'");
    
    idcu_JsonValue* value_val = idcu_json_object_get(result.data.object_val, "value");
    IDCU_TEST_ASSERT(value_val != NULL, "Value should exist");
    
    int64_t int_val;
    ret = idcu_json_get_int(value_val, &int_val);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Get int should succeed");
    IDCU_TEST_ASSERT(int_val == 42, "Value should be 42");
    
    idcu_JsonValue* enabled_val = idcu_json_object_get(result.data.object_val, "enabled");
    IDCU_TEST_ASSERT(enabled_val != NULL, "Enabled should exist");
    
    int bool_val;
    ret = idcu_json_get_bool(enabled_val, &bool_val);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Get bool should succeed");
    IDCU_TEST_ASSERT(bool_val == 1, "Enabled should be true");
    
    idcu_json_free(&result);
    IDCU_TEST_PASS();
}

static void test_json_parse_array(void) {
    const char* json_str = "[1, 2, 3, 4, 5]";
    idcu_JsonValue result;
    
    int ret = idcu_json_parse(json_str, &result);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Parse array should succeed");
    IDCU_TEST_ASSERT(idcu_json_get_type(&result) == IDCU_JSON_TYPE_ARRAY, "Should be array");
    
    IDCU_TEST_ASSERT(idcu_json_array_size(result.data.array_val) == 5, "Array should have 5 elements");
    
    idcu_JsonValue* elem = idcu_json_array_get(result.data.array_val, 2);
    IDCU_TEST_ASSERT(elem != NULL, "Element 2 should exist");
    
    int64_t int_val;
    ret = idcu_json_get_int(elem, &int_val);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Get int should succeed");
    IDCU_TEST_ASSERT(int_val == 3, "Element 2 should be 3");
    
    idcu_json_free(&result);
    IDCU_TEST_PASS();
}

static void test_json_parse_null(void) {
    const char* json_str = "{\"data\": null}";
    idcu_JsonValue result;
    
    int ret = idcu_json_parse(json_str, &result);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Parse null should succeed");
    
    idcu_JsonValue* data_val = idcu_json_object_get(result.data.object_val, "data");
    IDCU_TEST_ASSERT(data_val != NULL, "Data should exist");
    IDCU_TEST_ASSERT(idcu_json_get_type(data_val) == IDCU_JSON_TYPE_NULL, "Data should be null");
    
    idcu_json_free(&result);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "JSON Parser Tests");
    
    idcu_test_suite_add_test(&g_suite, "json_parse_simple_object", test_json_parse_simple_object);
    idcu_test_suite_add_test(&g_suite, "json_parse_array", test_json_parse_array);
    idcu_test_suite_add_test(&g_suite, "json_parse_null", test_json_parse_null);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
