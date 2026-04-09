#ifndef IDCU_JSON_TYPES_H
#define IDCU_JSON_TYPES_H

#include <idcu/common/config.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
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

struct idcu_JsonArray {
    idcu_JsonValue* elements;
    size_t          count;
    size_t          capacity;
};

struct idcu_JsonObject {
    char**          keys;
    idcu_JsonValue* values;
    size_t          count;
    size_t          capacity;
};

struct idcu_JsonValue {
    idcu_JsonType type;
    union {
        int              bool_val;
        int64_t          int_val;
        double           double_val;
        char*            string_val;
        idcu_JsonArray*  array_val;
        idcu_JsonObject* object_val;
    } data;
};

#ifdef __cplusplus
}
#endif

#endif
