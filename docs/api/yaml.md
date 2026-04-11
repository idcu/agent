# idcu-yaml API Documentation

YAML parser and serializer library.

## YAML Value Types

```c
typedef enum {
    IDCU_YAML_TYPE_NULL,
    IDCU_YAML_TYPE_BOOL,
    IDCU_YAML_TYPE_INT,
    IDCU_YAML_TYPE_DOUBLE,
    IDCU_YAML_TYPE_STRING,
    IDCU_YAML_TYPE_SEQUENCE,
    IDCU_YAML_TYPE_MAPPING
} idcu_YamlType;
```

## YAML Value Structure

```c
typedef struct idcu_YamlValue idcu_YamlValue;
```

## Parsing Functions

```c
int idcu_yaml_parse(const char* yaml_str, idcu_YamlValue* out_value);
int idcu_yaml_parse_file(const char* filepath, idcu_YamlValue* out_value);
void idcu_yaml_free(idcu_YamlValue* value);
```

## Type Checking

```c
idcu_YamlType idcu_yaml_get_type(const idcu_YamlValue* value);
```

## Value Getters

```c
int idcu_yaml_get_bool(const idcu_YamlValue* value, int* out_bool);
int idcu_yaml_get_int(const idcu_YamlValue* value, int64_t* out_int);
int idcu_yaml_get_double(const idcu_YamlValue* value, double* out_double);
int idcu_yaml_get_string(const idcu_YamlValue* value, const char** out_string);
```

## Sequence (Array) Operations

```c
size_t idcu_yaml_sequence_size(const idcu_YamlValue* value);
idcu_YamlValue* idcu_yaml_sequence_get(const idcu_YamlValue* value, size_t index);
```

## Mapping (Object) Operations

```c
size_t idcu_yaml_mapping_size(const idcu_YamlValue* value);
int idcu_yaml_mapping_has(const idcu_YamlValue* value, const char* key);
idcu_YamlValue* idcu_yaml_mapping_get(const idcu_YamlValue* value, const char* key);
```

## Serialization Functions

```c
int idcu_yaml_to_string(const idcu_YamlValue* value, char* buffer, size_t buffer_size);
char* idcu_yaml_to_string_alloc(const idcu_YamlValue* value);
int idcu_yaml_save_to_file(const idcu_YamlValue* value, const char* filepath);
```

## JSON Conversion

```c
int idcu_yaml_to_json(const idcu_YamlValue* value, char* buffer, size_t buffer_size);
char* idcu_yaml_to_json_alloc(const idcu_YamlValue* value);
```

## Example

```c
#include <idcu/yaml/yaml.h>
#include <stdio.h>

int main(void) {
    // Parse YAML
    const char* yaml_str = 
        "name: test\n"
        "value: 42\n"
        "items:\n"
        "  - item1\n"
        "  - item2\n";
    
    idcu_YamlValue value;
    int ret = idcu_yaml_parse(yaml_str, &value);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "Parse failed\n");
        return 1;
    }
    
    // Get values
    const char* name = NULL;
    idcu_yaml_get_string(idcu_yaml_mapping_get(&value, "name"), &name);
    printf("Name: %s\n", name);
    
    // Free
    idcu_yaml_free(&value);
    return 0;
}
```
