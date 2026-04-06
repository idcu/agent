#include <idcu/json/json.h>
#include <stdio.h>
#include <string.h>

int test_json_parse_simple_object(void)
{
    const char* json_str = "{\"name\": \"test\", \"value\": 42, \"enabled\": true}";
    idcu_JsonValue result;
    
    int ret = idcu_json_parse(json_str, &result);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Test failed: Parse should succeed\n");
        return 1;
    }
    
    if (idcu_json_get_type(&result) != IDCU_JSON_TYPE_OBJECT) {
        printf("Test failed: Should be object\n");
        idcu_json_free(&result);
        return 1;
    }
    
    idcu_JsonValue* name_val = idcu_json_object_get(result.data.object_val, "name");
    if (!name_val) {
        printf("Test failed: Name should exist\n");
        idcu_json_free(&result);
        return 1;
    }
    
    if (idcu_json_get_type(name_val) != IDCU_JSON_TYPE_STRING) {
        printf("Test failed: Name should be string\n");
        idcu_json_free(&result);
        return 1;
    }
    
    const char* str_val;
    ret = idcu_json_get_string(name_val, &str_val);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Test failed: Get string should succeed\n");
        idcu_json_free(&result);
        return 1;
    }
    
    if (strcmp(str_val, "test") != 0) {
        printf("Test failed: Name should be 'test'\n");
        idcu_json_free(&result);
        return 1;
    }
    
    idcu_JsonValue* value_val = idcu_json_object_get(result.data.object_val, "value");
    if (!value_val) {
        printf("Test failed: Value should exist\n");
        idcu_json_free(&result);
        return 1;
    }
    
    int64_t int_val;
    ret = idcu_json_get_int(value_val, &int_val);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Test failed: Get int should succeed\n");
        idcu_json_free(&result);
        return 1;
    }
    
    if (int_val != 42) {
        printf("Test failed: Value should be 42\n");
        idcu_json_free(&result);
        return 1;
    }
    
    idcu_JsonValue* enabled_val = idcu_json_object_get(result.data.object_val, "enabled");
    if (!enabled_val) {
        printf("Test failed: Enabled should exist\n");
        idcu_json_free(&result);
        return 1;
    }
    
    int bool_val;
    ret = idcu_json_get_bool(enabled_val, &bool_val);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Test failed: Get bool should succeed\n");
        idcu_json_free(&result);
        return 1;
    }
    
    if (bool_val != 1) {
        printf("Test failed: Enabled should be true\n");
        idcu_json_free(&result);
        return 1;
    }
    
    idcu_json_free(&result);
    printf("test_json_parse_simple_object passed\n");
    return 0;
}

int test_json_parse_array(void)
{
    const char* json_str = "[1, 2, 3, 4, 5]";
    idcu_JsonValue result;
    
    int ret = idcu_json_parse(json_str, &result);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Test failed: Parse array should succeed\n");
        return 1;
    }
    
    if (idcu_json_get_type(&result) != IDCU_JSON_TYPE_ARRAY) {
        printf("Test failed: Should be array\n");
        idcu_json_free(&result);
        return 1;
    }
    
    if (idcu_json_array_size(result.data.array_val) != 5) {
        printf("Test failed: Array should have 5 elements\n");
        idcu_json_free(&result);
        return 1;
    }
    
    idcu_JsonValue* elem = idcu_json_array_get(result.data.array_val, 2);
    if (!elem) {
        printf("Test failed: Element 2 should exist\n");
        idcu_json_free(&result);
        return 1;
    }
    
    int64_t int_val;
    ret = idcu_json_get_int(elem, &int_val);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Test failed: Get int should succeed\n");
        idcu_json_free(&result);
        return 1;
    }
    
    if (int_val != 3) {
        printf("Test failed: Element 2 should be 3\n");
        idcu_json_free(&result);
        return 1;
    }
    
    idcu_json_free(&result);
    printf("test_json_parse_array passed\n");
    return 0;
}

int test_json_parse_null(void)
{
    const char* json_str = "{\"data\": null}";
    idcu_JsonValue result;
    
    int ret = idcu_json_parse(json_str, &result);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Test failed: Parse null should succeed\n");
        return 1;
    }
    
    idcu_JsonValue* data_val = idcu_json_object_get(result.data.object_val, "data");
    if (!data_val) {
        printf("Test failed: Data should exist\n");
        idcu_json_free(&result);
        return 1;
    }
    
    if (idcu_json_get_type(data_val) != IDCU_JSON_TYPE_NULL) {
        printf("Test failed: Data should be null\n");
        idcu_json_free(&result);
        return 1;
    }
    
    idcu_json_free(&result);
    printf("test_json_parse_null passed\n");
    return 0;
}

int main(void)
{
    int failures = 0;
    
    failures += test_json_parse_simple_object();
    failures += test_json_parse_array();
    failures += test_json_parse_null();
    
    if (failures == 0) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("%d tests failed!\n", failures);
        return 1;
    }
}
