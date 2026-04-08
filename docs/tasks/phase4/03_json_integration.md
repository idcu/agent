# 任务 4.3: JSON 集成模块

## 目标

创建 JSON 集成模块，支持：
- 统一的 JSON 解析和序列化
- 与配置系统集成
- 与存储系统集成
- 与网络系统集成
- JSON Schema 验证
- JSON 补丁
- JSON 合并
- JSON 查询 (JSONPath)

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/json-integration/include/idcu/json_integration
mkdir -p modules/json-integration/src/idcu/json_integration
mkdir -p modules/json-integration/tests
mkdir -p modules/json-integration/examples
```

### 2. 创建 JSON 集成头文件 (json_integration.h)

创建 `modules/json-integration/include/idcu/json_integration/json_integration.h`：

```c
#ifndef IDCU_JSON_INTEGRATION_JSON_INTEGRATION_H
#define IDCU_JSON_INTEGRATION_JSON_INTEGRATION_H

#include "idcu/common/error_code.h"
#include "idcu/json/json.h"
#include "idcu/config/config.h"
#include "idcu/storage/storage.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_JSON_TYPE_AUTO = 0,
    IDCU_JSON_TYPE_PRETTY,
    IDCU_JSON_TYPE_COMPACT
} idcu_JsonFormat;

typedef struct idcu_JsonPatch
{
    char op[32];
    char path[256];
    char value[1024];
    char from[256];
} idcu_JsonPatch;

typedef struct idcu_JsonMergeOptions
{
    int deep_merge;
    int overwrite_arrays;
    int concat_arrays;
    int preserve_null;
} idcu_JsonMergeOptions;

int  idcu_json_integration_parse(const char* json, idcu_JsonValue** result);
int  idcu_json_integration_parse_file(const char* path, idcu_JsonValue** result);
int  idcu_json_integration_serialize(const idcu_JsonValue* json, char* buffer, size_t buffer_size, idcu_JsonFormat format);
int  idcu_json_integration_serialize_file(const idcu_JsonValue* json, const char* path, idcu_JsonFormat format);

int  idcu_json_integration_to_config(const idcu_JsonValue* json, idcu_ConfigManager* config, const char* prefix);
int  idcu_json_integration_from_config(idcu_ConfigManager* config, const char* prefix, idcu_JsonValue** result);

int  idcu_json_integration_to_kvstore(const idcu_JsonValue* json, idcu_KVStore* store, const char* prefix);
int  idcu_json_integration_from_kvstore(idcu_KVStore* store, const char* prefix, idcu_JsonValue** result);

int  idcu_json_integration_get(const idcu_JsonValue* json, const char* path, char* buffer, size_t buffer_size);
int  idcu_json_integration_get_int(const idcu_JsonValue* json, const char* path, int64_t* value, int64_t default_value);
int  idcu_json_integration_get_double(const idcu_JsonValue* json, const char* path, double* value, double default_value);
int  idcu_json_integration_get_bool(const idcu_JsonValue* json, const char* path, int* value, int default_value);
idcu_JsonValue* idcu_json_integration_get_value(const idcu_JsonValue* json, const char* path);

int  idcu_json_integration_set(idcu_JsonValue* json, const char* path, const char* value);
int  idcu_json_integration_set_int(idcu_JsonValue* json, const char* path, int64_t value);
int  idcu_json_integration_set_double(idcu_JsonValue* json, const char* path, double value);
int  idcu_json_integration_set_bool(idcu_JsonValue* json, const char* path, int value);
int  idcu_json_integration_set_value(idcu_JsonValue* json, const char* path, const idcu_JsonValue* value);

int  idcu_json_integration_remove(idcu_JsonValue* json, const char* path);
int  idcu_json_integration_exists(const idcu_JsonValue* json, const char* path);
int  idcu_json_integration_type(const idcu_JsonValue* json, const char* path, idcu_JsonType* type);

int  idcu_json_integration_query(const idcu_JsonValue* json, const char* jsonpath, idcu_Vector* results);
int  idcu_json_integration_query_single(const idcu_JsonValue* json, const char* jsonpath, idcu_JsonValue** result);

int  idcu_json_integration_validate(const idcu_JsonValue* json, const idcu_JsonValue* schema);
int  idcu_json_integration_validate_file(const idcu_JsonValue* json, const char* schema_path);
int  idcu_json_integration_validate_str(const idcu_JsonValue* json, const char* schema_json);

int  idcu_json_integration_patch_init(idcu_JsonPatch* patch);
void idcu_json_integration_patch_destroy(idcu_JsonPatch* patch);
int  idcu_json_integration_patch_add(idcu_JsonPatch* patch, const char* path, const char* value);
int  idcu_json_integration_patch_remove(idcu_JsonPatch* patch, const char* path);
int  idcu_json_integration_patch_replace(idcu_JsonPatch* patch, const char* path, const char* value);
int  idcu_json_integration_patch_move(idcu_JsonPatch* patch, const char* from, const char* path);
int  idcu_json_integration_patch_copy(idcu_JsonPatch* patch, const char* from, const char* path);
int  idcu_json_integration_patch_test(idcu_JsonPatch* patch, const char* path, const char* value);

int  idcu_json_integration_apply_patch(idcu_JsonValue* json, const idcu_JsonPatch* patch);
int  idcu_json_integration_apply_patches(idcu_JsonValue* json, const idcu_Vector* patches);
int  idcu_json_integration_diff(const idcu_JsonValue* original, const idcu_JsonValue* modified, idcu_Vector* patches);

int  idcu_json_integration_merge_options_init(idcu_JsonMergeOptions* options);
int  idcu_json_integration_merge(idcu_JsonValue* target, const idcu_JsonValue* source, const idcu_JsonMergeOptions* options);
int  idcu_json_integration_merge_all(idcu_JsonValue* target, const idcu_Vector* sources, const idcu_JsonMergeOptions* options);

int  idcu_json_integration_clone(const idcu_JsonValue* src, idcu_JsonValue** dst);
int  idcu_json_integration_deep_clone(const idcu_JsonValue* src, idcu_JsonValue** dst);

int  idcu_json_integration_compare(const idcu_JsonValue* a, const idcu_JsonValue* b, int* equal);
int  idcu_json_integration_hash(const idcu_JsonValue* json, uint64_t* hash);

int  idcu_json_integration_minify(const char* input, char* output, size_t* output_len);
int  idcu_json_integration_prettify(const char* input, char* output, size_t* output_len, int indent);

int  idcu_json_integration_escape(const char* input, char* output, size_t* output_len);
int  idcu_json_integration_unescape(const char* input, char* output, size_t* output_len);

int  idcu_json_integration_from_string(const char* str, idcu_JsonValue** result);
int  idcu_json_integration_from_int(int64_t value, idcu_JsonValue** result);
int  idcu_json_integration_from_double(double value, idcu_JsonValue** result);
int  idcu_json_integration_from_bool(int value, idcu_JsonValue** result);
int  idcu_json_integration_from_null(idcu_JsonValue** result);
int  idcu_json_integration_create_object(idcu_JsonValue** result);
int  idcu_json_integration_create_array(idcu_JsonValue** result);

int  idcu_json_integration_object_set(idcu_JsonValue* object, const char* key, const idcu_JsonValue* value);
int  idcu_json_integration_object_get(const idcu_JsonValue* object, const char* key, idcu_JsonValue** value);
int  idcu_json_integration_object_has(const idcu_JsonValue* object, const char* key);
int  idcu_json_integration_object_remove(idcu_JsonValue* object, const char* key);
size_t idcu_json_integration_object_size(const idcu_JsonValue* object);

int  idcu_json_integration_array_append(idcu_JsonValue* array, const idcu_JsonValue* value);
int  idcu_json_integration_array_insert(idcu_JsonValue* array, size_t index, const idcu_JsonValue* value);
int  idcu_json_integration_array_get(const idcu_JsonValue* array, size_t index, idcu_JsonValue** value);
int  idcu_json_integration_array_set(idcu_JsonValue* array, size_t index, const idcu_JsonValue* value);
int  idcu_json_integration_array_remove(idcu_JsonValue* array, size_t index);
size_t idcu_json_integration_array_size(const idcu_JsonValue* array);

int  idcu_json_integration_is_object(const idcu_JsonValue* json);
int  idcu_json_integration_is_array(const idcu_JsonValue* json);
int  idcu_json_integration_is_string(const idcu_JsonValue* json);
int  idcu_json_integration_is_int(const idcu_JsonValue* json);
int  idcu_json_integration_is_double(const idcu_JsonValue* json);
int  idcu_json_integration_is_bool(const idcu_JsonValue* json);
int  idcu_json_integration_is_null(const idcu_JsonValue* json);

typedef struct
{
    const char* key;
    const idcu_JsonValue* value;
} idcu_JsonObjectIterator;

int  idcu_json_integration_object_iterator_init(idcu_JsonObjectIterator* iter, const idcu_JsonValue* object);
int  idcu_json_integration_object_iterator_next(idcu_JsonObjectIterator* iter);
int  idcu_json_integration_object_iterator_done(const idcu_JsonObjectIterator* iter);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `modules/json-integration/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(json-integration VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(json-integration STATIC
    src/idcu/json_integration/json_integration.c
    src/idcu/json_integration/jsonpath.c
    src/idcu/json_integration/jsonschema.c
    src/idcu/json_integration/jsonpatch.c
    src/idcu/json_integration/jsonmerge.c
)

target_include_directories(json-integration PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(json-integration PRIVATE
    idcu::common
    idcu::json
    idcu::config
    idcu::storage
    idcu::utils
    idcu::log
)

add_library(idcu::json-integration ALIAS json-integration)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `modules/json-integration/module.yaml`：

```yaml
name: json-integration
version: 1.0.0
description: JSON integration module for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-json
  - idcu-config
  - idcu-storage
  - idcu-utils
  - idcu-log

build:
  type: cmake
  targets:
    - json-integration

headers:
  - idcu/json_integration/json_integration.h

features:
  - parse: Unified JSON parsing and serialization
  - config: Integration with config system
  - storage: Integration with storage system
  - network: Integration with network system
  - schema: JSON Schema validation
  - patch: JSON Patch (RFC 6902)
  - merge: JSON Merge Patch (RFC 7386)
  - query: JSON Query (JSONPath)
  - transform: JSON transformation
  - diff: JSON diff

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `modules/json-integration/README.md`：

```markdown
# json-integration

IDCU Agent 的 JSON 集成模块。

## 功能特性

- **统一解析**: 统一的 JSON 解析和序列化
- **配置集成**: 与配置系统集成
- **存储集成**: 与存储系统集成
- **网络集成**: 与网络系统集成
- **Schema 验证**: JSON Schema 验证
- **JSON 补丁**: JSON 补丁
- **JSON 合并**: JSON 合并
- **JSON 查询**: JSON 查询 (JSONPath)
- **JSON 转换**: JSON 转换
- **JSON 差异**: JSON 差异

## 快速开始

### 解析和序列化

```c
#include "idcu/json_integration/json_integration.h"

const char* json_str = "{"
    "\"name\": \"John\","
    "\"age\": 30,"
    "\"active\": true"
"}";

idcu_JsonValue* json;
idcu_json_integration_parse(json_str, &json);

char buffer[1024];
idcu_json_integration_serialize(json, buffer, sizeof(buffer), IDCU_JSON_TYPE_PRETTY);
printf("%s\n", buffer);

idcu_json_value_destroy(json);
```

### 从文件解析

```c
idcu_JsonValue* json;
idcu_json_integration_parse_file("data.json", &json);

idcu_json_integration_serialize_file(json, "output.json", IDCU_JSON_TYPE_COMPACT);

idcu_json_value_destroy(json);
```

### 路径访问

```c
idcu_JsonValue* json;
idcu_json_integration_parse_file("data.json", &json);

char name[256];
idcu_json_integration_get(json, "user.name", name, sizeof(name));
printf("Name: %s\n", name);

int64_t age;
idcu_json_integration_get_int(json, "user.age", &age, 0);
printf("Age: %" PRId64 "\n", age);

double score;
idcu_json_integration_get_double(json, "user.score", &score, 0.0);
printf("Score: %f\n", score);

int active;
idcu_json_integration_get_bool(json, "user.active", &active, 0);
printf("Active: %d\n", active);

idcu_json_value_destroy(json);
```

### 路径设置

```c
idcu_JsonValue* json;
idcu_json_integration_create_object(&json);

idcu_json_integration_set(json, "user.name", "Jane");
idcu_json_integration_set_int(json, "user.age", 25);
idcu_json_integration_set_double(json, "user.score", 95.5);
idcu_json_integration_set_bool(json, "user.active", 1);

idcu_json_value_destroy(json);
```

### 与配置系统集成

```c
idcu_JsonValue* config_json;
idcu_json_integration_parse_file("config.json", &config_json);

idcu_ConfigManager config;
idcu_config_manager_init(&config);

idcu_json_integration_to_config(config_json, &config, "app.");

idcu_JsonValue* exported_json;
idcu_json_integration_from_config(&config, "app.", &exported_json);

idcu_json_value_destroy(config_json);
idcu_json_value_destroy(exported_json);
idcu_config_manager_destroy(&config);
```

### 与存储系统集成

```c
idcu_JsonValue* data_json;
idcu_json_integration_parse_file("data.json", &data_json);

idcu_KVStore store;
idcu_kvstore_init(&store, "./data.kv");

idcu_json_integration_to_kvstore(data_json, &store, "data.");

idcu_JsonValue* exported_json;
idcu_json_integration_from_kvstore(&store, "data.", &exported_json);

idcu_json_value_destroy(data_json);
idcu_json_value_destroy(exported_json);
idcu_kvstore_destroy(&store);
```

### JSONPath 查询

```c
idcu_JsonValue* json;
idcu_json_integration_parse_file("data.json", &json);

idcu_Vector results;
idcu_vector_init(&results, sizeof(idcu_JsonValue*));

idcu_json_integration_query(json, "$.users[*].name", &results);

for (size_t i = 0; i < results.count; i++) {
    idcu_JsonValue** value_ptr = (idcu_JsonValue**)idcu_vector_get(&results, i);
    const char* name = idcu_json_value_get_string(*value_ptr);
    printf("User: %s\n", name);
}

idcu_vector_destroy(&results);
idcu_json_value_destroy(json);
```

### JSON Schema 验证

```c
idcu_JsonValue* data;
idcu_json_integration_parse_file("data.json", &data);

idcu_JsonValue* schema;
idcu_json_integration_parse_file("schema.json", &schema);

if (idcu_json_integration_validate(data, schema) == IDCU_ERR_OK) {
    printf("Data is valid\n");
} else {
    printf("Data is invalid\n");
}

idcu_json_value_destroy(data);
idcu_json_value_destroy(schema);
```

### JSON 补丁

```c
idcu_JsonValue* json;
idcu_json_integration_parse_file("data.json", &json);

idcu_JsonPatch patch;
idcu_json_integration_patch_init(&patch);

idcu_json_integration_patch_add(&patch, "/user/email", "john@example.com");
idcu_json_integration_patch_replace(&patch, "/user/age", "31");
idcu_json_integration_patch_remove(&patch, "/user/temp");

idcu_json_integration_apply_patch(json, &patch);

idcu_json_integration_patch_destroy(&patch);
idcu_json_value_destroy(json);
```

### JSON 合并

```c
idcu_JsonValue* base;
idcu_json_integration_parse_file("base.json", &base);

idcu_JsonValue* override;
idcu_json_integration_parse_file("override.json", &override);

idcu_JsonMergeOptions options;
idcu_json_integration_merge_options_init(&options);
options.deep_merge = 1;
options.overwrite_arrays = 1;

idcu_json_integration_merge(base, override, &options);

idcu_json_value_destroy(override);
idcu_json_value_destroy(base);
```

### 对象操作

```c
idcu_JsonValue* obj;
idcu_json_integration_create_object(&obj);

idcu_JsonValue* name;
idcu_json_integration_from_string("John", &name);
idcu_json_integration_object_set(obj, "name", name);

idcu_JsonValue* age;
idcu_json_integration_from_int(30, &age);
idcu_json_integration_object_set(obj, "age", age);

if (idcu_json_integration_object_has(obj, "name")) {
    idcu_JsonValue* value;
    idcu_json_integration_object_get(obj, "name", &value);
    printf("Name: %s\n", idcu_json_value_get_string(value));
}

size_t size = idcu_json_integration_object_size(obj);
printf("Object size: %zu\n", size);

idcu_json_value_destroy(obj);
```

### 数组操作

```c
idcu_JsonValue* arr;
idcu_json_integration_create_array(&arr);

idcu_JsonValue* item1;
idcu_json_integration_from_string("first", &item1);
idcu_json_integration_array_append(arr, item1);

idcu_JsonValue* item2;
idcu_json_integration_from_string("second", &item2);
idcu_json_integration_array_append(arr, item2);

size_t size = idcu_json_integration_array_size(arr);
printf("Array size: %zu\n", size);

idcu_JsonValue* value;
idcu_json_integration_array_get(arr, 0, &value);
printf("First item: %s\n", idcu_json_value_get_string(value));

idcu_json_value_destroy(arr);
```

### 美化和压缩

```c
const char* compact = "{\"name\":\"John\",\"age\":30}";
char pretty[1024];
size_t pretty_len;

idcu_json_integration_prettify(compact, pretty, &pretty_len, 2);
printf("Pretty: %s\n", pretty);

const char* pretty_json = "{\n  \"name\": \"John\"\n}";
char compact_json[256];
size_t compact_len;

idcu_json_integration_minify(pretty_json, compact_json, &compact_len);
printf("Compact: %s\n", compact_json);
```

## JSON 格式

| 格式 | 说明 |
|-----|------|
| AUTO | 自动 |
| PRETTY | 美化格式 |
| COMPACT | 压缩格式 |

## API 文档

详见 [include/idcu/json_integration/json_integration.h](include/idcu/json_integration/json_integration.h)
```

## 验证检查清单

- [ ] JSON 集成头文件已创建
- [ ] JSON 集成实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以解析和序列化 JSON
- [ ] 与配置系统集成正常工作
- [ ] JSONPath 查询正常工作

## Git 提交

```bash
git add modules/json-integration/
git commit -m "feat: add json-integration module

- Add unified JSON parsing and serialization
- Add integration with config system
- Add integration with storage system
- Add integration with network system
- Add JSON Schema validation
- Add JSON Patch (RFC 6902)
- Add JSON Merge Patch (RFC 7386)
- Add JSON Query (JSONPath)
- Add JSON transformation
- Add JSON diff
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 解析失败 | JSON 格式错误 | 检查 JSON 语法 |
| 路径不存在 | 路径错误 | 确保路径正确 |
| Schema 验证失败 | 数据不符合 schema | 检查数据和 schema |
