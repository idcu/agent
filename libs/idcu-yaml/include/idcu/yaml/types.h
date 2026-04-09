#ifndef IDCU_YAML_TYPES_H
#define IDCU_YAML_TYPES_H

#include <idcu/common/config.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IDCU_YAML_TYPE_NULL = 0,
    IDCU_YAML_TYPE_BOOL,
    IDCU_YAML_TYPE_INT,
    IDCU_YAML_TYPE_DOUBLE,
    IDCU_YAML_TYPE_STRING,
    IDCU_YAML_TYPE_SEQUENCE,
    IDCU_YAML_TYPE_MAPPING
} idcu_YamlType;

typedef struct idcu_YamlValue  idcu_YamlValue;
typedef struct idcu_YamlSequence idcu_YamlSequence;
typedef struct idcu_YamlMapping  idcu_YamlMapping;

struct idcu_YamlSequence {
    idcu_YamlValue* elements;
    size_t          count;
    size_t          capacity;
};

struct idcu_YamlMapping {
    char**          keys;
    idcu_YamlValue* values;
    size_t          count;
    size_t          capacity;
};

struct idcu_YamlValue {
    idcu_YamlType type;
    union {
        int               bool_val;
        int64_t           int_val;
        double            double_val;
        char*             string_val;
        idcu_YamlSequence* sequence_val;
        idcu_YamlMapping*  mapping_val;
    } data;
};

#ifdef __cplusplus
}
#endif

#endif
