#include <idcu/testframework/testframework.h>
#include <idcu/yaml/yaml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

IDCU_TEST_CASE(yaml, parse_null) {
    idcu_YamlValue value;
    int ret = idcu_yaml_parse("null", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_yaml_get_type(&value) == IDCU_YAML_TYPE_NULL);
    idcu_yaml_free(&value);
}

IDCU_TEST_CASE(yaml, parse_null_tilde) {
    idcu_YamlValue value;
    int ret = idcu_yaml_parse("~", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_yaml_get_type(&value) == IDCU_YAML_TYPE_NULL);
    idcu_yaml_free(&value);
}

IDCU_TEST_CASE(yaml, parse_bool_true) {
    idcu_YamlValue value;
    int ret = idcu_yaml_parse("true", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_yaml_get_type(&value) == IDCU_YAML_TYPE_BOOL);
    int b;
    ret = idcu_yaml_get_bool(&value, &b);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(b == 1);
    idcu_yaml_free(&value);
}

IDCU_TEST_CASE(yaml, parse_bool_false) {
    idcu_YamlValue value;
    int ret = idcu_yaml_parse("false", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_yaml_get_type(&value) == IDCU_YAML_TYPE_BOOL);
    int b;
    ret = idcu_yaml_get_bool(&value, &b);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(b == 0);
    idcu_yaml_free(&value);
}

IDCU_TEST_CASE(yaml, parse_int) {
    idcu_YamlValue value;
    int ret = idcu_yaml_parse("12345", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_yaml_get_type(&value) == IDCU_YAML_TYPE_INT);
    int64_t i;
    ret = idcu_yaml_get_int(&value, &i);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(i == 12345);
    idcu_yaml_free(&value);
}

IDCU_TEST_CASE(yaml, parse_double) {
    idcu_YamlValue value;
    int ret = idcu_yaml_parse("123.45", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_yaml_get_type(&value) == IDCU_YAML_TYPE_DOUBLE);
    double d;
    ret = idcu_yaml_get_double(&value, &d);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(d == 123.45);
    idcu_yaml_free(&value);
}

IDCU_TEST_CASE(yaml, parse_string) {
    idcu_YamlValue value;
    int ret = idcu_yaml_parse("hello world", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_yaml_get_type(&value) == IDCU_YAML_TYPE_STRING);
    const char* s;
    ret = idcu_yaml_get_string(&value, &s);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(strcmp(s, "hello world") == 0);
    idcu_yaml_free(&value);
}

IDCU_TEST_CASE(yaml, parse_quoted_string) {
    idcu_YamlValue value;
    int ret = idcu_yaml_parse("\"quoted string\"", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_yaml_get_type(&value) == IDCU_YAML_TYPE_STRING);
    const char* s;
    ret = idcu_yaml_get_string(&value, &s);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(strcmp(s, "quoted string") == 0);
    idcu_yaml_free(&value);
}

IDCU_TEST_CASE(yaml, parse_simple_mapping) {
    const char* yaml_str = 
        "key: value\n"
        "number: 123\n";
    idcu_YamlValue value;
    int ret = idcu_yaml_parse(yaml_str, &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_yaml_get_type(&value) == IDCU_YAML_TYPE_MAPPING);
    IDCU_TEST_ASSERT(idcu_yaml_mapping_has(value.data.mapping_val, "key") == 1);
    IDCU_TEST_ASSERT(idcu_yaml_mapping_has(value.data.mapping_val, "number") == 1);
    
    idcu_YamlValue* val = idcu_yaml_mapping_get(value.data.mapping_val, "key");
    IDCU_TEST_ASSERT(val != NULL);
    const char* s;
    ret = idcu_yaml_get_string(val, &s);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(strcmp(s, "value") == 0);
    
    idcu_yaml_free(&value);
}

IDCU_TEST_CASE(yaml, parse_simple_sequence) {
    const char* yaml_str = 
        "- item1\n"
        "- item2\n"
        "- item3\n";
    idcu_YamlValue value;
    int ret = idcu_yaml_parse(yaml_str, &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_yaml_get_type(&value) == IDCU_YAML_TYPE_SEQUENCE);
    IDCU_TEST_ASSERT(idcu_yaml_sequence_size(value.data.sequence_val) == 3);
    
    idcu_YamlValue* elem = idcu_yaml_sequence_get(value.data.sequence_val, 1);
    IDCU_TEST_ASSERT(elem != NULL);
    const char* s;
    ret = idcu_yaml_get_string(elem, &s);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(strcmp(s, "item2") == 0);
    
    idcu_yaml_free(&value);
}

IDCU_TEST_CASE(yaml, to_string) {
    const char* yaml_str = 
        "name: test\n"
        "value: 123\n";
    idcu_YamlValue value;
    int ret = idcu_yaml_parse(yaml_str, &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    
    char buffer[512];
    ret = idcu_yaml_to_string(&value, buffer, sizeof(buffer));
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(strlen(buffer) > 0);
    
    idcu_yaml_free(&value);
}

IDCU_TEST_CASE(yaml, to_string_alloc) {
    const char* yaml_str = 
        "- 1\n"
        "- 2\n"
        "- 3\n";
    idcu_YamlValue value;
    int ret = idcu_yaml_parse(yaml_str, &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    
    char* str = idcu_yaml_to_string_alloc(&value);
    IDCU_TEST_ASSERT(str != NULL);
    IDCU_TEST_ASSERT(strlen(str) > 0);
    free(str);
    
    idcu_yaml_free(&value);
}

IDCU_TEST_CASE(yaml, to_json) {
    const char* yaml_str = 
        "name: test\n"
        "value: 123\n";
    idcu_YamlValue value;
    int ret = idcu_yaml_parse(yaml_str, &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    
    char buffer[512];
    ret = idcu_yaml_to_json(&value, buffer, sizeof(buffer));
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(strlen(buffer) > 0);
    IDCU_TEST_ASSERT(strstr(buffer, "\"name\"") != NULL);
    IDCU_TEST_ASSERT(strstr(buffer, "\"test\"") != NULL);
    
    idcu_yaml_free(&value);
}

IDCU_TEST_CASE(yaml, parse_null_args) {
    idcu_YamlValue value;
    int ret = idcu_yaml_parse(NULL, &value);
    IDCU_TEST_ASSERT(ret != IDCU_ERR_OK);
    
    ret = idcu_yaml_parse("null", NULL);
    IDCU_TEST_ASSERT(ret != IDCU_ERR_OK);
}

IDCU_TEST_CASE(yaml, parse_empty) {
    idcu_YamlValue value;
    int ret = idcu_yaml_parse("", &value);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(idcu_yaml_get_type(&value) == IDCU_YAML_TYPE_MAPPING);
    idcu_yaml_free(&value);
}

int main(void) {
    return idcu_test_run_all();
}
