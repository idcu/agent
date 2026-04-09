#include <idcu/testframework/test.h>
#include <idcu/json/json.h>
#include <stdio.h>
#include <string.h>

TEST_CASE(json_parse_null) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("null", &value);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(idcu_json_get_type(&value) == IDCU_JSON_TYPE_NULL);
    idcu_json_free(&value);
}

TEST_CASE(json_parse_bool_true) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("true", &value);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(idcu_json_get_type(&value) == IDCU_JSON_TYPE_BOOL);
    int b;
    ret = idcu_json_get_bool(&value, &b);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(b == 1);
    idcu_json_free(&value);
}

TEST_CASE(json_parse_bool_false) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("false", &value);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(idcu_json_get_type(&value) == IDCU_JSON_TYPE_BOOL);
    int b;
    ret = idcu_json_get_bool(&value, &b);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(b == 0);
    idcu_json_free(&value);
}

TEST_CASE(json_parse_int) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("12345", &value);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(idcu_json_get_type(&value) == IDCU_JSON_TYPE_INT);
    int64_t i;
    ret = idcu_json_get_int(&value, &i);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(i == 12345);
    idcu_json_free(&value);
}

TEST_CASE(json_parse_double) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("123.45", &value);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(idcu_json_get_type(&value) == IDCU_JSON_TYPE_DOUBLE);
    double d;
    ret = idcu_json_get_double(&value, &d);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(d == 123.45);
    idcu_json_free(&value);
}

TEST_CASE(json_parse_string) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("\"hello world\"", &value);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(idcu_json_get_type(&value) == IDCU_JSON_TYPE_STRING);
    const char* s;
    ret = idcu_json_get_string(&value, &s);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(strcmp(s, "hello world") == 0);
    idcu_json_free(&value);
}

TEST_CASE(json_parse_array) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("[1, 2, 3]", &value);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(idcu_json_get_type(&value) == IDCU_JSON_TYPE_ARRAY);
    TEST_ASSERT(idcu_json_array_size(value.data.array_val) == 3);
    idcu_JsonValue* elem = idcu_json_array_get(value.data.array_val, 1);
    TEST_ASSERT(elem != NULL);
    int64_t i;
    ret = idcu_json_get_int(elem, &i);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(i == 2);
    idcu_json_free(&value);
}

TEST_CASE(json_parse_object) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("{\"key\": \"value\"}", &value);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(idcu_json_get_type(&value) == IDCU_JSON_TYPE_OBJECT);
    TEST_ASSERT(idcu_json_object_has(value.data.object_val, "key") == 1);
    idcu_JsonValue* val = idcu_json_object_get(value.data.object_val, "key");
    TEST_ASSERT(val != NULL);
    const char* s;
    ret = idcu_json_get_string(val, &s);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(strcmp(s, "value") == 0);
    idcu_json_free(&value);
}

TEST_CASE(json_to_string) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("{\"name\": \"test\", \"value\": 123}", &value);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    
    char buffer[256];
    ret = idcu_json_to_string(&value, buffer, sizeof(buffer));
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(strlen(buffer) > 0);
    
    idcu_json_free(&value);
}

TEST_CASE(json_to_string_alloc) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("[1, 2, 3]", &value);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    
    char* str = idcu_json_to_string_alloc(&value);
    TEST_ASSERT(str != NULL);
    TEST_ASSERT(strlen(str) > 0);
    free(str);
    
    idcu_json_free(&value);
}

TEST_CASE(json_parse_null_args) {
    idcu_JsonValue value;
    int ret = idcu_json_parse(NULL, &value);
    TEST_ASSERT(ret != IDCU_ERR_OK);
    
    ret = idcu_json_parse("null", NULL);
    TEST_ASSERT(ret != IDCU_ERR_OK);
}

TEST_CASE(json_parse_invalid) {
    idcu_JsonValue value;
    int ret = idcu_json_parse("{invalid}", &value);
    TEST_ASSERT(ret != IDCU_ERR_OK);
}

TEST_SUITE_BEGIN(json_suite)
    TEST_SUITE_ADD(json_parse_null)
    TEST_SUITE_ADD(json_parse_bool_true)
    TEST_SUITE_ADD(json_parse_bool_false)
    TEST_SUITE_ADD(json_parse_int)
    TEST_SUITE_ADD(json_parse_double)
    TEST_SUITE_ADD(json_parse_string)
    TEST_SUITE_ADD(json_parse_array)
    TEST_SUITE_ADD(json_parse_object)
    TEST_SUITE_ADD(json_to_string)
    TEST_SUITE_ADD(json_to_string_alloc)
    TEST_SUITE_ADD(json_parse_null_args)
    TEST_SUITE_ADD(json_parse_invalid)
TEST_SUITE_END()

TEST_MAIN(json_suite)
