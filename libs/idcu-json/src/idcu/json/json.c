#include <idcu/json/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#define JSON_INIT_CAPACITY 8
#define JSON_CAPACITY_GROWTH_FACTOR 2

typedef struct {
    const char* pos;
    size_t line;
    size_t col;
} json_parser_t;

static void parser_skip_whitespace(json_parser_t* parser) {
    while (*parser->pos && isspace((unsigned char)*parser->pos)) {
        if (*parser->pos == '\n') {
            parser->line++;
            parser->col = 1;
        } else {
            parser->col++;
        }
        parser->pos++;
    }
}

static int parse_null(json_parser_t* parser, idcu_JsonValue* result) {
    if (strncmp(parser->pos, "null", 4) != 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    result->type = IDCU_JSON_TYPE_NULL;
    parser->pos += 4;
    parser->col += 4;
    return IDCU_ERR_OK;
}

static int parse_bool(json_parser_t* parser, idcu_JsonValue* result) {
    if (strncmp(parser->pos, "true", 4) == 0) {
        result->type = IDCU_JSON_TYPE_BOOL;
        result->data.bool_val = 1;
        parser->pos += 4;
        parser->col += 4;
        return IDCU_ERR_OK;
    } else if (strncmp(parser->pos, "false", 5) == 0) {
        result->type = IDCU_JSON_TYPE_BOOL;
        result->data.bool_val = 0;
        parser->pos += 5;
        parser->col += 5;
        return IDCU_ERR_OK;
    }
    return IDCU_ERR_INVALID_ARG;
}

static int parse_number(json_parser_t* parser, idcu_JsonValue* result) {
    char* end;
    double d = strtod(parser->pos, &end);
    
    if (end == parser->pos) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (d == (int64_t)d) {
        result->type = IDCU_JSON_TYPE_INT;
        result->data.int_val = (int64_t)d;
    } else {
        result->type = IDCU_JSON_TYPE_DOUBLE;
        result->data.double_val = d;
    }
    
    parser->col += (end - parser->pos);
    parser->pos = end;
    return IDCU_ERR_OK;
}

static int parse_string(json_parser_t* parser, char** result) {
    if (*parser->pos != '\"') {
        return IDCU_ERR_INVALID_ARG;
    }
    parser->pos++;
    parser->col++;
    
    const char* start = parser->pos;
    size_t len = 0;
    
    while (*parser->pos && *parser->pos != '\"') {
        if (*parser->pos == '\\') {
            parser->pos++;
            parser->col++;
            if (!*parser->pos) {
                return IDCU_ERR_INVALID_ARG;
            }
        }
        parser->pos++;
        parser->col++;
        len++;
    }
    
    if (*parser->pos != '\"') {
        return IDCU_ERR_INVALID_ARG;
    }
    
    char* str = (char*)malloc(len + 1);
    if (!str) {
        return IDCU_ERR_NO_MEMORY;
    }
    
    char* dst = str;
    parser->pos = start;
    while (*parser->pos != '\"') {
        if (*parser->pos == '\\') {
            parser->pos++;
            switch (*parser->pos) {
                case 'n': *dst++ = '\n'; break;
                case 't': *dst++ = '\t'; break;
                case 'r': *dst++ = '\r'; break;
                case '\\': *dst++ = '\\'; break;
                case '\"': *dst++ = '\"'; break;
                default: *dst++ = *parser->pos; break;
            }
        } else {
            *dst++ = *parser->pos;
        }
        parser->pos++;
    }
    *dst = '\0';
    
    parser->pos++;
    parser->col++;
    *result = str;
    return IDCU_ERR_OK;
}

static int json_value_init(idcu_JsonValue* value) {
    memset(value, 0, sizeof(*value));
    return IDCU_ERR_OK;
}

static int json_array_init(idcu_JsonArray* array) {
    array->elements = (idcu_JsonValue*)malloc(JSON_INIT_CAPACITY * sizeof(idcu_JsonValue));
    if (!array->elements) {
        return IDCU_ERR_NO_MEMORY;
    }
    array->count = 0;
    array->capacity = JSON_INIT_CAPACITY;
    return IDCU_ERR_OK;
}

static int json_array_append(idcu_JsonArray* array, const idcu_JsonValue* value) {
    if (array->count >= array->capacity) {
        size_t new_capacity = array->capacity * JSON_CAPACITY_GROWTH_FACTOR;
        idcu_JsonValue* new_elements = (idcu_JsonValue*)realloc(
            array->elements, new_capacity * sizeof(idcu_JsonValue));
        if (!new_elements) {
            return IDCU_ERR_NO_MEMORY;
        }
        array->elements = new_elements;
        array->capacity = new_capacity;
    }
    memcpy(&array->elements[array->count], value, sizeof(idcu_JsonValue));
    array->count++;
    return IDCU_ERR_OK;
}

static int json_object_init(idcu_JsonObject* object) {
    object->keys = (char**)malloc(JSON_INIT_CAPACITY * sizeof(char*));
    if (!object->keys) {
        return IDCU_ERR_NO_MEMORY;
    }
    object->values = (idcu_JsonValue*)malloc(JSON_INIT_CAPACITY * sizeof(idcu_JsonValue));
    if (!object->values) {
        free(object->keys);
        return IDCU_ERR_NO_MEMORY;
    }
    object->count = 0;
    object->capacity = JSON_INIT_CAPACITY;
    return IDCU_ERR_OK;
}

static int json_object_set(idcu_JsonObject* object, const char* key, const idcu_JsonValue* value) {
    if (object->count >= object->capacity) {
        size_t new_capacity = object->capacity * JSON_CAPACITY_GROWTH_FACTOR;
        char** new_keys = (char**)realloc(object->keys, new_capacity * sizeof(char*));
        if (!new_keys) {
            return IDCU_ERR_NO_MEMORY;
        }
        idcu_JsonValue* new_values = (idcu_JsonValue*)realloc(
            object->values, new_capacity * sizeof(idcu_JsonValue));
        if (!new_values) {
            free(new_keys);
            return IDCU_ERR_NO_MEMORY;
        }
        object->keys = new_keys;
        object->values = new_values;
        object->capacity = new_capacity;
    }
    
    object->keys[object->count] = strdup(key);
    if (!object->keys[object->count]) {
        return IDCU_ERR_NO_MEMORY;
    }
    memcpy(&object->values[object->count], value, sizeof(idcu_JsonValue));
    object->count++;
    return IDCU_ERR_OK;
}

static int parse_value(json_parser_t* parser, idcu_JsonValue* result);

static int parse_array(json_parser_t* parser, idcu_JsonValue* result) {
    if (*parser->pos != '[') {
        return IDCU_ERR_INVALID_ARG;
    }
    parser->pos++;
    parser->col++;
    
    idcu_JsonArray* array = (idcu_JsonArray*)malloc(sizeof(idcu_JsonArray));
    if (!array) {
        return IDCU_ERR_NO_MEMORY;
    }
    
    int ret = json_array_init(array);
    if (ret != IDCU_ERR_OK) {
        free(array);
        return ret;
    }
    
    parser_skip_whitespace(parser);
    
    if (*parser->pos == ']') {
        parser->pos++;
        parser->col++;
        result->type = IDCU_JSON_TYPE_ARRAY;
        result->data.array_val = array;
        return IDCU_ERR_OK;
    }
    
    while (1) {
        idcu_JsonValue element;
        json_value_init(&element);
        ret = parse_value(parser, &element);
        if (ret != IDCU_ERR_OK) {
            idcu_json_free(result);
            free(array);
            return ret;
        }
        
        ret = json_array_append(array, &element);
        if (ret != IDCU_ERR_OK) {
            idcu_json_free(result);
            free(array);
            return ret;
        }
        
        parser_skip_whitespace(parser);
        
        if (*parser->pos == ']') {
            parser->pos++;
            parser->col++;
            break;
        } else if (*parser->pos == ',') {
            parser->pos++;
            parser->col++;
            parser_skip_whitespace(parser);
        } else {
            idcu_json_free(result);
            free(array);
            return IDCU_ERR_INVALID_ARG;
        }
    }
    
    result->type = IDCU_JSON_TYPE_ARRAY;
    result->data.array_val = array;
    return IDCU_ERR_OK;
}

static int parse_object(json_parser_t* parser, idcu_JsonValue* result) {
    if (*parser->pos != '{') {
        return IDCU_ERR_INVALID_ARG;
    }
    parser->pos++;
    parser->col++;
    
    idcu_JsonObject* object = (idcu_JsonObject*)malloc(sizeof(idcu_JsonObject));
    if (!object) {
        return IDCU_ERR_NO_MEMORY;
    }
    
    int ret = json_object_init(object);
    if (ret != IDCU_ERR_OK) {
        free(object);
        return ret;
    }
    
    parser_skip_whitespace(parser);
    
    if (*parser->pos == '}') {
        parser->pos++;
        parser->col++;
        result->type = IDCU_JSON_TYPE_OBJECT;
        result->data.object_val = object;
        return IDCU_ERR_OK;
    }
    
    while (1) {
        char* key = NULL;
        ret = parse_string(parser, &key);
        if (ret != IDCU_ERR_OK) {
            idcu_json_free(result);
            free(object);
            return ret;
        }
        
        parser_skip_whitespace(parser);
        
        if (*parser->pos != ':') {
            free(key);
            idcu_json_free(result);
            free(object);
            return IDCU_ERR_INVALID_ARG;
        }
        parser->pos++;
        parser->col++;
        parser_skip_whitespace(parser);
        
        idcu_JsonValue value;
        json_value_init(&value);
        ret = parse_value(parser, &value);
        if (ret != IDCU_ERR_OK) {
            free(key);
            idcu_json_free(result);
            free(object);
            return ret;
        }
        
        ret = json_object_set(object, key, &value);
        free(key);
        if (ret != IDCU_ERR_OK) {
            idcu_json_free(result);
            free(object);
            return ret;
        }
        
        parser_skip_whitespace(parser);
        
        if (*parser->pos == '}') {
            parser->pos++;
            parser->col++;
            break;
        } else if (*parser->pos == ',') {
            parser->pos++;
            parser->col++;
            parser_skip_whitespace(parser);
        } else {
            idcu_json_free(result);
            free(object);
            return IDCU_ERR_INVALID_ARG;
        }
    }
    
    result->type = IDCU_JSON_TYPE_OBJECT;
    result->data.object_val = object;
    return IDCU_ERR_OK;
}

static int parse_value(json_parser_t* parser, idcu_JsonValue* result) {
    parser_skip_whitespace(parser);
    
    if (!*parser->pos) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    switch (*parser->pos) {
        case 'n':
            return parse_null(parser, result);
        case 't':
        case 'f':
            return parse_bool(parser, result);
        case '\"':
            result->type = IDCU_JSON_TYPE_STRING;
            return parse_string(parser, &result->data.string_val);
        case '[':
            return parse_array(parser, result);
        case '{':
            return parse_object(parser, result);
        default:
            if (*parser->pos == '-' || isdigit((unsigned char)*parser->pos)) {
                return parse_number(parser, result);
            }
            return IDCU_ERR_INVALID_ARG;
    }
}

int idcu_json_parse(const char* json_str, idcu_JsonValue* result) {
    if (!json_str || !result) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    json_parser_t parser;
    parser.pos = json_str;
    parser.line = 1;
    parser.col = 1;
    
    json_value_init(result);
    return parse_value(&parser, result);
}

void idcu_json_free(idcu_JsonValue* value) {
    if (!value) {
        return;
    }
    
    switch (value->type) {
        case IDCU_JSON_TYPE_STRING:
            if (value->data.string_val) {
                free(value->data.string_val);
            }
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
    
    memset(value, 0, sizeof(*value));
}

idcu_JsonType idcu_json_get_type(const idcu_JsonValue* value) {
    if (!value) {
        return IDCU_JSON_TYPE_NULL;
    }
    return value->type;
}

int idcu_json_get_bool(const idcu_JsonValue* value, int* out) {
    if (!value || !out || value->type != IDCU_JSON_TYPE_BOOL) {
        return IDCU_ERR_INVALID_ARG;
    }
    *out = value->data.bool_val;
    return IDCU_ERR_OK;
}

int idcu_json_get_int(const idcu_JsonValue* value, int64_t* out) {
    if (!value || !out) {
        return IDCU_ERR_INVALID_ARG;
    }
    if (value->type == IDCU_JSON_TYPE_INT) {
        *out = value->data.int_val;
        return IDCU_ERR_OK;
    } else if (value->type == IDCU_JSON_TYPE_DOUBLE) {
        *out = (int64_t)value->data.double_val;
        return IDCU_ERR_OK;
    }
    return IDCU_ERR_INVALID_ARG;
}

int idcu_json_get_double(const idcu_JsonValue* value, double* out) {
    if (!value || !out) {
        return IDCU_ERR_INVALID_ARG;
    }
    if (value->type == IDCU_JSON_TYPE_DOUBLE) {
        *out = value->data.double_val;
        return IDCU_ERR_OK;
    } else if (value->type == IDCU_JSON_TYPE_INT) {
        *out = (double)value->data.int_val;
        return IDCU_ERR_OK;
    }
    return IDCU_ERR_INVALID_ARG;
}

int idcu_json_get_string(const idcu_JsonValue* value, const char** out) {
    if (!value || !out || value->type != IDCU_JSON_TYPE_STRING) {
        return IDCU_ERR_INVALID_ARG;
    }
    *out = value->data.string_val;
    return IDCU_ERR_OK;
}

size_t idcu_json_array_size(const idcu_JsonArray* array) {
    if (!array) {
        return 0;
    }
    return array->count;
}

idcu_JsonValue* idcu_json_array_get(const idcu_JsonArray* array, size_t index) {
    if (!array || index >= array->count) {
        return NULL;
    }
    return &array->elements[index];
}

idcu_JsonValue* idcu_json_object_get(const idcu_JsonObject* object, const char* key) {
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

int idcu_json_object_has(const idcu_JsonObject* object, const char* key) {
    return idcu_json_object_get(object, key) != NULL ? 1 : 0;
}

static int serialize_value(const idcu_JsonValue* value, char** buffer, size_t* buffer_size, size_t* offset);

static int ensure_buffer(char** buffer, size_t* buffer_size, size_t* offset, size_t required) {
    if (*offset + required >= *buffer_size) {
        size_t new_size = (*buffer_size == 0) ? 1024 : (*buffer_size * 2);
        while (*offset + required >= new_size) {
            new_size *= 2;
        }
        char* new_buffer = (char*)realloc(*buffer, new_size);
        if (!new_buffer) {
            return IDCU_ERR_NO_MEMORY;
        }
        *buffer = new_buffer;
        *buffer_size = new_size;
    }
    return IDCU_ERR_OK;
}

static int append_string(char** buffer, size_t* buffer_size, size_t* offset, const char* str) {
    size_t len = strlen(str);
    int ret = ensure_buffer(buffer, buffer_size, offset, len);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    strcpy(*buffer + *offset, str);
    *offset += len;
    return IDCU_ERR_OK;
}

static int append_char(char** buffer, size_t* buffer_size, size_t* offset, char c) {
    int ret = ensure_buffer(buffer, buffer_size, offset, 1);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    (*buffer)[(*offset)++] = c;
    return IDCU_ERR_OK;
}

static int serialize_string(const char* str, char** buffer, size_t* buffer_size, size_t* offset) {
    int ret = append_char(buffer, buffer_size, offset, '\"');
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    while (*str) {
        switch (*str) {
            case '\"':
                ret = append_string(buffer, buffer_size, offset, "\\\"");
                break;
            case '\\':
                ret = append_string(buffer, buffer_size, offset, "\\\\");
                break;
            case '\n':
                ret = append_string(buffer, buffer_size, offset, "\\n");
                break;
            case '\t':
                ret = append_string(buffer, buffer_size, offset, "\\t");
                break;
            case '\r':
                ret = append_string(buffer, buffer_size, offset, "\\r");
                break;
            default:
                ret = append_char(buffer, buffer_size, offset, *str);
                break;
        }
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
        str++;
    }
    
    return append_char(buffer, buffer_size, offset, '\"');
}

static int serialize_array(const idcu_JsonArray* array, char** buffer, size_t* buffer_size, size_t* offset) {
    int ret = append_char(buffer, buffer_size, offset, '[');
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    for (size_t i = 0; i < array->count; i++) {
        if (i > 0) {
            ret = append_char(buffer, buffer_size, offset, ',');
            if (ret != IDCU_ERR_OK) {
                return ret;
            }
        }
        ret = serialize_value(&array->elements[i], buffer, buffer_size, offset);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
    }
    
    return append_char(buffer, buffer_size, offset, ']');
}

static int serialize_object(const idcu_JsonObject* object, char** buffer, size_t* buffer_size, size_t* offset) {
    int ret = append_char(buffer, buffer_size, offset, '{');
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    for (size_t i = 0; i < object->count; i++) {
        if (i > 0) {
            ret = append_char(buffer, buffer_size, offset, ',');
            if (ret != IDCU_ERR_OK) {
                return ret;
            }
        }
        ret = serialize_string(object->keys[i], buffer, buffer_size, offset);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
        ret = append_char(buffer, buffer_size, offset, ':');
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
        ret = serialize_value(&object->values[i], buffer, buffer_size, offset);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
    }
    
    return append_char(buffer, buffer_size, offset, '}');
}

static int serialize_value(const idcu_JsonValue* value, char** buffer, size_t* buffer_size, size_t* offset) {
    char num_buf[64];
    
    switch (value->type) {
        case IDCU_JSON_TYPE_NULL:
            return append_string(buffer, buffer_size, offset, "null");
        case IDCU_JSON_TYPE_BOOL:
            return append_string(buffer, buffer_size, offset, 
                value->data.bool_val ? "true" : "false");
        case IDCU_JSON_TYPE_INT:
            snprintf(num_buf, sizeof(num_buf), "%" PRId64, value->data.int_val);
            return append_string(buffer, buffer_size, offset, num_buf);
        case IDCU_JSON_TYPE_DOUBLE:
            snprintf(num_buf, sizeof(num_buf), "%g", value->data.double_val);
            return append_string(buffer, buffer_size, offset, num_buf);
        case IDCU_JSON_TYPE_STRING:
            return serialize_string(value->data.string_val, buffer, buffer_size, offset);
        case IDCU_JSON_TYPE_ARRAY:
            return serialize_array(value->data.array_val, buffer, buffer_size, offset);
        case IDCU_JSON_TYPE_OBJECT:
            return serialize_object(value->data.object_val, buffer, buffer_size, offset);
        default:
            return IDCU_ERR_INVALID_ARG;
    }
}

int idcu_json_to_string(const idcu_JsonValue* value, char* buffer, size_t buffer_size) {
    if (!value || !buffer) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    char* dyn_buffer = NULL;
    size_t dyn_size = 0;
    size_t offset = 0;
    
    int ret = serialize_value(value, &dyn_buffer, &dyn_size, &offset);
    if (ret != IDCU_ERR_OK) {
        free(dyn_buffer);
        return ret;
    }
    
    if (offset >= buffer_size) {
        free(dyn_buffer);
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    
    memcpy(buffer, dyn_buffer, offset + 1);
    free(dyn_buffer);
    return IDCU_ERR_OK;
}

char* idcu_json_to_string_alloc(const idcu_JsonValue* value) {
    if (!value) {
        return NULL;
    }
    
    char* buffer = NULL;
    size_t buffer_size = 0;
    size_t offset = 0;
    
    int ret = serialize_value(value, &buffer, &buffer_size, &offset);
    if (ret != IDCU_ERR_OK) {
        free(buffer);
        return NULL;
    }
    
    return buffer;
}
