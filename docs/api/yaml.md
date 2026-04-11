# idcu-yaml API 文档

YAML 解析器和序列化器库。

## YAML 值类型

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

## YAML 值结构

```c
typedef struct idcu_YamlValue idcu_YamlValue;
```

## 解析函数

```c
int idcu_yaml_parse(const char* yaml_str, idcu_YamlValue* out_value);
int idcu_yaml_parse_file(const char* filepath, idcu_YamlValue* out_value);
void idcu_yaml_free(idcu_YamlValue* value);
```

## 类型检查

```c
idcu_YamlType idcu_yaml_get_type(const idcu_YamlValue* value);
```

## 值获取器

```c
int idcu_yaml_get_bool(const idcu_YamlValue* value, int* out_bool);
int idcu_yaml_get_int(const idcu_YamlValue* value, int64_t* out_int);
int idcu_yaml_get_double(const idcu_YamlValue* value, double* out_double);
int idcu_yaml_get_string(const idcu_YamlValue* value, const char** out_string);
```

## 序列（数组）操作

```c
size_t idcu_yaml_sequence_size(const idcu_YamlValue* value);
idcu_YamlValue* idcu_yaml_sequence_get(const idcu_YamlValue* value, size_t index);
```

## 映射（对象）操作

```c
size_t idcu_yaml_mapping_size(const idcu_YamlValue* value);
int idcu_yaml_mapping_has(const idcu_YamlValue* value, const char* key);
idcu_YamlValue* idcu_yaml_mapping_get(const idcu_YamlValue* value, const char* key);
```

## 序列化函数

```c
int idcu_yaml_to_string(const idcu_YamlValue* value, char* buffer, size_t buffer_size);
char* idcu_yaml_to_string_alloc(const idcu_YamlValue* value);
int idcu_yaml_save_to_file(const idcu_YamlValue* value, const char* filepath);
```

## JSON 转换

```c
int idcu_yaml_to_json(const idcu_YamlValue* value, char* buffer, size_t buffer_size);
char* idcu_yaml_to_json_alloc(const idcu_YamlValue* value);
```

## 示例

```c
#include <idcu/yaml/yaml.h>
#include <stdio.h>

int main(void) {
    // 解析 YAML
    const char* yaml_str = 
        "name: test\n"
        "value: 42\n"
        "items:\n"
        "  - item1\n"
        "  - item2\n";
    
    idcu_YamlValue value;
    int ret = idcu_yaml_parse(yaml_str, &value);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "解析失败\n");
        return 1;
    }
    
    // 获取值
    const char* name = NULL;
    idcu_yaml_get_string(idcu_yaml_mapping_get(&value, "name"), &name);
    printf("名称: %s\n", name);
    
    // 释放
    idcu_yaml_free(&value);
    return 0;
}
```
