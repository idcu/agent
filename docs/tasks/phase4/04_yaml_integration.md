# 任务 4.4: YAML 集成模块

## 目标

创建 YAML 集成模块，支持：
- 统一的 YAML 解析和序列化
- 与配置系统集成
- 与存储系统集成
- YAML 验证
- YAML Schema 验证
- YAML 与 JSON 互转
- YAML 合并
- YAML 补丁

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/yaml-integration/include/idcu/yaml_integration
mkdir -p modules/yaml-integration/src/idcu/yaml_integration
mkdir -p modules/yaml-integration/tests
mkdir -p modules/yaml-integration/examples
```

### 2. 创建 YAML 集成头文件 (yaml_integration.h)

创建 `modules/yaml-integration/include/idcu/yaml_integration/yaml_integration.h`：

```c
#ifndef IDCU_YAML_INTEGRATION_YAML_INTEGRATION_H
#define IDCU_YAML_INTEGRATION_YAML_INTEGRATION_H

#include "idcu/common/error_code.h"
#include "idcu/yaml/yaml.h"
#include "idcu/config/config.h"
#include "idcu/storage/storage.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_YAML_FORMAT_AUTO = 0,
    IDCU_YAML_FORMAT_PRETTY,
    IDCU_YAML_FORMAT_COMPACT
} idcu_YamlFormat;

typedef struct idcu_YamlMergeOptions
{
    int deep_merge;
    int overwrite_arrays;
    int concat_arrays;
    int preserve_null;
} idcu_YamlMergeOptions;

int  idcu_yaml_integration_parse(const char* yaml, idcu_YamlValue** result);
int  idcu_yaml_integration_parse_file(const char* path, idcu_YamlValue** result);
int  idcu_yaml_integration_serialize(const idcu_YamlValue* yaml, char* buffer, size_t buffer_size, idcu_YamlFormat format);
int  idcu_yaml_integration_serialize_file(const idcu_YamlValue* yaml, const char* path, idcu_YamlFormat format);

int  idcu_yaml_integration_to_config(const idcu_YamlValue* yaml, idcu_ConfigManager* config, const char* prefix);
int  idcu_yaml_integration_from_config(idcu_ConfigManager* config, const char* prefix, idcu_YamlValue** result);

int  idcu_yaml_integration_to_kvstore(const idcu_YamlValue* yaml, idcu_KVStore* store, const char* prefix);
int  idcu_yaml_integration_from_kvstore(idcu_KVStore* store, const char* prefix, idcu_YamlValue** result);

int  idcu_yaml_integration_to_json(const idcu_YamlValue* yaml, idcu_JsonValue** json);
int  idcu_yaml_integration_from_json(const idcu_JsonValue* json, idcu_YamlValue** yaml);

int  idcu_yaml_integration_get(const idcu_YamlValue* yaml, const char* path, char* buffer, size_t buffer_size);
int  idcu_yaml_integration_get_int(const idcu_YamlValue* yaml, const char* path, int64_t* value, int64_t default_value);
int  idcu_yaml_integration_get_double(const idcu_YamlValue* yaml, const char* path, double* value, double default_value);
int  idcu_yaml_integration_get_bool(const idcu_YamlValue* yaml, const char* path, int* value, int default_value);
idcu_YamlValue* idcu_yaml_integration_get_value(const idcu_YamlValue* yaml, const char* path);

int  idcu_yaml_integration_set(idcu_YamlValue* yaml, const char* path, const char* value);
int  idcu_yaml_integration_set_int(idcu_YamlValue* yaml, const char* path, int64_t value);
int  idcu_yaml_integration_set_double(idcu_YamlValue* yaml, const char* path, double value);
int  idcu_yaml_integration_set_bool(idcu_YamlValue* yaml, const char* path, int value);
int  idcu_yaml_integration_set_value(idcu_YamlValue* yaml, const char* path, const idcu_YamlValue* value);

int  idcu_yaml_integration_remove(idcu_YamlValue* yaml, const char* path);
int  idcu_yaml_integration_exists(const idcu_YamlValue* yaml, const char* path);
int  idcu_yaml_integration_type(const idcu_YamlValue* yaml, const char* path, idcu_YamlType* type);

int  idcu_yaml_integration_validate(const idcu_YamlValue* yaml, const idcu_YamlValue* schema);
int  idcu_yaml_integration_validate_file(const idcu_YamlValue* yaml, const char* schema_path);
int  idcu_yaml_integration_validate_str(const idcu_YamlValue* yaml, const char* schema_yaml);

int  idcu_yaml_integration_merge_options_init(idcu_YamlMergeOptions* options);
int  idcu_yaml_integration_merge(idcu_YamlValue* target, const idcu_YamlValue* source, const idcu_YamlMergeOptions* options);
int  idcu_yaml_integration_merge_all(idcu_YamlValue* target, const idcu_Vector* sources, const idcu_YamlMergeOptions* options);

int  idcu_yaml_integration_clone(const idcu_YamlValue* src, idcu_YamlValue** dst);
int  idcu_yaml_integration_deep_clone(const idcu_YamlValue* src, idcu_YamlValue** dst);

int  idcu_yaml_integration_compare(const idcu_YamlValue* a, const idcu_YamlValue* b, int* equal);
int  idcu_yaml_integration_hash(const idcu_YamlValue* yaml, uint64_t* hash);

int  idcu_yaml_integration_from_string(const char* str, idcu_YamlValue** result);
int  idcu_yaml_integration_from_int(int64_t value, idcu_YamlValue** result);
int  idcu_yaml_integration_from_double(double value, idcu_YamlValue** result);
int  idcu_yaml_integration_from_bool(int value, idcu_YamlValue** result);
int  idcu_yaml_integration_from_null(idcu_YamlValue** result);
int  idcu_yaml_integration_create_mapping(idcu_YamlValue** result);
int  idcu_yaml_integration_create_sequence(idcu_YamlValue** result);

int  idcu_yaml_integration_mapping_set(idcu_YamlValue* mapping, const char* key, const idcu_YamlValue* value);
int  idcu_yaml_integration_mapping_get(const idcu_YamlValue* mapping, const char* key, idcu_YamlValue** value);
int  idcu_yaml_integration_mapping_has(const idcu_YamlValue* mapping, const char* key);
int  idcu_yaml_integration_mapping_remove(idcu_YamlValue* mapping, const char* key);
size_t idcu_yaml_integration_mapping_size(const idcu_YamlValue* mapping);

int  idcu_yaml_integration_sequence_append(idcu_YamlValue* sequence, const idcu_YamlValue* value);
int  idcu_yaml_integration_sequence_insert(idcu_YamlValue* sequence, size_t index, const idcu_YamlValue* value);
int  idcu_yaml_integration_sequence_get(const idcu_YamlValue* sequence, size_t index, idcu_YamlValue** value);
int  idcu_yaml_integration_sequence_set(idcu_YamlValue* sequence, size_t index, const idcu_YamlValue* value);
int  idcu_yaml_integration_sequence_remove(idcu_YamlValue* sequence, size_t index);
size_t idcu_yaml_integration_sequence_size(const idcu_YamlValue* sequence);

int  idcu_yaml_integration_is_mapping(const idcu_YamlValue* yaml);
int  idcu_yaml_integration_is_sequence(const idcu_YamlValue* yaml);
int  idcu_yaml_integration_is_scalar(const idcu_YamlValue* yaml);
int  idcu_yaml_integration_is_string(const idcu_YamlValue* yaml);
int  idcu_yaml_integration_is_int(const idcu_YamlValue* yaml);
int  idcu_yaml_integration_is_double(const idcu_YamlValue* yaml);
int  idcu_yaml_integration_is_bool(const idcu_YamlValue* yaml);
int  idcu_yaml_integration_is_null(const idcu_YamlValue* yaml);

typedef struct
{
    const char* key;
    const idcu_YamlValue* value;
} idcu_YamlMappingIterator;

int  idcu_yaml_integration_mapping_iterator_init(idcu_YamlMappingIterator* iter, const idcu_YamlValue* mapping);
int  idcu_yaml_integration_mapping_iterator_next(idcu_YamlMappingIterator* iter);
int  idcu_yaml_integration_mapping_iterator_done(const idcu_YamlMappingIterator* iter);

int  idcu_yaml_integration_load_with_overrides(const char* base_path, const char* override_path, idcu_YamlValue** result);
int  idcu_yaml_integration_load_with_env(const char* path, const char* env_prefix, idcu_YamlValue** result);

int  idcu_yaml_integration_anchor_resolve(idcu_YamlValue* yaml);
int  idcu_yaml_integration_tag_process(idcu_YamlValue* yaml);

int  idcu_yaml_integration_comment_strip(const char* input, char* output, size_t* output_len);
int  idcu_yaml_integration_minify(const char* input, char* output, size_t* output_len);
int  idcu_yaml_integration_prettify(const char* input, char* output, size_t* output_len, int indent);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `modules/yaml-integration/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(yaml-integration VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(yaml-integration STATIC
    src/idcu/yaml_integration/yaml_integration.c
    src/idcu/yaml_integration/yamljson.c
    src/idcu/yaml_integration/yamlmerge.c
)

target_include_directories(yaml-integration PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(yaml-integration PRIVATE
    idcu::common
    idcu::yaml
    idcu::json
    idcu::config
    idcu::storage
    idcu::utils
    idcu::log
)

add_library(idcu::yaml-integration ALIAS yaml-integration)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `modules/yaml-integration/module.yaml`：

```yaml
name: yaml-integration
version: 1.0.0
description: YAML integration module for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-yaml
  - idcu-json
  - idcu-config
  - idcu-storage
  - idcu-utils
  - idcu-log

build:
  type: cmake
  targets:
    - yaml-integration

headers:
  - idcu/yaml_integration/yaml_integration.h

features:
  - parse: Unified YAML parsing and serialization
  - config: Integration with config system
  - storage: Integration with storage system
  - json: YAML to JSON conversion
  - validation: YAML validation
  - merge: YAML merge
  - patch: YAML patch
  - anchors: YAML anchor support
  - tags: YAML tag support

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `modules/yaml-integration/README.md`：

```markdown
# yaml-integration

IDCU Agent 的 YAML 集成模块。

## 功能特性

- **统一解析**: 统一的 YAML 解析和序列化
- **配置集成**: 与配置系统集成
- **存储集成**: 与存储系统集成
- **JSON 互转**: YAML 与 JSON 互转
- **验证**: YAML 验证
- **合并**: YAML 合并
- **补丁**: YAML 补丁
- **锚点**: YAML 锚点支持
- **标签**: YAML 标签支持

## 快速开始

### 解析和序列化

```c
#include "idcu/yaml_integration/yaml_integration.h"

const char* yaml_str = "name: John\nage: 30\nactive: true";

idcu_YamlValue* yaml;
idcu_yaml_integration_parse(yaml_str, &yaml);

char buffer[1024];
idcu_yaml_integration_serialize(yaml, buffer, sizeof(buffer), IDCU_YAML_FORMAT_PRETTY);
printf("%s\n", buffer);

idcu_yaml_value_destroy(yaml);
```

### 从文件解析

```c
idcu_YamlValue* yaml;
idcu_yaml_integration_parse_file("config.yaml", &yaml);

idcu_yaml_integration_serialize_file(yaml, "output.yaml", IDCU_YAML_FORMAT_PRETTY);

idcu_yaml_value_destroy(yaml);
```

### 路径访问

```c
idcu_YamlValue* yaml;
idcu_yaml_integration_parse_file("config.yaml", &yaml);

char name[256];
idcu_yaml_integration_get(yaml, "user.name", name, sizeof(name));
printf("Name: %s\n", name);

int64_t age;
idcu_yaml_integration_get_int(yaml, "user.age", &age, 0);
printf("Age: %" PRId64 "\n", age);

double score;
idcu_yaml_integration_get_double(yaml, "user.score", &score, 0.0);
printf("Score: %f\n", score);

int active;
idcu_yaml_integration_get_bool(yaml, "user.active", &active, 0);
printf("Active: %d\n", active);

idcu_yaml_value_destroy(yaml);
```

### 路径设置

```c
idcu_YamlValue* yaml;
idcu_yaml_integration_create_mapping(&yaml);

idcu_yaml_integration_set(yaml, "user.name", "Jane");
idcu_yaml_integration_set_int(yaml, "user.age", 25);
idcu_yaml_integration_set_double(yaml, "user.score", 95.5);
idcu_yaml_integration_set_bool(yaml, "user.active", 1);

idcu_yaml_value_destroy(yaml);
```

### 与配置系统集成

```c
idcu_YamlValue* config_yaml;
idcu_yaml_integration_parse_file("config.yaml", &config_yaml);

idcu_ConfigManager config;
idcu_config_manager_init(&config);

idcu_yaml_integration_to_config(config_yaml, &config, "app.");

idcu_YamlValue* exported_yaml;
idcu_yaml_integration_from_config(&config, "app.", &exported_yaml);

idcu_yaml_value_destroy(config_yaml);
idcu_yaml_value_destroy(exported_yaml);
idcu_config_manager_destroy(&config);
```

### 与存储系统集成

```c
idcu_YamlValue* data_yaml;
idcu_yaml_integration_parse_file("data.yaml", &data_yaml);

idcu_KVStore store;
idcu_kvstore_init(&store, "./data.kv");

idcu_yaml_integration_to_kvstore(data_yaml, &store, "data.");

idcu_YamlValue* exported_yaml;
idcu_yaml_integration_from_kvstore(&store, "data.", &exported_yaml);

idcu_yaml_value_destroy(data_yaml);
idcu_yaml_value_destroy(exported_yaml);
idcu_kvstore_destroy(&store);
```

### 与 JSON 互转

```c
idcu_YamlValue* yaml;
idcu_yaml_integration_parse_file("data.yaml", &yaml);

idcu_JsonValue* json;
idcu_yaml_integration_to_json(yaml, &json);

char json_buffer[2048];
idcu_json_value_serialize(json, json_buffer, sizeof(json_buffer), 1);
printf("JSON: %s\n", json_buffer);

idcu_YamlValue* yaml_from_json;
idcu_yaml_integration_from_json(json, &yaml_from_json);

idcu_yaml_value_destroy(yaml);
idcu_json_value_destroy(json);
idcu_yaml_value_destroy(yaml_from_json);
```

### YAML 合并

```c
idcu_YamlValue* base;
idcu_yaml_integration_parse_file("base.yaml", &base);

idcu_YamlValue* override;
idcu_yaml_integration_parse_file("override.yaml", &override);

idcu_YamlMergeOptions options;
idcu_yaml_integration_merge_options_init(&options);
options.deep_merge = 1;
options.overwrite_arrays = 1;

idcu_yaml_integration_merge(base, override, &options);

idcu_yaml_value_destroy(override);
idcu_yaml_value_destroy(base);
```

### Mapping 操作

```c
idcu_YamlValue* mapping;
idcu_yaml_integration_create_mapping(&mapping);

idcu_YamlValue* name;
idcu_yaml_integration_from_string("John", &name);
idcu_yaml_integration_mapping_set(mapping, "name", name);

idcu_YamlValue* age;
idcu_yaml_integration_from_int(30, &age);
idcu_yaml_integration_mapping_set(mapping, "age", age);

if (idcu_yaml_integration_mapping_has(mapping, "name")) {
    idcu_YamlValue* value;
    idcu_yaml_integration_mapping_get(mapping, "name", &value);
    printf("Name: %s\n", idcu_yaml_value_get_string(value));
}

size_t size = idcu_yaml_integration_mapping_size(mapping);
printf("Mapping size: %zu\n", size);

idcu_yaml_value_destroy(mapping);
```

### Sequence 操作

```c
idcu_YamlValue* sequence;
idcu_yaml_integration_create_sequence(&sequence);

idcu_YamlValue* item1;
idcu_yaml_integration_from_string("first", &item1);
idcu_yaml_integration_sequence_append(sequence, item1);

idcu_YamlValue* item2;
idcu_yaml_integration_from_string("second", &item2);
idcu_yaml_integration_sequence_append(sequence, item2);

size_t size = idcu_yaml_integration_sequence_size(sequence);
printf("Sequence size: %zu\n", size);

idcu_YamlValue* value;
idcu_yaml_integration_sequence_get(sequence, 0, &value);
printf("First item: %s\n", idcu_yaml_value_get_string(value));

idcu_yaml_value_destroy(sequence);
```

### 带覆盖的加载

```c
idcu_YamlValue* merged;
idcu_yaml_integration_load_with_overrides("base.yaml", "override.yaml", &merged);

idcu_yaml_value_destroy(merged);
```

### 带环境变量的加载

```c
idcu_YamlValue* yaml;
idcu_yaml_integration_load_with_env("config.yaml", "APP_", &yaml);

idcu_yaml_value_destroy(yaml);
```

### 美化和压缩

```c
const char* compact = "name:John age:30";
char pretty[1024];
size_t pretty_len;

idcu_yaml_integration_prettify(compact, pretty, &pretty_len, 2);
printf("Pretty: %s\n", pretty);

const char* pretty_yaml = "name: John\nage: 30";
char compact_yaml[256];
size_t compact_len;

idcu_yaml_integration_minify(pretty_yaml, compact_yaml, &compact_len);
printf("Compact: %s\n", compact_yaml);
```

## YAML 格式

| 格式 | 说明 |
|-----|------|
| AUTO | 自动 |
| PRETTY | 美化格式 |
| COMPACT | 压缩格式 |

## API 文档

详见 [include/idcu/yaml_integration/yaml_integration.h](include/idcu/yaml_integration/yaml_integration.h)
```

## 验证检查清单

- [ ] YAML 集成头文件已创建
- [ ] YAML 集成实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以解析和序列化 YAML
- [ ] 与配置系统集成正常工作
- [ ] 与 JSON 互转正常工作

## Git 提交

```bash
git add modules/yaml-integration/
git commit -m "feat: add yaml-integration module

- Add unified YAML parsing and serialization
- Add integration with config system
- Add integration with storage system
- Add YAML to JSON conversion
- Add YAML validation
- Add YAML merge
- Add YAML patch
- Add YAML anchor support
- Add YAML tag support
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 解析失败 | YAML 格式错误 | 检查 YAML 语法 |
| 路径不存在 | 路径错误 | 确保路径正确 |
| JSON 转换失败 | 类型不兼容 | 检查 YAML 类型 |
