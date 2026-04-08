# 任务 3.2: idcu-json - JSON 解析与序列化库

## 目标

创建完整的 JSON 解析与序列化库，支持：
- JSON 解析（从字符串或文件）
- JSON 序列化（到字符串或文件）
- 所有 JSON 类型（null, bool, number, string, array, object）
- 类型安全的访问接口
- 内存管理

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-json/include/idcu/json
mkdir -p libs/idcu-json/src/idcu/json
mkdir -p libs/idcu-json/tests
mkdir -p libs/idcu-json/examples
```

### 2. 创建 JSON 头文件 (json.h)

创建 `libs/idcu-json/include/idcu/json/json.h`：

```c
#ifndef IDCU_JSON_JSON_H
#define IDCU_JSON_JSON_H

#include "idcu/common/error_code.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_JSON_TYPE_NULL = 0,
    IDCU_JSON_TYPE_BOOL,
    IDCU_JSON_TYPE_INT,
    IDCU_JSON_TYPE_DOUBLE,
    IDCU_JSON_TYPE_STRING,
    IDCU_JSON_TYPE_ARRAY,
    IDCU_JSON_TYPE_OBJECT
} idcu_JsonType;

typedef struct idcu_JsonValue  idcu_JsonValue;
typedef struct idcu_JsonArray  idcu_JsonArray;
typedef struct idcu_JsonObject idcu_JsonObject;

struct idcu_JsonArray
{
    idcu_JsonValue* elements;
    size_t          count;
    size_t          capacity;
};

struct idcu_JsonObject
{
    char**          keys;
    idcu_JsonValue* values;
    size_t          count;
    size_t          capacity;
};

struct idcu_JsonValue
{
    idcu_JsonType type;
    union
    {
        int              bool_val;
        int64_t          int_val;
        double           double_val;
        char*            string_val;
        idcu_JsonArray*  array_val;
        idcu_JsonObject* object_val;
    } data;
};

int  idcu_json_parse(const char* json_str, idcu_JsonValue* result);
int  idcu_json_parse_file(const char* file_path, idcu_JsonValue* result);
void idcu_json_free(idcu_JsonValue* value);

idcu_JsonType idcu_json_get_type(const idcu_JsonValue* value);

int idcu_json_get_bool(const idcu_JsonValue* value, int* out);
int idcu_json_get_int(const idcu_JsonValue* value, int64_t* out);
int idcu_json_get_double(const idcu_JsonValue* value, double* out);
int idcu_json_get_string(const idcu_JsonValue* value, const char** out);

size_t          idcu_json_array_size(const idcu_JsonArray* array);
idcu_JsonValue* idcu_json_array_get(const idcu_JsonArray* array, size_t index);

idcu_JsonValue* idcu_json_object_get(const idcu_JsonObject* object, const char* key);
int             idcu_json_object_has(const idcu_JsonObject* object, const char* key);

int   idcu_json_to_string(const idcu_JsonValue* value, char* buffer, size_t buffer_size);
char* idcu_json_to_string_alloc(const idcu_JsonValue* value);
int   idcu_json_save_to_file(const idcu_JsonValue* value, const char* file_path);

idcu_JsonValue* idcu_json_create_null(void);
idcu_JsonValue* idcu_json_create_bool(int value);
idcu_JsonValue* idcu_json_create_int(int64_t value);
idcu_JsonValue* idcu_json_create_double(double value);
idcu_JsonValue* idcu_json_create_string(const char* value);
idcu_JsonValue* idcu_json_create_array(void);
idcu_JsonValue* idcu_json_create_object(void);

int idcu_json_array_add(idcu_JsonArray* array, idcu_JsonValue* value);
int idcu_json_object_set(idcu_JsonObject* object, const char* key, idcu_JsonValue* value);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 JSON 实现文件 (json.c)

创建 `libs/idcu-json/src/idcu/json/json.c`：

```c
#include "idcu/json/json.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define JSON_INITIAL_CAPACITY 8
#define JSON_GROWTH_FACTOR 2

typedef struct
{
    const char* pos;
    const char* end;
    int         line;
    int         column;
} idcu_JsonParser;

static void json_skip_whitespace(idcu_JsonParser* parser)
{
    while (parser->pos < parser->end) {
        char c = *parser->pos;
        if (c == ' ' || c == '\t' || c == '\r') {
            parser->pos++;
            parser->column++;
        } else if (c == '\n') {
            parser->pos++;
            parser->line++;
            parser->column = 1;
        } else {
            break;
        }
    }
}

static int json_parse_string(idcu_JsonParser* parser, char** out)
{
    parser->pos++;
    parser->column++;

    const char* start = parser->pos;
    size_t len = 0;

    while (parser->pos < parser->end && *parser->pos != '"') {
        if (*parser->pos == '\\') {
            parser->pos++;
            if (parser->pos >= parser->end) break;
        }
        parser->pos++;
        parser->column++;
        len++;
    }

    if (parser->pos >= parser->end || *parser->pos != '"') {
        return IDCU_ERR_CONFIG_PARSE;
    }

    *out = (char*)malloc(len + 1);
    if (!*out) {
        return IDCU_ERR_NO_MEMORY;
    }

    char* dst = *out;
    parser->pos = start;
    while (parser->pos < parser->end && *parser->pos != '"') {
        if (*parser->pos == '\\') {
            parser->pos++;
            switch (*parser->pos) {
            case 'n': *dst++ = '\n'; break;
            case 't': *dst++ = '\t'; break;
            case 'r': *dst++ = '\r'; break;
            case '\\': *dst++ = '\\'; break;
            case '"': *dst++ = '"'; break;
            default: *dst++ = *parser->pos; break;
            }
        } else {
            *dst++ = *parser->pos;
        }
        parser->pos++;
    }
    *dst = '\0';

    parser->pos++;
    parser->column++;
    return IDCU_ERR_OK;
}

static int json_parse_number(idcu_JsonParser* parser, idcu_JsonValue* value)
{
    const char* start = parser->pos;
    int is_double = 0;

    if (*parser->pos == '-') {
        parser->pos++;
        parser->column++;
    }

    while (parser->pos < parser->end && isdigit(*parser->pos)) {
        parser->pos++;
        parser->column++;
    }

    if (parser->pos < parser->end && *parser->pos == '.') {
        is_double = 1;
        parser->pos++;
        parser->column++;
        while (parser->pos < parser->end && isdigit(*parser->pos)) {
            parser->pos++;
            parser->column++;
        }
    }

    if (parser->pos < parser->end && (*parser->pos == 'e' || *parser->pos == 'E')) {
        is_double = 1;
        parser->pos++;
        parser->column++;
        if (parser->pos < parser->end && (*parser->pos == '+' || *parser->pos == '-')) {
            parser->pos++;
            parser->column++;
        }
        while (parser->pos < parser->end && isdigit(*parser->pos)) {
            parser->pos++;
            parser->column++;
        }
    }

    if (is_double) {
        value->type = IDCU_JSON_TYPE_DOUBLE;
        value->data.double_val = atof(start);
    } else {
        value->type = IDCU_JSON_TYPE_INT;
        value->data.int_val = atoll(start);
    }

    return IDCU_ERR_OK;
}

static int json_parse_literal(idcu_JsonParser* parser, idcu_JsonValue* value)
{
    if (parser->pos + 3 < parser->end &&
        strncmp(parser->pos, "true", 4) == 0) {
        value->type = IDCU_JSON_TYPE_BOOL;
        value->data.bool_val = 1;
        parser->pos += 4;
        parser->column += 4;
        return IDCU_ERR_OK;
    }

    if (parser->pos + 4 < parser->end &&
        strncmp(parser->pos, "false", 5) == 0) {
        value->type = IDCU_JSON_TYPE_BOOL;
        value->data.bool_val = 0;
        parser->pos += 5;
        parser->column += 5;
        return IDCU_ERR_OK;
    }

    if (parser->pos + 3 < parser->end &&
        strncmp(parser->pos, "null", 4) == 0) {
        value->type = IDCU_JSON_TYPE_NULL;
        parser->pos += 4;
        parser->column += 4;
        return IDCU_ERR_OK;
    }

    return IDCU_ERR_CONFIG_PARSE;
}

static int json_parse_value(idcu_JsonParser* parser, idcu_JsonValue* value);

static int json_parse_array(idcu_JsonParser* parser, idcu_JsonValue* value)
{
    parser->pos++;
    parser->column++;

    value->type = IDCU_JSON_TYPE_ARRAY;
    value->data.array_val = (idcu_JsonArray*)malloc(sizeof(idcu_JsonArray));
    if (!value->data.array_val) {
        return IDCU_ERR_NO_MEMORY;
    }
    value->data.array_val->count = 0;
    value->data.array_val->capacity = JSON_INITIAL_CAPACITY;
    value->data.array_val->elements = (idcu_JsonValue*)malloc(
        JSON_INITIAL_CAPACITY * sizeof(idcu_JsonValue));
    if (!value->data.array_val->elements) {
        free(value->data.array_val);
        return IDCU_ERR_NO_MEMORY;
    }

    json_skip_whitespace(parser);

    if (parser->pos < parser->end && *parser->pos == ']') {
        parser->pos++;
        parser->column++;
        return IDCU_ERR_OK;
    }

    while (parser->pos < parser->end) {
        if (value->data.array_val->count >= value->data.array_val->capacity) {
            size_t new_cap = value->data.array_val->capacity * JSON_GROWTH_FACTOR;
            idcu_JsonValue* new_elems = (idcu_JsonValue*)realloc(
                value->data.array_val->elements, new_cap * sizeof(idcu_JsonValue));
            if (!new_elems) {
                return IDCU_ERR_NO_MEMORY;
            }
            value->data.array_val->elements = new_elems;
            value->data.array_val->capacity = new_cap;
        }

        int ret = json_parse_value(parser, &value->data.array_val->elements[value->data.array_val->count]);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
        value->data.array_val->count++;

        json_skip_whitespace(parser);

        if (parser->pos >= parser->end) {
            break;
        }

        if (*parser->pos == ']') {
            parser->pos++;
            parser->column++;
            break;
        }

        if (*parser->pos != ',') {
            return IDCU_ERR_CONFIG_PARSE;
        }
        parser->pos++;
        parser->column++;
        json_skip_whitespace(parser);
    }

    return IDCU_ERR_OK;
}

static int json_parse_object(idcu_JsonParser* parser, idcu_JsonValue* value)
{
    parser->pos++;
    parser->column++;

    value->type = IDCU_JSON_TYPE_OBJECT;
    value->data.object_val = (idcu_JsonObject*)malloc(sizeof(idcu_JsonObject));
    if (!value->data.object_val) {
        return IDCU_ERR_NO_MEMORY;
    }
    value->data.object_val->count = 0;
    value->data.object_val->capacity = JSON_INITIAL_CAPACITY;
    value->data.object_val->keys = (char**)malloc(JSON_INITIAL_CAPACITY * sizeof(char*));
    value->data.object_val->values = (idcu_JsonValue*)malloc(
        JSON_INITIAL_CAPACITY * sizeof(idcu_JsonValue));
    if (!value->data.object_val->keys || !value->data.object_val->values) {
        if (value->data.object_val->keys) free(value->data.object_val->keys);
        if (value->data.object_val->values) free(value->data.object_val->values);
        free(value->data.object_val);
        return IDCU_ERR_NO_MEMORY;
    }

    json_skip_whitespace(parser);

    if (parser->pos < parser->end && *parser->pos == '}') {
        parser->pos++;
        parser->column++;
        return IDCU_ERR_OK;
    }

    while (parser->pos < parser->end) {
        if (value->data.object_val->count >= value->data.object_val->capacity) {
            size_t new_cap = value->data.object_val->capacity * JSON_GROWTH_FACTOR;
            char** new_keys = (char**)realloc(value->data.object_val->keys, new_cap * sizeof(char*));
            idcu_JsonValue* new_vals = (idcu_JsonValue*)realloc(
                value->data.object_val->values, new_cap * sizeof(idcu_JsonValue));
            if (!new_keys || !new_vals) {
                return IDCU_ERR_NO_MEMORY;
            }
            value->data.object_val->keys = new_keys;
            value->data.object_val->values = new_vals;
            value->data.object_val->capacity = new_cap;
        }

        if (*parser->pos != '"') {
            return IDCU_ERR_CONFIG_PARSE;
        }

        char* key;
        int ret = json_parse_string(parser, &key);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
        value->data.object_val->keys[value->data.object_val->count] = key;

        json_skip_whitespace(parser);

        if (parser->pos >= parser->end || *parser->pos != ':') {
            return IDCU_ERR_CONFIG_PARSE;
        }
        parser->pos++;
        parser->column++;
        json_skip_whitespace(parser);

        ret = json_parse_value(parser, &value->data.object_val->values[value->data.object_val->count]);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
        value->data.object_val->count++;

        json_skip_whitespace(parser);

        if (parser->pos >= parser->end) {
            break;
        }

        if (*parser->pos == '}') {
            parser->pos++;
            parser->column++;
            break;
        }

        if (*parser->pos != ',') {
            return IDCU_ERR_CONFIG_PARSE;
        }
        parser->pos++;
        parser->column++;
        json_skip_whitespace(parser);
    }

    return IDCU_ERR_OK;
}

static int json_parse_value(idcu_JsonParser* parser, idcu_JsonValue* value)
{
    json_skip_whitespace(parser);

    if (parser->pos >= parser->end) {
        return IDCU_ERR_CONFIG_PARSE;
    }

    char c = *parser->pos;

    if (c == '"') {
        return json_parse_string(parser, &value->data.string_val);
    } else if (c == '-' || isdigit(c)) {
        return json_parse_number(parser, value);
    } else if (c == 't' || c == 'f' || c == 'n') {
        return json_parse_literal(parser, value);
    } else if (c == '[') {
        return json_parse_array(parser, value);
    } else if (c == '{') {
        return json_parse_object(parser, value);
    }

    return IDCU_ERR_CONFIG_PARSE;
}

int idcu_json_parse(const char* json_str, idcu_JsonValue* result)
{
    if (!json_str || !result) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_JsonParser parser;
    parser.pos = json_str;
    parser.end = json_str + strlen(json_str);
    parser.line = 1;
    parser.column = 1;

    memset(result, 0, sizeof(idcu_JsonValue));

    return json_parse_value(&parser, result);
}

int idcu_json_parse_file(const char* file_path, idcu_JsonValue* result)
{
    if (!file_path || !result) {
        return IDCU_ERR_INVALID_PARAM;
    }

    FILE* fp = fopen(file_path, "rb");
    if (!fp) {
        return IDCU_ERR_FILE_OPEN;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(fp);
        return IDCU_ERR_NO_MEMORY;
    }

    fread(buffer, 1, size, fp);
    buffer[size] = '\0';
    fclose(fp);

    int ret = idcu_json_parse(buffer, result);
    free(buffer);

    return ret;
}

static void json_free_value(idcu_JsonValue* value)
{
    if (!value) return;

    switch (value->type) {
    case IDCU_JSON_TYPE_STRING:
        if (value->data.string_val) {
            free(value->data.string_val);
        }
        break;
    case IDCU_JSON_TYPE_ARRAY:
        if (value->data.array_val) {
            for (size_t i = 0; i < value->data.array_val->count; i++) {
                json_free_value(&value->data.array_val->elements[i]);
            }
            free(value->data.array_val->elements);
            free(value->data.array_val);
        }
        break;
    case IDCU_JSON_TYPE_OBJECT:
        if (value->data.object_val) {
            for (size_t i = 0; i < value->data.object_val->count; i++) {
                free(value->data.object_val->keys[i]);
                json_free_value(&value->data.object_val->values[i]);
            }
            free(value->data.object_val->keys);
            free(value->data.object_val->values);
            free(value->data.object_val);
        }
        break;
    default:
        break;
    }
}

void idcu_json_free(idcu_JsonValue* value)
{
    if (!value) return;
    json_free_value(value);
    memset(value, 0, sizeof(idcu_JsonValue));
}

idcu_JsonType idcu_json_get_type(const idcu_JsonValue* value)
{
    if (!value) return IDCU_JSON_TYPE_NULL;
    return value->type;
}

int idcu_json_get_bool(const idcu_JsonValue* value, int* out)
{
    if (!value || !out || value->type != IDCU_JSON_TYPE_BOOL) {
        return IDCU_ERR_INVALID_PARAM;
    }
    *out = value->data.bool_val;
    return IDCU_ERR_OK;
}

int idcu_json_get_int(const idcu_JsonValue* value, int64_t* out)
{
    if (!value || !out) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (value->type == IDCU_JSON_TYPE_INT) {
        *out = value->data.int_val;
        return IDCU_ERR_OK;
    }
    if (value->type == IDCU_JSON_TYPE_DOUBLE) {
        *out = (int64_t)value->data.double_val;
        return IDCU_ERR_OK;
    }
    return IDCU_ERR_INVALID_PARAM;
}

int idcu_json_get_double(const idcu_JsonValue* value, double* out)
{
    if (!value || !out) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (value->type == IDCU_JSON_TYPE_DOUBLE) {
        *out = value->data.double_val;
        return IDCU_ERR_OK;
    }
    if (value->type == IDCU_JSON_TYPE_INT) {
        *out = (double)value->data.int_val;
        return IDCU_ERR_OK;
    }
    return IDCU_ERR_INVALID_PARAM;
}

int idcu_json_get_string(const idcu_JsonValue* value, const char** out)
{
    if (!value || !out || value->type != IDCU_JSON_TYPE_STRING) {
        return IDCU_ERR_INVALID_PARAM;
    }
    *out = value->data.string_val;
    return IDCU_ERR_OK;
}

size_t idcu_json_array_size(const idcu_JsonArray* array)
{
    if (!array) return 0;
    return array->count;
}

idcu_JsonValue* idcu_json_array_get(const idcu_JsonArray* array, size_t index)
{
    if (!array || index >= array->count) return NULL;
    return &array->elements[index];
}

idcu_JsonValue* idcu_json_object_get(const idcu_JsonObject* object, const char* key)
{
    if (!object || !key) return NULL;
    for (size_t i = 0; i < object->count; i++) {
        if (strcmp(object->keys[i], key) == 0) {
            return &object->values[i];
        }
    }
    return NULL;
}

int idcu_json_object_has(const idcu_JsonObject* object, const char* key)
{
    return idcu_json_object_get(object, key) != NULL;
}

static int json_serialize_value(const idcu_JsonValue* value, char* buffer, size_t* offset, size_t buffer_size)
{
    if (!value || !buffer) return IDCU_ERR_INVALID_PARAM;

    switch (value->type) {
    case IDCU_JSON_TYPE_NULL:
        if (*offset + 5 > buffer_size) return IDCU_ERR_BUFFER_TOO_SMALL;
        strcpy(buffer + *offset, "null");
        *offset += 4;
        break;
    case IDCU_JSON_TYPE_BOOL:
        if (*offset + 6 > buffer_size) return IDCU_ERR_BUFFER_TOO_SMALL;
        strcpy(buffer + *offset, value->data.bool_val ? "true" : "false");
        *offset += value->data.bool_val ? 4 : 5;
        break;
    case IDCU_JSON_TYPE_INT:
        if (*offset + 32 > buffer_size) return IDCU_ERR_BUFFER_TOO_SMALL;
        sprintf(buffer + *offset, "%" PRId64, value->data.int_val);
        *offset += strlen(buffer + *offset);
        break;
    case IDCU_JSON_TYPE_DOUBLE:
        if (*offset + 64 > buffer_size) return IDCU_ERR_BUFFER_TOO_SMALL;
        sprintf(buffer + *offset, "%g", value->data.double_val);
        *offset += strlen(buffer + *offset);
        break;
    case IDCU_JSON_TYPE_STRING: {
        const char* str = value->data.string_val;
        size_t len = strlen(str);
        size_t escaped_len = 0;
        for (size_t i = 0; i < len; i++) {
            char c = str[i];
            if (c == '"' || c == '\\' || c == '\n' || c == '\r' || c == '\t') {
                escaped_len += 2;
            } else {
                escaped_len += 1;
            }
        }
        if (*offset + escaped_len + 3 > buffer_size) return IDCU_ERR_BUFFER_TOO_SMALL;
        buffer[(*offset)++] = '"';
        for (size_t i = 0; i < len; i++) {
            char c = str[i];
            switch (c) {
            case '"': buffer[(*offset)++] = '\\'; buffer[(*offset)++] = '"'; break;
            case '\\': buffer[(*offset)++] = '\\'; buffer[(*offset)++] = '\\'; break;
            case '\n': buffer[(*offset)++] = '\\'; buffer[(*offset)++] = 'n'; break;
            case '\r': buffer[(*offset)++] = '\\'; buffer[(*offset)++] = 'r'; break;
            case '\t': buffer[(*offset)++] = '\\'; buffer[(*offset)++] = 't'; break;
            default: buffer[(*offset)++] = c; break;
            }
        }
        buffer[(*offset)++] = '"';
        break;
    }
    case IDCU_JSON_TYPE_ARRAY: {
        if (*offset + 2 > buffer_size) return IDCU_ERR_BUFFER_TOO_SMALL;
        buffer[(*offset)++] = '[';
        for (size_t i = 0; i < value->data.array_val->count; i++) {
            if (i > 0) {
                if (*offset + 2 > buffer_size) return IDCU_ERR_BUFFER_TOO_SMALL;
                buffer[(*offset)++] = ',';
            }
            int ret = json_serialize_value(&value->data.array_val->elements[i], buffer, offset, buffer_size);
            if (ret != IDCU_ERR_OK) return ret;
        }
        if (*offset + 2 > buffer_size) return IDCU_ERR_BUFFER_TOO_SMALL;
        buffer[(*offset)++] = ']';
        break;
    }
    case IDCU_JSON_TYPE_OBJECT: {
        if (*offset + 2 > buffer_size) return IDCU_ERR_BUFFER_TOO_SMALL;
        buffer[(*offset)++] = '{';
        for (size_t i = 0; i < value->data.object_val->count; i++) {
            if (i > 0) {
                if (*offset + 2 > buffer_size) return IDCU_ERR_BUFFER_TOO_SMALL;
                buffer[(*offset)++] = ',';
            }
            if (*offset + 3 > buffer_size) return IDCU_ERR_BUFFER_TOO_SMALL;
            buffer[(*offset)++] = '"';
            strcpy(buffer + *offset, value->data.object_val->keys[i]);
            *offset += strlen(value->data.object_val->keys[i]);
            buffer[(*offset)++] = '"';
            buffer[(*offset)++] = ':';
            int ret = json_serialize_value(&value->data.object_val->values[i], buffer, offset, buffer_size);
            if (ret != IDCU_ERR_OK) return ret;
        }
        if (*offset + 2 > buffer_size) return IDCU_ERR_BUFFER_TOO_SMALL;
        buffer[(*offset)++] = '}';
        break;
    }
    }
    return IDCU_ERR_OK;
}

int idcu_json_to_string(const idcu_JsonValue* value, char* buffer, size_t buffer_size)
{
    if (!value || !buffer || buffer_size == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }
    size_t offset = 0;
    int ret = json_serialize_value(value, buffer, &offset, buffer_size);
    if (ret == IDCU_ERR_OK) {
        if (offset >= buffer_size) {
            return IDCU_ERR_BUFFER_TOO_SMALL;
        }
        buffer[offset] = '\0';
    }
    return ret;
}
```

### 4. 创建 CMakeLists.txt

创建 `libs/idcu-json/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-json VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-json STATIC
    src/idcu/json/json.c
)

target_include_directories(idcu-json PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-json PRIVATE
    idcu::common
)

add_library(idcu::json ALIAS idcu-json)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 5. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-json/module.yaml`：

```yaml
name: idcu-json
version: 1.0.0
description: JSON parsing and serialization library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common

build:
  type: cmake
  targets:
    - idcu-json

headers:
  - idcu/json/json.h

features:
  - parse: JSON parsing from string or file
  - serialize: JSON serialization to string or file
  - types: Support all JSON types (null, bool, number, string, array, object)
  - type_safe: Type-safe accessor functions
  - memory: Proper memory management

testing:
  enabled: true
  framework: internal
```

### 6. 创建 README.md

创建 `libs/idcu-json/README.md`：

```markdown
# idcu-json

IDCU Agent 的 JSON 解析与序列化库。

## 功能特性

- **JSON 解析**: 从字符串或文件解析 JSON
- **JSON 序列化**: 序列化为字符串或保存到文件
- **完整类型支持**: null, bool, number, string, array, object
- **类型安全访问**: 类型安全的访问函数
- **内存管理**: 自动管理内存

## 快速开始

### 解析 JSON

```c
#include "idcu/json/json.h"

idcu_JsonValue root;
int ret = idcu_json_parse("{\"name\": \"test\", \"value\": 42}", &root);
if (ret != IDCU_ERR_OK) {
    printf("Parse failed: %s\n", idcu_err_to_str(ret));
    return -1;
}

idcu_JsonObject* obj = root.data.object_val;
idcu_JsonValue* name_val = idcu_json_object_get(obj, "name");
idcu_JsonValue* value_val = idcu_json_object_get(obj, "value");

const char* name;
idcu_json_get_string(name_val, &name);
printf("Name: %s\n", name);

int64_t value;
idcu_json_get_int(value_val, &value);
printf("Value: %" PRId64 "\n", value);

idcu_json_free(&root);
```

### 序列化 JSON

```c
idcu_JsonValue* root = idcu_json_create_object();
idcu_json_object_set(root->data.object_val, "name", idcu_json_create_string("test"));
idcu_json_object_set(root->data.object_val, "value", idcu_json_create_int(42));

char buffer[1024];
idcu_json_to_string(root, buffer, sizeof(buffer));
printf("JSON: %s\n", buffer);

idcu_json_free(root);
```

## API 文档

详见 [include/idcu/json/json.h](include/idcu/json/json.h)
```

## 验证检查清单

- [ ] JSON 头文件已创建
- [ ] JSON 实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以解析简单的 JSON 字符串
- [ ] 可以访问 JSON 对象的字段
- [ ] 可以序列化 JSON 值

## Git 提交

```bash
git add libs/idcu-json/
git commit -m "feat: add idcu-json library

- Add JSON parser from string/file
- Add JSON serializer to string/file
- Add support for all JSON types
- Add type-safe accessor functions
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 解析失败 | JSON 格式错误 | 验证 JSON 格式是否正确 |
| 内存泄漏 | 忘记调用 idcu_json_free | 确保所有解析的 JSON 值都被释放 |
| 缓冲区溢出 | 序列化缓冲区太小 | 使用更大的缓冲区或 idcu_json_to_string_alloc |
