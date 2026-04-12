#include <idcu/testframework/testframework.h>
#include <idcu/json/json.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

IDCU_TEST_CASE(json, parse_null) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("null", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_json_get_type(&value) == IDCU_JSON_TYPE_NULL);
    idcu_json_free(&value);
}

IDCU_TEST_CASE(json, parse_bool_true) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("true", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_json_get_type(&value) == IDCU_JSON_TYPE_BOOL);
    int b;
    ret = idcu_json_get_bool(&value, &b);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(b == 1);
    idcu_json_free(&value);
}

IDCU_TEST_CASE(json, parse_bool_false) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("false", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_json_get_type(&value) == IDCU_JSON_TYPE_BOOL);
    int b;
    ret = idcu_json_get_bool(&value, &b);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(b == 0);
    idcu_json_free(&value);
}

IDCU_TEST_CASE(json, parse_int) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("12345", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_json_get_type(&value) == IDCU_JSON_TYPE_INT);
    int64_t i;
    ret = idcu_json_get_int(&value, &i);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(i == 12345);
    idcu_json_free(&value);
}

IDCU_TEST_CASE(json, parse_double) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("123.45", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_json_get_type(&value) == IDCU_JSON_TYPE_DOUBLE);
    double d;
    ret = idcu_json_get_double(&value, &d);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(d == 123.45);
    idcu_json_free(&value);
}

IDCU_TEST_CASE(json, parse_string) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("\"hello world\"", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_json_get_type(&value) == IDCU_JSON_TYPE_STRING);
    const char* s;
    ret = idcu_json_get_string(&value, &s);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(strcmp(s, "hello world") == 0);
    idcu_json_free(&value);
}

IDCU_TEST_CASE(json, parse_array) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("[1, 2, 3]", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_json_get_type(&value) == IDCU_JSON_TYPE_ARRAY);
    IDCU_TEST_ASSERT(idcu_json_array_size(value.data.array_val) == 3);
    idcu_JsonValue* elem = idcu_json_array_get(value.data.array_val, 1);
    IDCU_TEST_ASSERT(elem != NULL);
    int64_t i;
    ret = idcu_json_get_int(elem, &i);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(i == 2);
    idcu_json_free(&value);
}

IDCU_TEST_CASE(json, parse_object) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("{\"key\": \"value\"}", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_json_get_type(&value) == IDCU_JSON_TYPE_OBJECT);
    IDCU_TEST_ASSERT(idcu_json_object_has(value.data.object_val, "key") == 1);
    idcu_JsonValue* val = idcu_json_object_get(value.data.object_val, "key");
    IDCU_TEST_ASSERT(val != NULL);
    const char* s;
    ret = idcu_json_get_string(val, &s);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(strcmp(s, "value") == 0);
    idcu_json_free(&value);
}

IDCU_TEST_CASE(json, to_string) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("{\"name\": \"test\", \"value\": 123}", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    
    char buffer[256];
    ret = idcu_json_to_string(&value, buffer, sizeof(buffer));
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(strlen(buffer) > 0);
    
    idcu_json_free(&value);
}

IDCU_TEST_CASE(json, to_string_alloc) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("[1, 2, 3]", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    
    char* str = idcu_json_to_string_alloc(&value);
    IDCU_TEST_ASSERT(str != NULL);
    IDCU_TEST_ASSERT(strlen(str) > 0);
    free(str);
    
    idcu_json_free(&value);
}

IDCU_TEST_CASE(json, parse_null_args) {
    idcu_JsonValue value;
    int ret = idcu_json_parse(NULL, &value);
    IDCU_TEST_ASSERT(ret != IDCU_ERR_OK);
    
    ret = idcu_json_parse("null", NULL);
    IDCU_TEST_ASSERT(ret != IDCU_ERR_OK);
}

IDCU_TEST_CASE(json, parse_invalid) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("{invalid}", &value);
    IDCU_TEST_ASSERT(ret != IDCU_ERR_OK);
}

int main(void) {
    return idcu_test_run_all();
}
