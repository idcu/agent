# idcu-json API 文档

JSON 解析器和序列化器库。

## JSON 值类型

```c
typedef enum {
    IDCU_JSON_TYPE_NULL,
    IDCU_JSON_TYPE_BOOL,
    IDCU_JSON_TYPE_INT,
    IDCU_JSON_TYPE_DOUBLE,
    IDCU_JSON_TYPE_STRING,
    IDCU_JSON_TYPE_ARRAY,
    IDCU_JSON_TYPE_OBJECT
} idcu_JsonType;
```

## JSON 值结构

```c
typedef struct idcu_JsonValue idcu_JsonValue;
```

## 解析函数

```c
int idcu_json_parse(const char* json_str, idcu_JsonValue** out_value);
int idcu_json_parse_file(const char* filepath, idcu_JsonValue** out_value);
void idcu_json_free(idcu_JsonValue* value);
```

## 类型检查

```c
idcu_JsonType idcu_json_get_type(const idcu_JsonValue* value);
int idcu_json_is_null(const idcu_JsonValue* value);
int idcu_json_is_bool(const idcu_JsonValue* value);
int idcu_json_is_int(const idcu_JsonValue* value);
int idcu_json_is_double(const idcu_JsonValue* value);
int idcu_json_is_string(const idcu_JsonValue* value);
int idcu_json_is_array(const idcu_JsonValue* value);
int idcu_json_is_object(const idcu_JsonValue* value);
```

## 值获取器

```c
int idcu_json_get_bool(const idcu_JsonValue* value, int* out_bool);
int idcu_json_get_int(const idcu_JsonValue* value, int64_t* out_int);
int idcu_json_get_double(const idcu_JsonValue* value, double* out_double);
int idcu_json_get_string(const idcu_JsonValue* value, const char** out_string);
```

## 数组操作

```c
size_t idcu_json_array_size(const idcu_JsonValue* value);
idcu_JsonValue* idcu_json_array_get(const idcu_JsonValue* value, size_t index);
```

## 对象操作

```c
size_t idcu_json_object_size(const idcu_JsonValue* value);
int idcu_json_object_has(const idcu_JsonValue* value, const char* key);
idcu_JsonValue* idcu_json_object_get(const idcu_JsonValue* value, const char* key);
const char* idcu_json_object_key_at(const idcu_JsonValue* value, size_t index);
idcu_JsonValue* idcu_json_object_value_at(const idcu_JsonValue* value, size_t index);
```

## 序列化函数

```c
int idcu_json_to_string(const idcu_JsonValue* value, char* buffer, size_t buffer_size, size_t* out_length);
char* idcu_json_to_string_alloc(const idcu_JsonValue* value);
int idcu_json_to_string_pretty(const idcu_JsonValue* value, char* buffer, size_t buffer_size, size_t* out_length);
char* idcu_json_to_string_pretty_alloc(const idcu_JsonValue* value);
int idcu_json_save_to_file(const idcu_JsonValue* value, const char* filepath);
```

## 构建器函数

```c
idcu_JsonValue* idcu_json_create_null(void);
idcu_JsonValue* idcu_json_create_bool(int value);
idcu_JsonValue* idcu_json_create_int(int64_t value);
idcu_JsonValue* idcu_json_create_double(double value);
idcu_JsonValue* idcu_json_create_string(const char* value);
idcu_JsonValue* idcu_json_create_array(void);
idcu_JsonValue* idcu_json_create_object(void);

int idcu_json_array_push(idcu_JsonValue* array, idcu_JsonValue* value);
int idcu_json_object_set(idcu_JsonValue* object, const char* key, idcu_JsonValue* value);
```

## 示例

```c
#include <idcu/json/json.h>
#include <stdio.h>

int main(void) {
    // 解析 JSON
    const char* json_str = "{\"name\":\"test\",\"value\":42}";
    idcu_JsonValue* root = NULL;
    
    int ret = idcu_json_parse(json_str, &root);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "解析失败\n");
        return 1;
    }
    
    // 获取值
    const char* name = NULL;
    idcu_json_get_string(idcu_json_object_get(root, "name"), &name);
    printf("名称: %s\n", name);
    
    int64_t value = 0;
    idcu_json_get_int(idcu_json_object_get(root, "value"), &value);
    printf("值: %" PRId64 "\n", value);
    
    // 释放
    idcu_json_free(root);
    return 0;
}
```
