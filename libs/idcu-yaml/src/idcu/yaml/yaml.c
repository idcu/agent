#include <idcu/yaml/yaml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#define YAML_INIT_CAPACITY 8
#define YAML_CAPACITY_GROWTH_FACTOR 2

typedef struct {
    const char* pos;
    const char* line_start;
    size_t line;
    size_t col;
} yaml_parser_t;

static int yaml_value_init(idcu_YamlValue* value) {
    memset(value, 0, sizeof(*value));
    return IDCU_ERR_OK;
}

static int yaml_sequence_init(idcu_YamlSequence* seq) {
    seq->elements = (idcu_YamlValue*)malloc(YAML_INIT_CAPACITY * sizeof(idcu_YamlValue));
    if (!seq->elements) {
        return IDCU_ERR_NO_MEMORY;
    }
    seq->count = 0;
    seq->capacity = YAML_INIT_CAPACITY;
    return IDCU_ERR_OK;
}

static int yaml_sequence_append(idcu_YamlSequence* seq, const idcu_YamlValue* value) {
    if (seq->count >= seq->capacity) {
        size_t new_capacity = seq->capacity * YAML_CAPACITY_GROWTH_FACTOR;
        idcu_YamlValue* new_elements = (idcu_YamlValue*)realloc(
            seq->elements, new_capacity * sizeof(idcu_YamlValue));
        if (!new_elements) {
            return IDCU_ERR_NO_MEMORY;
        }
        seq->elements = new_elements;
        seq->capacity = new_capacity;
    }
    memcpy(&seq->elements[seq->count], value, sizeof(idcu_YamlValue));
    seq->count++;
    return IDCU_ERR_OK;
}

static int yaml_mapping_init(idcu_YamlMapping* map) {
    map->keys = (char**)malloc(YAML_INIT_CAPACITY * sizeof(char*));
    if (!map->keys) {
        return IDCU_ERR_NO_MEMORY;
    }
    map->values = (idcu_YamlValue*)malloc(YAML_INIT_CAPACITY * sizeof(idcu_YamlValue));
    if (!map->values) {
        free(map->keys);
        return IDCU_ERR_NO_MEMORY;
    }
    map->count = 0;
    map->capacity = YAML_INIT_CAPACITY;
    return IDCU_ERR_OK;
}

static int yaml_mapping_set(idcu_YamlMapping* map, const char* key, const idcu_YamlValue* value) {
    if (map->count >= map->capacity) {
        size_t new_capacity = map->capacity * YAML_CAPACITY_GROWTH_FACTOR;
        char** new_keys = (char**)realloc(map->keys, new_capacity * sizeof(char*));
        if (!new_keys) {
            return IDCU_ERR_NO_MEMORY;
        }
        idcu_YamlValue* new_values = (idcu_YamlValue*)realloc(
            map->values, new_capacity * sizeof(idcu_YamlValue));
        if (!new_values) {
            free(new_keys);
            return IDCU_ERR_NO_MEMORY;
        }
        map->keys = new_keys;
        map->values = new_values;
        map->capacity = new_capacity;
    }
    
    map->keys[map->count] = strdup(key);
    if (!map->keys[map->count]) {
        return IDCU_ERR_NO_MEMORY;
    }
    memcpy(&map->values[map->count], value, sizeof(idcu_YamlValue));
    map->count++;
    return IDCU_ERR_OK;
}

static void parser_skip_whitespace(yaml_parser_t* parser) {
    while (*parser->pos && isspace((unsigned char)*parser->pos) && *parser->pos != '\n') {
        parser->pos++;
        parser->col++;
    }
}

static int parser_is_eol(yaml_parser_t* parser) {
    return *parser->pos == '\n' || *parser->pos == '\r' || *parser->pos == '\0';
}

static void parser_skip_line(yaml_parser_t* parser) {
    while (*parser->pos && *parser->pos != '\n') {
        parser->pos++;
    }
    if (*parser->pos == '\n') {
        parser->pos++;
        parser->line++;
        parser->col = 1;
        parser->line_start = parser->pos;
    }
}

static size_t parser_get_indent(yaml_parser_t* parser) {
    size_t indent = 0;
    const char* p = parser->line_start;
    while (*p == ' ') {
        indent++;
        p++;
    }
    return indent;
}

static int parse_scalar(yaml_parser_t* parser, idcu_YamlValue* result) {
    parser_skip_whitespace(parser);
    
    if (*parser->pos == '\"') {
        parser->pos++;
        parser->col++;
        const char* start = parser->pos;
        while (*parser->pos && *parser->pos != '\"') {
            if (*parser->pos == '\\') {
                parser->pos++;
                if (!*parser->pos) break;
            }
            parser->pos++;
        }
        if (*parser->pos == '\"') {
            size_t len = parser->pos - start;
            char* str = (char*)malloc(len + 1);
            if (!str) {
                return IDCU_ERR_NO_MEMORY;
            }
            memcpy(str, start, len);
            str[len] = '\0';
            result->type = IDCU_YAML_TYPE_STRING;
            result->data.string_val = str;
            parser->pos++;
            return IDCU_ERR_OK;
        }
        return IDCU_ERR_INVALID_ARG;
    }
    
    const char* start = parser->pos;
    while (*parser->pos && !parser_is_eol(parser) && *parser->pos != '#') {
        parser->pos++;
    }
    
    while (parser->pos > start && isspace((unsigned char)*(parser->pos - 1))) {
        parser->pos--;
    }
    
    if (parser->pos == start) {
        result->type = IDCU_YAML_TYPE_NULL;
        return IDCU_ERR_OK;
    }
    
    size_t len = parser->pos - start;
    char* scalar = (char*)malloc(len + 1);
    if (!scalar) {
        return IDCU_ERR_NO_MEMORY;
    }
    memcpy(scalar, start, len);
    scalar[len] = '\0';
    
    if (strcmp(scalar, "null") == 0 || strcmp(scalar, "~") == 0) {
        free(scalar);
        result->type = IDCU_YAML_TYPE_NULL;
        return IDCU_ERR_OK;
    }
    
    if (strcmp(scalar, "true") == 0) {
        free(scalar);
        result->type = IDCU_YAML_TYPE_BOOL;
        result->data.bool_val = 1;
        return IDCU_ERR_OK;
    }
    
    if (strcmp(scalar, "false") == 0) {
        free(scalar);
        result->type = IDCU_YAML_TYPE_BOOL;
        result->data.bool_val = 0;
        return IDCU_ERR_OK;
    }
    
    char* end;
    double d = strtod(scalar, &end);
    if (end == scalar + len) {
        if (d == (int64_t)d) {
            free(scalar);
            result->type = IDCU_YAML_TYPE_INT;
            result->data.int_val = (int64_t)d;
        } else {
            free(scalar);
            result->type = IDCU_YAML_TYPE_DOUBLE;
            result->data.double_val = d;
        }
        return IDCU_ERR_OK;
    }
    
    result->type = IDCU_YAML_TYPE_STRING;
    result->data.string_val = scalar;
    return IDCU_ERR_OK;
}

static int parse_value(yaml_parser_t* parser, idcu_YamlValue* result, size_t indent);

static int parse_sequence_item(yaml_parser_t* parser, idcu_YamlValue* result, size_t indent) {
    parser->pos += 2;
    parser->col += 2;
    
    idcu_YamlValue item;
    yaml_value_init(&item);
    
    parser_skip_whitespace(parser);
    
    if (parser_is_eol(parser)) {
        parser_skip_line(parser);
        idcu_YamlSequence* seq = (idcu_YamlSequence*)malloc(sizeof(idcu_YamlSequence));
        if (!seq) {
            return IDCU_ERR_NO_MEMORY;
        }
        yaml_sequence_init(seq);
        
        while (1) {
            while (*parser->pos && (*parser->pos == '\n' || *parser->pos == '\r')) {
                parser_skip_line(parser);
            }
            
            if (!*parser->pos) {
                break;
            }
            
            size_t current_indent = parser_get_indent(parser);
            if (current_indent <= indent) {
                break;
            }
            
            parser->pos = parser->line_start + current_indent;
            parser->col = current_indent + 1;
            
            idcu_YamlValue nested_item;
            yaml_value_init(&nested_item);
            
            int ret = parse_value(parser, &nested_item, current_indent);
            if (ret != IDCU_ERR_OK) {
                idcu_yaml_free(result);
                free(seq);
                return ret;
            }
            
            yaml_sequence_append(seq, &nested_item);
        }
        
        result->type = IDCU_YAML_TYPE_SEQUENCE;
        result->data.sequence_val = seq;
        return IDCU_ERR_OK;
    }
    
    int ret = parse_scalar(parser, &item);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    result->type = IDCU_YAML_TYPE_SEQUENCE;
    idcu_YamlSequence* seq = (idcu_YamlSequence*)malloc(sizeof(idcu_YamlSequence));
    if (!seq) {
        return IDCU_ERR_NO_MEMORY;
    }
    yaml_sequence_init(seq);
    yaml_sequence_append(seq, &item);
    result->data.sequence_val = seq;
    
    return IDCU_ERR_OK;
}

static int parse_key_value(yaml_parser_t* parser, idcu_YamlValue* result, size_t indent) {
    const char* key_start = parser->pos;
    while (*parser->pos && *parser->pos != ':' && !parser_is_eol(parser)) {
        parser->pos++;
    }
    
    if (*parser->pos != ':') {
        return IDCU_ERR_INVALID_ARG;
    }
    
    const char* key_end = parser->pos;
    while (key_end > key_start && isspace((unsigned char)*(key_end - 1))) {
        key_end--;
    }
    
    size_t key_len = key_end - key_start;
    char* key = (char*)malloc(key_len + 1);
    if (!key) {
        return IDCU_ERR_NO_MEMORY;
    }
    memcpy(key, key_start, key_len);
    key[key_len] = '\0';
    
    parser->pos++;
    parser->col++;
    parser_skip_whitespace(parser);
    
    idcu_YamlValue value;
    yaml_value_init(&value);
    
    if (parser_is_eol(parser)) {
        parser_skip_line(parser);
        idcu_YamlMapping* map = (idcu_YamlMapping*)malloc(sizeof(idcu_YamlMapping));
        if (!map) {
            free(key);
            return IDCU_ERR_NO_MEMORY;
        }
        yaml_mapping_init(map);
        
        while (1) {
            while (*parser->pos && (*parser->pos == '\n' || *parser->pos == '\r')) {
                parser_skip_line(parser);
            }
            
            if (!*parser->pos) {
                break;
            }
            
            size_t current_indent = parser_get_indent(parser);
            if (current_indent <= indent) {
                break;
            }
            
            parser->pos = parser->line_start + current_indent;
            parser->col = current_indent + 1;
            
            idcu_YamlValue nested_key;
            yaml_value_init(&nested_key);
            
            int ret = parse_value(parser, &nested_key, current_indent);
            if (ret != IDCU_ERR_OK) {
                idcu_yaml_free(result);
                free(key);
                free(map);
                return ret;
            }
            
            if (nested_key.type == IDCU_YAML_TYPE_MAPPING) {
                for (size_t i = 0; i < nested_key.data.mapping_val->count; i++) {
                    yaml_mapping_set(map, nested_key.data.mapping_val->keys[i], 
                                     &nested_key.data.mapping_val->values[i]);
                }
                free(nested_key.data.mapping_val->keys);
                free(nested_key.data.mapping_val->values);
                free(nested_key.data.mapping_val);
            }
        }
        
        value.type = IDCU_YAML_TYPE_MAPPING;
        value.data.mapping_val = map;
    } else {
        int ret = parse_scalar(parser, &value);
        if (ret != IDCU_ERR_OK) {
            free(key);
            return ret;
        }
    }
    
    result->type = IDCU_YAML_TYPE_MAPPING;
    idcu_YamlMapping* map = (idcu_YamlMapping*)malloc(sizeof(idcu_YamlMapping));
    if (!map) {
        free(key);
        idcu_yaml_free(&value);
        return IDCU_ERR_NO_MEMORY;
    }
    yaml_mapping_init(map);
    yaml_mapping_set(map, key, &value);
    free(key);
    result->data.mapping_val = map;
    
    return IDCU_ERR_OK;
}

static int parse_value(yaml_parser_t* parser, idcu_YamlValue* result, size_t indent) {
    parser_skip_whitespace(parser);
    
    if (!*parser->pos) {
        result->type = IDCU_YAML_TYPE_NULL;
        return IDCU_ERR_OK;
    }
    
    if (*parser->pos == '#') {
        parser_skip_line(parser);
        return parse_value(parser, result, indent);
    }
    
    if (*parser->pos == '-' && *(parser->pos + 1) == ' ') {
        return parse_sequence_item(parser, result, indent);
    }
    
    return parse_key_value(parser, result, indent);
}

int idcu_yaml_parse(const char* yaml_str, idcu_YamlValue* result) {
    if (!yaml_str || !result) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    yaml_parser_t parser;
    parser.pos = yaml_str;
    parser.line_start = yaml_str;
    parser.line = 1;
    parser.col = 1;
    
    yaml_value_init(result);
    
    while (*parser.pos && (*parser.pos == '\n' || *parser.pos == '\r')) {
        if (*parser.pos == '\n') {
            parser.line++;
            parser.col = 1;
        }
        parser.pos++;
        parser.line_start = parser.pos;
    }
    
    if (!*parser.pos) {
        result->type = IDCU_YAML_TYPE_MAPPING;
        idcu_YamlMapping* map = (idcu_YamlMapping*)malloc(sizeof(idcu_YamlMapping));
        if (!map) {
            return IDCU_ERR_NO_MEMORY;
        }
        yaml_mapping_init(map);
        result->data.mapping_val = map;
        return IDCU_ERR_OK;
    }
    
    idcu_YamlValue root;
    yaml_value_init(&root);
    
    idcu_YamlMapping* map = (idcu_YamlMapping*)malloc(sizeof(idcu_YamlMapping));
    if (!map) {
        return IDCU_ERR_NO_MEMORY;
    }
    yaml_mapping_init(map);
    
    while (1) {
        while (*parser.pos && (*parser.pos == '\n' || *parser.pos == '\r')) {
            parser_skip_line(&parser);
        }
        
        if (!*parser.pos) {
            break;
        }
        
        if (*parser.pos == '#') {
            parser_skip_line(&parser);
            continue;
        }
        
        parser.line_start = parser.pos;
        size_t current_indent = parser_get_indent(&parser);
        parser.pos = parser.line_start + current_indent;
        parser.col = current_indent + 1;
        
        idcu_YamlValue value;
        yaml_value_init(&value);
        
        int ret = parse_value(&parser, &value, current_indent);
        if (ret != IDCU_ERR_OK) {
            idcu_yaml_free(&root);
            free(map->keys);
            free(map->values);
            free(map);
            return ret;
        }
        
        if (value.type == IDCU_YAML_TYPE_MAPPING) {
            for (size_t i = 0; i < value.data.mapping_val->count; i++) {
                yaml_mapping_set(map, value.data.mapping_val->keys[i], 
                                 &value.data.mapping_val->values[i]);
            }
            free(value.data.mapping_val->keys);
            free(value.data.mapping_val->values);
            free(value.data.mapping_val);
        }
    }
    
    result->type = IDCU_YAML_TYPE_MAPPING;
    result->data.mapping_val = map;
    
    return IDCU_ERR_OK;
}

void idcu_yaml_free(idcu_YamlValue* value) {
    if (!value) {
        return;
    }
    
    switch (value->type) {
        case IDCU_YAML_TYPE_STRING:
            if (value->data.string_val) {
                free(value->data.string_val);
            }
            break;
        case IDCU_YAML_TYPE_SEQUENCE:
            if (value->data.sequence_val) {
                for (size_t i = 0; i < value->data.sequence_val->count; i++) {
                    idcu_yaml_free(&value->data.sequence_val->elements[i]);
                }
                free(value->data.sequence_val->elements);
                free(value->data.sequence_val);
            }
            break;
        case IDCU_YAML_TYPE_MAPPING:
            if (value->data.mapping_val) {
                for (size_t i = 0; i < value->data.mapping_val->count; i++) {
                    free(value->data.mapping_val->keys[i]);
                    idcu_yaml_free(&value->data.mapping_val->values[i]);
                }
                free(value->data.mapping_val->keys);
                free(value->data.mapping_val->values);
                free(value->data.mapping_val);
            }
            break;
        default:
            break;
    }
    
    memset(value, 0, sizeof(*value));
}

idcu_YamlType idcu_yaml_get_type(const idcu_YamlValue* value) {
    if (!value) {
        return IDCU_YAML_TYPE_NULL;
    }
    return value->type;
}

int idcu_yaml_get_bool(const idcu_YamlValue* value, int* out) {
    if (!value || !out || value->type != IDCU_YAML_TYPE_BOOL) {
        return IDCU_ERR_INVALID_ARG;
    }
    *out = value->data.bool_val;
    return IDCU_ERR_OK;
}

int idcu_yaml_get_int(const idcu_YamlValue* value, int64_t* out) {
    if (!value || !out) {
        return IDCU_ERR_INVALID_ARG;
    }
    if (value->type == IDCU_YAML_TYPE_INT) {
        *out = value->data.int_val;
        return IDCU_ERR_OK;
    } else if (value->type == IDCU_YAML_TYPE_DOUBLE) {
        *out = (int64_t)value->data.double_val;
        return IDCU_ERR_OK;
    }
    return IDCU_ERR_INVALID_ARG;
}

int idcu_yaml_get_double(const idcu_YamlValue* value, double* out) {
    if (!value || !out) {
        return IDCU_ERR_INVALID_ARG;
    }
    if (value->type == IDCU_YAML_TYPE_DOUBLE) {
        *out = value->data.double_val;
        return IDCU_ERR_OK;
    } else if (value->type == IDCU_YAML_TYPE_INT) {
        *out = (double)value->data.int_val;
        return IDCU_ERR_OK;
    }
    return IDCU_ERR_INVALID_ARG;
}

int idcu_yaml_get_string(const idcu_YamlValue* value, const char** out) {
    if (!value || !out || value->type != IDCU_YAML_TYPE_STRING) {
        return IDCU_ERR_INVALID_ARG;
    }
    *out = value->data.string_val;
    return IDCU_ERR_OK;
}

size_t idcu_yaml_sequence_size(const idcu_YamlSequence* seq) {
    if (!seq) {
        return 0;
    }
    return seq->count;
}

idcu_YamlValue* idcu_yaml_sequence_get(const idcu_YamlSequence* seq, size_t index) {
    if (!seq || index >= seq->count) {
        return NULL;
    }
    return &seq->elements[index];
}

idcu_YamlValue* idcu_yaml_mapping_get(const idcu_YamlMapping* map, const char* key) {
    if (!map || !key) {
        return NULL;
    }
    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->keys[i], key) == 0) {
            return &map->values[i];
        }
    }
    return NULL;
}

int idcu_yaml_mapping_has(const idcu_YamlMapping* map, const char* key) {
    return idcu_yaml_mapping_get(map, key) != NULL ? 1 : 0;
}

static int serialize_value(const idcu_YamlValue* value, char** buffer, size_t* buffer_size, 
                           size_t* offset, size_t indent);

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

static int append_indent(char** buffer, size_t* buffer_size, size_t* offset, size_t indent) {
    for (size_t i = 0; i < indent; i++) {
        int ret = append_char(buffer, buffer_size, offset, ' ');
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
    }
    return IDCU_ERR_OK;
}

static int append_newline(char** buffer, size_t* buffer_size, size_t* offset) {
    return append_char(buffer, buffer_size, offset, '\n');
}

static int serialize_scalar(const idcu_YamlValue* value, char** buffer, size_t* buffer_size, size_t* offset) {
    char num_buf[64];
    
    switch (value->type) {
        case IDCU_YAML_TYPE_NULL:
            return append_string(buffer, buffer_size, offset, "null");
        case IDCU_YAML_TYPE_BOOL:
            return append_string(buffer, buffer_size, offset, 
                value->data.bool_val ? "true" : "false");
        case IDCU_YAML_TYPE_INT:
            snprintf(num_buf, sizeof(num_buf), "%" PRId64, value->data.int_val);
            return append_string(buffer, buffer_size, offset, num_buf);
        case IDCU_YAML_TYPE_DOUBLE:
            snprintf(num_buf, sizeof(num_buf), "%g", value->data.double_val);
            return append_string(buffer, buffer_size, offset, num_buf);
        case IDCU_YAML_TYPE_STRING:
            return append_string(buffer, buffer_size, offset, value->data.string_val);
        default:
            return IDCU_ERR_INVALID_ARG;
    }
}

static int serialize_sequence(const idcu_YamlSequence* seq, char** buffer, size_t* buffer_size, 
                              size_t* offset, size_t indent) {
    for (size_t i = 0; i < seq->count; i++) {
        int ret = append_indent(buffer, buffer_size, offset, indent);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
        ret = append_string(buffer, buffer_size, offset, "- ");
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
        
        idcu_YamlValue* item = &seq->elements[i];
        if (item->type == IDCU_YAML_TYPE_SEQUENCE || item->type == IDCU_YAML_TYPE_MAPPING) {
            ret = append_newline(buffer, buffer_size, offset);
            if (ret != IDCU_ERR_OK) {
                return ret;
            }
            ret = serialize_value(item, buffer, buffer_size, offset, indent + 2);
            if (ret != IDCU_ERR_OK) {
                return ret;
            }
        } else {
            ret = serialize_scalar(item, buffer, buffer_size, offset);
            if (ret != IDCU_ERR_OK) {
                return ret;
            }
            if (i < seq->count - 1) {
                ret = append_newline(buffer, buffer_size, offset);
                if (ret != IDCU_ERR_OK) {
                    return ret;
                }
            }
        }
    }
    return IDCU_ERR_OK;
}

static int serialize_mapping(const idcu_YamlMapping* map, char** buffer, size_t* buffer_size, 
                             size_t* offset, size_t indent) {
    for (size_t i = 0; i < map->count; i++) {
        int ret = append_indent(buffer, buffer_size, offset, indent);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
        ret = append_string(buffer, buffer_size, offset, map->keys[i]);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
        ret = append_string(buffer, buffer_size, offset, ": ");
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
        
        idcu_YamlValue* value = &map->values[i];
        if (value->type == IDCU_YAML_TYPE_SEQUENCE || value->type == IDCU_YAML_TYPE_MAPPING) {
            ret = append_newline(buffer, buffer_size, offset);
            if (ret != IDCU_ERR_OK) {
                return ret;
            }
            ret = serialize_value(value, buffer, buffer_size, offset, indent + 2);
            if (ret != IDCU_ERR_OK) {
                return ret;
            }
        } else {
            ret = serialize_scalar(value, buffer, buffer_size, offset);
            if (ret != IDCU_ERR_OK) {
                return ret;
            }
            if (i < map->count - 1) {
                ret = append_newline(buffer, buffer_size, offset);
                if (ret != IDCU_ERR_OK) {
                    return ret;
                }
            }
        }
    }
    return IDCU_ERR_OK;
}

static int serialize_value(const idcu_YamlValue* value, char** buffer, size_t* buffer_size, 
                           size_t* offset, size_t indent) {
    switch (value->type) {
        case IDCU_YAML_TYPE_SEQUENCE:
            return serialize_sequence(value->data.sequence_val, buffer, buffer_size, offset, indent);
        case IDCU_YAML_TYPE_MAPPING:
            return serialize_mapping(value->data.mapping_val, buffer, buffer_size, offset, indent);
        default:
            return serialize_scalar(value, buffer, buffer_size, offset);
    }
}

int idcu_yaml_to_string(const idcu_YamlValue* value, char* buffer, size_t buffer_size) {
    if (!value || !buffer) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    char* dyn_buffer = NULL;
    size_t dyn_size = 0;
    size_t offset = 0;
    
    int ret = serialize_value(value, &dyn_buffer, &dyn_size, &offset, 0);
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

char* idcu_yaml_to_string_alloc(const idcu_YamlValue* value) {
    if (!value) {
        return NULL;
    }
    
    char* buffer = NULL;
    size_t buffer_size = 0;
    size_t offset = 0;
    
    int ret = serialize_value(value, &buffer, &buffer_size, &offset, 0);
    if (ret != IDCU_ERR_OK) {
        free(buffer);
        return NULL;
    }
    
    return buffer;
}

static int json_serialize_value(const idcu_YamlValue* value, char** buffer, size_t* buffer_size, size_t* offset);

static int json_ensure_buffer(char** buffer, size_t* buffer_size, size_t* offset, size_t required) {
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

static int json_append_string(char** buffer, size_t* buffer_size, size_t* offset, const char* str) {
    size_t len = strlen(str);
    int ret = json_ensure_buffer(buffer, buffer_size, offset, len);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    strcpy(*buffer + *offset, str);
    *offset += len;
    return IDCU_ERR_OK;
}

static int json_append_char(char** buffer, size_t* buffer_size, size_t* offset, char c) {
    int ret = json_ensure_buffer(buffer, buffer_size, offset, 1);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    (*buffer)[(*offset)++] = c;
    return IDCU_ERR_OK;
}

static int json_serialize_string(const char* str, char** buffer, size_t* buffer_size, size_t* offset) {
    int ret = json_append_char(buffer, buffer_size, offset, '\"');
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    while (*str) {
        switch (*str) {
            case '\"':
                ret = json_append_string(buffer, buffer_size, offset, "\\\"");
                break;
            case '\\':
                ret = json_append_string(buffer, buffer_size, offset, "\\\\");
                break;
            case '\n':
                ret = json_append_string(buffer, buffer_size, offset, "\\n");
                break;
            case '\t':
                ret = json_append_string(buffer, buffer_size, offset, "\\t");
                break;
            case '\r':
                ret = json_append_string(buffer, buffer_size, offset, "\\r");
                break;
            default:
                ret = json_append_char(buffer, buffer_size, offset, *str);
                break;
        }
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
        str++;
    }
    
    return json_append_char(buffer, buffer_size, offset, '\"');
}

static int json_serialize_scalar(const idcu_YamlValue* value, char** buffer, size_t* buffer_size, size_t* offset) {
    char num_buf[64];
    
    switch (value->type) {
        case IDCU_YAML_TYPE_NULL:
            return json_append_string(buffer, buffer_size, offset, "null");
        case IDCU_YAML_TYPE_BOOL:
            return json_append_string(buffer, buffer_size, offset, 
                value->data.bool_val ? "true" : "false");
        case IDCU_YAML_TYPE_INT:
            snprintf(num_buf, sizeof(num_buf), "%" PRId64, value->data.int_val);
            return json_append_string(buffer, buffer_size, offset, num_buf);
        case IDCU_YAML_TYPE_DOUBLE:
            snprintf(num_buf, sizeof(num_buf), "%g", value->data.double_val);
            return json_append_string(buffer, buffer_size, offset, num_buf);
        case IDCU_YAML_TYPE_STRING:
            return json_serialize_string(value->data.string_val, buffer, buffer_size, offset);
        default:
            return IDCU_ERR_INVALID_ARG;
    }
}

static int json_serialize_sequence(const idcu_YamlSequence* seq, char** buffer, size_t* buffer_size, size_t* offset) {
    int ret = json_append_char(buffer, buffer_size, offset, '[');
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    for (size_t i = 0; i < seq->count; i++) {
        if (i > 0) {
            ret = json_append_char(buffer, buffer_size, offset, ',');
            if (ret != IDCU_ERR_OK) {
                return ret;
            }
        }
        ret = json_serialize_value(&seq->elements[i], buffer, buffer_size, offset);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
    }
    
    return json_append_char(buffer, buffer_size, offset, ']');
}

static int json_serialize_mapping(const idcu_YamlMapping* map, char** buffer, size_t* buffer_size, size_t* offset) {
    int ret = json_append_char(buffer, buffer_size, offset, '{');
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    for (size_t i = 0; i < map->count; i++) {
        if (i > 0) {
            ret = json_append_char(buffer, buffer_size, offset, ',');
            if (ret != IDCU_ERR_OK) {
                return ret;
            }
        }
        ret = json_serialize_string(map->keys[i], buffer, buffer_size, offset);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
        ret = json_append_char(buffer, buffer_size, offset, ':');
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
        ret = json_serialize_value(&map->values[i], buffer, buffer_size, offset);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
    }
    
    return json_append_char(buffer, buffer_size, offset, '}');
}

static int json_serialize_value(const idcu_YamlValue* value, char** buffer, size_t* buffer_size, size_t* offset) {
    switch (value->type) {
        case IDCU_YAML_TYPE_SEQUENCE:
            return json_serialize_sequence(value->data.sequence_val, buffer, buffer_size, offset);
        case IDCU_YAML_TYPE_MAPPING:
            return json_serialize_mapping(value->data.mapping_val, buffer, buffer_size, offset);
        default:
            return json_serialize_scalar(value, buffer, buffer_size, offset);
    }
}

int idcu_yaml_to_json(const idcu_YamlValue* yaml_value, char* buffer, size_t buffer_size) {
    if (!yaml_value || !buffer) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    char* dyn_buffer = NULL;
    size_t dyn_size = 0;
    size_t offset = 0;
    
    int ret = json_serialize_value(yaml_value, &dyn_buffer, &dyn_size, &offset);
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
