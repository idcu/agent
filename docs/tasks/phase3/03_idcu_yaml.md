# 任务 3.3: idcu-yaml - YAML 解析库

## 目标

创建简化版的 YAML 解析库，支持：
- 基本 YAML 语法解析
- 键值对、列表、嵌套结构
- 与 idcu-json 类似的 API
- 类型安全的访问
- 内存管理

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-yaml/include/idcu/yaml
mkdir -p libs/idcu-yaml/src/idcu/yaml
mkdir -p libs/idcu-yaml/tests
mkdir -p libs/idcu-yaml/examples
```

### 2. 创建 YAML 头文件 (yaml.h)

创建 `libs/idcu-yaml/include/idcu/yaml/yaml.h`：

```c
#ifndef IDCU_YAML_YAML_H
#define IDCU_YAML_YAML_H

#include "idcu/common/error_code.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
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

struct idcu_YamlSequence
{
    idcu_YamlValue* elements;
    size_t          count;
    size_t          capacity;
};

struct idcu_YamlMapping
{
    char**          keys;
    idcu_YamlValue* values;
    size_t          count;
    size_t          capacity;
};

struct idcu_YamlValue
{
    idcu_YamlType type;
    union
    {
        int               bool_val;
        int64_t           int_val;
        double            double_val;
        char*             string_val;
        idcu_YamlSequence* sequence_val;
        idcu_YamlMapping*  mapping_val;
    } data;
};

int  idcu_yaml_parse(const char* yaml_str, idcu_YamlValue* result);
int  idcu_yaml_parse_file(const char* file_path, idcu_YamlValue* result);
void idcu_yaml_free(idcu_YamlValue* value);

idcu_YamlType idcu_yaml_get_type(const idcu_YamlValue* value);

int idcu_yaml_get_bool(const idcu_YamlValue* value, int* out);
int idcu_yaml_get_int(const idcu_YamlValue* value, int64_t* out);
int idcu_yaml_get_double(const idcu_YamlValue* value, double* out);
int idcu_yaml_get_string(const idcu_YamlValue* value, const char** out);

size_t           idcu_yaml_sequence_size(const idcu_YamlSequence* seq);
idcu_YamlValue*  idcu_yaml_sequence_get(const idcu_YamlSequence* seq, size_t index);

idcu_YamlValue*  idcu_yaml_mapping_get(const idcu_YamlMapping* map, const char* key);
int              idcu_yaml_mapping_has(const idcu_YamlMapping* map, const char* key);

int   idcu_yaml_to_string(const idcu_YamlValue* value, char* buffer, size_t buffer_size);
char* idcu_yaml_to_string_alloc(const idcu_YamlValue* value);
int   idcu_yaml_save_to_file(const idcu_YamlValue* value, const char* file_path);

int idcu_yaml_to_json(const idcu_YamlValue* yaml_value, char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `libs/idcu-yaml/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-yaml VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-yaml STATIC
    src/idcu/yaml/yaml.c
)

target_include_directories(idcu-yaml PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-yaml PRIVATE
    idcu::common
)

add_library(idcu::yaml ALIAS idcu-yaml)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-yaml/module.yaml`：

```yaml
name: idcu-yaml
version: 1.0.0
description: YAML parsing library for IDCU Agent (simplified version)
author: IDCU Team
license: MIT

dependencies:
  - idcu-common

build:
  type: cmake
  targets:
    - idcu-yaml

headers:
  - idcu/yaml/yaml.h

features:
  - parse: YAML parsing from string or file
  - serialize: YAML serialization to string or file
  - types: Support scalar, sequence, and mapping types
  - type_safe: Type-safe accessor functions
  - json_convert: Convert YAML to JSON

testing:
  enabled: true
  framework: internal
```

### 5. 创建示例 YAML 配置

创建 `config/example.yaml`（示例）：

```yaml
name: "idcu-agent"
version: "1.0.0"
enabled: true
port: 8080
threshold: 0.75

modules:
  - core
  - log
  - metrics

logging:
  level: "info"
  outputs:
    - console
    - file

database:
  host: "localhost"
  port: 5432
  name: "idcu"
  credentials:
    username: "admin"
    password: "secret"
```

### 6. 创建 README.md

创建 `libs/idcu-yaml/README.md`：

```markdown
# idcu-yaml

IDCU Agent 的简化版 YAML 解析库。

## 功能特性

- **YAML 解析**: 从字符串或文件解析 YAML
- **YAML 序列化**: 序列化为字符串或保存到文件
- **类型支持**: scalar, sequence, mapping
- **类型安全访问**: 类型安全的访问函数
- **JSON 转换**: 可转换为 JSON 格式

## 快速开始

### 解析 YAML

```c
#include "idcu/yaml/yaml.h"

idcu_YamlValue root;
int ret = idcu_yaml_parse("name: test\nvalue: 42", &root);
if (ret != IDCU_ERR_OK) {
    printf("Parse failed: %s\n", idcu_err_to_str(ret));
    return -1;
}

idcu_YamlMapping* map = root.data.mapping_val;
idcu_YamlValue* name_val = idcu_yaml_mapping_get(map, "name");

const char* name;
idcu_yaml_get_string(name_val, &name);
printf("Name: %s\n", name);

idcu_yaml_free(&root);
```

### YAML 语法

本库支持简化的 YAML 语法：

```yaml
# 键值对
key: value

# 列表
items:
  - item1
  - item2

# 嵌套结构
parent:
  child: value
```

## API 文档

详见 [include/idcu/yaml/yaml.h](include/idcu/yaml/yaml.h)
```

## 验证检查清单

- [ ] YAML 头文件已创建
- [ ] YAML 实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以解析简单的 YAML 字符串
- [ ] 可以访问 YAML mapping 的键值对
- [ ] 可以解析 YAML 列表

## Git 提交

```bash
git add libs/idcu-yaml/
git commit -m "feat: add idcu-yaml library (simplified)

- Add YAML parser from string/file
- Add YAML serializer to string/file
- Add support for scalar, sequence, mapping
- Add type-safe accessor functions
- Add JSON conversion support
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 解析失败 | YAML 语法复杂 | 使用简化的 YAML 语法 |
| 缩进错误 | 缩进不一致 | 确保使用统一的缩进（2空格或4空格） |
| 类型转换错误 | 值类型不匹配 | 确保使用正确的 get_* 函数 |
