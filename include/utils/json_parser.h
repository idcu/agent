#ifndef IDCU_UTILS_JSON_PARSER_H
#define IDCU_UTILS_JSON_PARSER_H

#include "common/error_code.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // IDCU_UTILS_JSON_PARSER_H

typedef enum {
    JSON_TYPE_NULL = 0,
    JSON_TYPE_BOOL,
    JSON_TYPE_INT,
    JSON_TYPE_DOUBLE,
    JSON_TYPE_STRING,
    JSON_TYPE_ARRAY,
    JSON_TYPE_OBJECT
} JsonType;

typedef struct JsonValue JsonValue;
typedef struct JsonArray JsonArray;
typedef struct JsonObject JsonObject;

struct JsonArray {
    JsonValue* elements;
    size_t count;
    size_t capacity;
};

struct JsonObject {
    char** keys;
    JsonValue* values;
    size_t count;
    size_t capacity;
};

struct JsonValue {
    JsonType type;
    union {
        int bool_val;
        int64_t int_val;
        double double_val;
        char* string_val;
        JsonArray* array_val;
        JsonObject* object_val;
    } data;
};

int json_parse(const char* json_str, JsonValue* result);
void json_free(JsonValue* value);

JsonType json_get_type(const JsonValue* value);

int json_get_bool(const JsonValue* value, int* out);
int json_get_int(const JsonValue* value, int64_t* out);
int json_get_double(const JsonValue* value, double* out);
int json_get_string(const JsonValue* value, const char** out);

size_t json_array_size(const JsonArray* array);
JsonValue* json_array_get(const JsonArray* array, size_t index);

JsonValue* json_object_get(const JsonObject* object, const char* key);
int json_object_has(const JsonObject* object, const char* key);

int json_to_string(const JsonValue* value, char* buffer, size_t buffer_size);
char* json_to_string_alloc(const JsonValue* value);

#ifdef __cplusplus
}
#endif // IDCU_UTILS_JSON_PARSER_H

#endif // IDCU_UTILS_JSON_PARSER_H
