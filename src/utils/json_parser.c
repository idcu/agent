#include "utils/json_parser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

typedef struct {
    const char* pos;
    const char* end;
} idcu_JsonParser;

static void skip_whitespace(idcu_JsonParser* p)
{
    while (p->pos < p->end && isspace((unsigned char)*p->pos)) {
        p->pos++;
    }
}

static int parse_null(idcu_JsonParser* p, idcu_JsonValue* out)
{
    if (p->pos + 3 < p->end && strncmp(p->pos, "null", 4) == 0) {
        p->pos += 4;
        out->type = IDCU_JSON_TYPE_NULL;
        return IDCU_ERR_SUCCESS;
    }
    return IDCU_ERR_CONFIG_PARSE;
}

static int parse_bool(idcu_JsonParser* p, idcu_JsonValue* out)
{
    if (p->pos + 3 < p->end && strncmp(p->pos, "true", 4) == 0) {
        p->pos += 4;
        out->type = IDCU_JSON_TYPE_BOOL;
        out->data.bool_val = 1;
        return IDCU_ERR_SUCCESS;
    }
    if (p->pos + 4 < p->end && strncmp(p->pos, "false", 5) == 0) {
        p->pos += 5;
        out->type = IDCU_JSON_TYPE_BOOL;
        out->data.bool_val = 0;
        return IDCU_ERR_SUCCESS;
    }
    return IDCU_ERR_CONFIG_PARSE;
}

static int parse_number(idcu_JsonParser* p, idcu_JsonValue* out)
{
    const char* start = p->pos;
    int is_double = 0;
    if (*p->pos == '-' || *p->pos == '+') {
        p->pos++;
    }
    while (p->pos < p->end && isdigit((unsigned char)*p->pos)) {
        p->pos++;
    }
    if (p->pos < p->end && *p->pos == '.') {
        is_double = 1;
        p->pos++;
        while (p->pos < p->end && isdigit((unsigned char)*p->pos)) {
            p->pos++;
        }
    }
    if (p->pos < p->end && (*p->pos == 'e' || *p->pos == 'E')) {
        is_double = 1;
        p->pos++;
        if (p->pos < p->end && (*p->pos == '+' || *p->pos == '-')) {
            p->pos++;
        }
        while (p->pos < p->end && isdigit((unsigned char)*p->pos)) {
            p->pos++;
        }
    }
    if (is_double) {
        out->type = IDCU_JSON_TYPE_DOUBLE;
        out->data.double_val = strtod(start, NULL);
    } else {
        out->type = IDCU_JSON_TYPE_INT;
        out->data.int_val = strtoll(start, NULL, 10);
    }
    return IDCU_ERR_SUCCESS;
}

static int parse_string(idcu_JsonParser* p, idcu_JsonValue* out)
{
    if (*p->pos != '"') {
        return IDCU_ERR_CONFIG_PARSE;
    }
    p->pos++;
    const char* start = p->pos;
    size_t len = 0;
    while (p->pos < p->end && *p->pos != '"') {
        if (*p->pos == '\\') {
            p->pos++;
            if (p->pos >= p->end) {
                return IDCU_ERR_CONFIG_PARSE;
            }
        }
        p->pos++;
        len++;
    }
    if (p->pos >= p->end) {
        return IDCU_ERR_CONFIG_PARSE;
    }
    p->pos++;
    char* str = (char*)malloc(len + 1);
    if (!str) {
        return IDCU_ERR_NO_MEMORY;
    }
    const char* src = start;
    char* dst = str;
    while (src < p->pos - 1) {
        if (*src == '\\') {
            src++;
            switch (*src) {
                case 'n': *dst++ = '\n'; break;
                case 'r': *dst++ = '\r'; break;
                case 't': *dst++ = '\t'; break;
                case 'b': *dst++ = '\b'; break;
                case 'f': *dst++ = '\f'; break;
                default: *dst++ = *src; break;
            }
        } else {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
    out->type = IDCU_JSON_TYPE_STRING;
    out->data.string_val = str;
    return IDCU_ERR_SUCCESS;
}

static int parse_value(idcu_JsonParser* p, idcu_JsonValue* out);

static int parse_array(idcu_JsonParser* p, idcu_JsonValue* out)
{
    if (*p->pos != '[') {
        return IDCU_ERR_CONFIG_PARSE;
    }
    p->pos++;
    skip_whitespace(p);
    idcu_JsonArray* array = (idcu_JsonArray*)malloc(sizeof(idcu_JsonArray));
    if (!array) {
        return IDCU_ERR_NO_MEMORY;
    }
    array->count = 0;
    array->capacity = 8;
    array->elements = (idcu_JsonValue*)malloc(array->capacity * sizeof(idcu_JsonValue));
    if (!array->elements) {
        free(array);
        return IDCU_ERR_NO_MEMORY;
    }
    int first = 1;
    while (p->pos < p->end && *p->pos != ']') {
        if (!first) {
            if (*p->pos != ',') {
                free(array->elements);
                free(array);
                return IDCU_ERR_CONFIG_PARSE;
            }
            p->pos++;
            skip_whitespace(p);
        }
        first = 0;
        if (array->count >= array->capacity) {
            size_t new_cap = array->capacity * 2;
            idcu_JsonValue* new_elems = (idcu_JsonValue*)realloc(array->elements, new_cap * sizeof(idcu_JsonValue));
            if (!new_elems) {
                for (size_t i = 0; i < array->count; i++) {
                    idcu_json_free(&array->elements[i]);
                }
                free(array->elements);
                free(array);
                return IDCU_ERR_NO_MEMORY;
            }
            array->elements = new_elems;
            array->capacity = new_cap;
        }
        int ret = parse_value(p, &array->elements[array->count]);
        if (ret != IDCU_ERR_SUCCESS) {
            for (size_t i = 0; i <= array->count; i++) {
                idcu_json_free(&array->elements[i]);
            }
            free(array->elements);
            free(array);
            return ret;
        }
        array->count++;
        skip_whitespace(p);
    }
    if (p->pos >= p->end) {
        for (size_t i = 0; i < array->count; i++) {
            idcu_json_free(&array->elements[i]);
        }
        free(array->elements);
        free(array);
        return IDCU_ERR_CONFIG_PARSE;
    }
    p->pos++;
    out->type = IDCU_JSON_TYPE_ARRAY;
    out->data.array_val = array;
    return IDCU_ERR_SUCCESS;
}

static int parse_object(idcu_JsonParser* p, idcu_JsonValue* out)
{
    if (*p->pos != '{') {
        return IDCU_ERR_CONFIG_PARSE;
    }
    p->pos++;
    skip_whitespace(p);
    idcu_JsonObject* obj = (idcu_JsonObject*)malloc(sizeof(idcu_JsonObject));
    if (!obj) {
        return IDCU_ERR_NO_MEMORY;
    }
    obj->count = 0;
    obj->capacity = 8;
    obj->keys = (char**)malloc(obj->capacity * sizeof(char*));
    obj->values = (idcu_JsonValue*)malloc(obj->capacity * sizeof(idcu_JsonValue));
    if (!obj->keys || !obj->values) {
        free(obj->keys);
        free(obj->values);
        free(obj);
        return IDCU_ERR_NO_MEMORY;
    }
    int first = 1;
    while (p->pos < p->end && *p->pos != '}') {
        if (!first) {
            if (*p->pos != ',') {
                free(obj->keys);
                free(obj->values);
                free(obj);
                return IDCU_ERR_CONFIG_PARSE;
            }
            p->pos++;
            skip_whitespace(p);
        }
        first = 0;
        idcu_JsonValue key_val;
        int ret = parse_string(p, &key_val);
        if (ret != IDCU_ERR_SUCCESS) {
            for (size_t i = 0; i < obj->count; i++) {
                free(obj->keys[i]);
                idcu_json_free(&obj->values[i]);
            }
            free(obj->keys);
            free(obj->values);
            free(obj);
            return ret;
        }
        skip_whitespace(p);
        if (*p->pos != ':') {
            idcu_json_free(&key_val);
            for (size_t i = 0; i < obj->count; i++) {
                free(obj->keys[i]);
                idcu_json_free(&obj->values[i]);
            }
            free(obj->keys);
            free(obj->values);
            free(obj);
            return IDCU_ERR_CONFIG_PARSE;
        }
        p->pos++;
        skip_whitespace(p);
        if (obj->count >= obj->capacity) {
            size_t new_cap = obj->capacity * 2;
            char** new_keys = (char**)realloc(obj->keys, new_cap * sizeof(char*));
            idcu_JsonValue* new_vals = (idcu_JsonValue*)realloc(obj->values, new_cap * sizeof(idcu_JsonValue));
            if (!new_keys || !new_vals) {
                idcu_json_free(&key_val);
                for (size_t i = 0; i < obj->count; i++) {
                    free(obj->keys[i]);
                    idcu_json_free(&obj->values[i]);
                }
                free(obj->keys);
                free(obj->values);
                free(obj);
                return IDCU_ERR_NO_MEMORY;
            }
            obj->keys = new_keys;
            obj->values = new_vals;
            obj->capacity = new_cap;
        }
        obj->keys[obj->count] = key_val.data.string_val;
        ret = parse_value(p, &obj->values[obj->count]);
        if (ret != IDCU_ERR_SUCCESS) {
            free(obj->keys[obj->count]);
            for (size_t i = 0; i < obj->count; i++) {
                free(obj->keys[i]);
                idcu_json_free(&obj->values[i]);
            }
            free(obj->keys);
            free(obj->values);
            free(obj);
            return ret;
        }
        obj->count++;
        skip_whitespace(p);
    }
    if (p->pos >= p->end) {
        for (size_t i = 0; i < obj->count; i++) {
            free(obj->keys[i]);
            idcu_json_free(&obj->values[i]);
        }
        free(obj->keys);
        free(obj->values);
        free(obj);
        return IDCU_ERR_CONFIG_PARSE;
    }
    p->pos++;
    out->type = IDCU_JSON_TYPE_OBJECT;
    out->data.object_val = obj;
    return IDCU_ERR_SUCCESS;
}

static int parse_value(idcu_JsonParser* p, idcu_JsonValue* out)
{
    skip_whitespace(p);
    if (p->pos >= p->end) {
        return IDCU_ERR_CONFIG_PARSE;
    }
    switch (*p->pos) {
        case 'n':
            return parse_null(p, out);
        case 't':
        case 'f':
            return parse_bool(p, out);
        case '"':
            return parse_string(p, out);
        case '[':
            return parse_array(p, out);
        case '{':
            return parse_object(p, out);
        case '-':
        case '+':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return parse_number(p, out);
        default:
            return IDCU_ERR_CONFIG_PARSE;
    }
}

int idcu_json_parse(const char* json_str, idcu_JsonValue* result)
{
    if (!json_str || !result) {
        return IDCU_ERR_INVALID_PARAM;
    }
    idcu_JsonParser p;
    p.pos = json_str;
    p.end = json_str + strlen(json_str);
    memset(result, 0, sizeof(idcu_JsonValue));
    return parse_value(&p, result);
}

void idcu_json_free(idcu_JsonValue* value)
{
    if (!value) {
        return;
    }
    switch (value->type) {
        case IDCU_JSON_TYPE_STRING:
            free(value->data.string_val);
            break;
        case IDCU_JSON_TYPE_ARRAY:
            if (value->data.array_val) {
                for (size_t i = 0; i < value->data.array_val->count; i++) {
                    idcu_json_free(&value->data.array_val->elements[i]);
                }
                free(value->data.array_val->elements);
                free(value->data.array_val);
            }
            break;
        case IDCU_JSON_TYPE_OBJECT:
            if (value->data.object_val) {
                for (size_t i = 0; i < value->data.object_val->count; i++) {
                    free(value->data.object_val->keys[i]);
                    idcu_json_free(&value->data.object_val->values[i]);
                }
                free(value->data.object_val->keys);
                free(value->data.object_val->values);
                free(value->data.object_val);
            }
            break;
        default:
            break;
    }
    memset(value, 0, sizeof(idcu_JsonValue));
}

idcu_JsonType idcu_json_get_type(const idcu_JsonValue* value)
{
    if (!value) {
        return IDCU_JSON_TYPE_NULL;
    }
    return value->type;
}

int idcu_json_get_bool(const idcu_JsonValue* value, int* out)
{
    if (!value || !out || value->type != IDCU_JSON_TYPE_BOOL) {
        return IDCU_ERR_INVALID_PARAM;
    }
    *out = value->data.bool_val;
    return IDCU_ERR_SUCCESS;
}

int idcu_json_get_int(const idcu_JsonValue* value, int64_t* out)
{
    if (!value || !out) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (value->type == IDCU_JSON_TYPE_INT) {
        *out = value->data.int_val;
        return IDCU_ERR_SUCCESS;
    }
    if (value->type == IDCU_JSON_TYPE_DOUBLE) {
        *out = (int64_t)value->data.double_val;
        return IDCU_ERR_SUCCESS;
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
        return IDCU_ERR_SUCCESS;
    }
    if (value->type == IDCU_JSON_TYPE_INT) {
        *out = (double)value->data.int_val;
        return IDCU_ERR_SUCCESS;
    }
    return IDCU_ERR_INVALID_PARAM;
}

int idcu_json_get_string(const idcu_JsonValue* value, const char** out)
{
    if (!value || !out || value->type != IDCU_JSON_TYPE_STRING) {
        return IDCU_ERR_INVALID_PARAM;
    }
    *out = value->data.string_val;
    return IDCU_ERR_SUCCESS;
}

size_t idcu_json_array_size(const idcu_JsonArray* array)
{
    if (!array) {
        return 0;
    }
    return array->count;
}

idcu_JsonValue* idcu_json_array_get(const idcu_JsonArray* array, size_t index)
{
    if (!array || index >= array->count) {
        return NULL;
    }
    return &array->elements[index];
}

idcu_JsonValue* idcu_json_object_get(const idcu_JsonObject* object, const char* key)
{
    if (!object || !key) {
        return NULL;
    }
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

static int json_to_string_internal(const idcu_JsonValue* value, char* buffer, size_t* offset, size_t buffer_size)
{
    if (!value || !buffer || !offset) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int len = 0;
    switch (value->type) {
        case IDCU_JSON_TYPE_NULL:
            len = snprintf(buffer + *offset, buffer_size - *offset, "null");
            break;
        case IDCU_JSON_TYPE_BOOL:
            len = snprintf(buffer + *offset, buffer_size - *offset, 
                          value->data.bool_val ? "true" : "false");
            break;
        case IDCU_JSON_TYPE_INT:
            len = snprintf(buffer + *offset, buffer_size - *offset, "%lld", 
                          (long long)value->data.int_val);
            break;
        case IDCU_JSON_TYPE_DOUBLE:
            len = snprintf(buffer + *offset, buffer_size - *offset, "%f", 
                          value->data.double_val);
            break;
        case IDCU_JSON_TYPE_STRING:
            len = snprintf(buffer + *offset, buffer_size - *offset, "\"%s\"", 
                          value->data.string_val);
            break;
        case IDCU_JSON_TYPE_ARRAY:
            len = snprintf(buffer + *offset, buffer_size - *offset, "[");
            if (len > 0) *offset += len;
            for (size_t i = 0; i < value->data.array_val->count; i++) {
                if (i > 0) {
                    len = snprintf(buffer + *offset, buffer_size - *offset, ",");
                    if (len > 0) *offset += len;
                }
                int ret = json_to_string_internal(&value->data.array_val->elements[i], 
                                                  buffer, offset, buffer_size);
                if (ret != IDCU_ERR_SUCCESS) return ret;
            }
            len = snprintf(buffer + *offset, buffer_size - *offset, "]");
            break;
        case IDCU_JSON_TYPE_OBJECT:
            len = snprintf(buffer + *offset, buffer_size - *offset, "{");
            if (len > 0) *offset += len;
            for (size_t i = 0; i < value->data.object_val->count; i++) {
                if (i > 0) {
                    len = snprintf(buffer + *offset, buffer_size - *offset, ",");
                    if (len > 0) *offset += len;
                }
                len = snprintf(buffer + *offset, buffer_size - *offset, "\"%s\":", 
                              value->data.object_val->keys[i]);
                if (len > 0) *offset += len;
                int ret = json_to_string_internal(&value->data.object_val->values[i], 
                                                  buffer, offset, buffer_size);
                if (ret != IDCU_ERR_SUCCESS) return ret;
            }
            len = snprintf(buffer + *offset, buffer_size - *offset, "}");
            break;
    }
    if (len < 0) {
        return IDCU_ERR_GENERAL;
    }
    *offset += len;
    return IDCU_ERR_SUCCESS;
}

int idcu_json_to_string(const idcu_JsonValue* value, char* buffer, size_t buffer_size)
{
    if (!value || !buffer || buffer_size == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }
    size_t offset = 0;
    return json_to_string_internal(value, buffer, &offset, buffer_size);
}

char* idcu_json_to_string_alloc(const idcu_JsonValue* value)
{
    if (!value) {
        return NULL;
    }
    size_t size = 1024;
    char* buffer = NULL;
    while (1) {
        buffer = (char*)malloc(size);
        if (!buffer) {
            return NULL;
        }
        int ret = idcu_json_to_string(value, buffer, size);
        if (ret == IDCU_ERR_SUCCESS) {
            return buffer;
        }
        free(buffer);
        size *= 2;
        if (size > 1024 * 1024) {
            return NULL;
        }
    }
}
