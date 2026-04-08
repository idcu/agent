# 任务 3.6: idcu-config - 配置管理库

## 目标

创建完整的配置管理库，支持：
- YAML 和 JSON 格式配置文件
- 配置热重载
- 多环境配置支持
- 类型安全的配置访问
- 配置变更通知
- 文件监控和自动重载

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-config/include/idcu/config
mkdir -p libs/idcu-config/src/idcu/config
mkdir -p libs/idcu-config/tests
mkdir -p libs/idcu-config/examples
```

### 2. 创建配置头文件 (config.h)

创建 `libs/idcu-config/include/idcu/config/config.h`：

```c
#ifndef IDCU_CONFIG_CONFIG_H
#define IDCU_CONFIG_CONFIG_H

#include "idcu/common/error_code.h"
#include "idcu/common/lock.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_CONFIG_MAX_SECTIONS         64
#define IDCU_CONFIG_MAX_KEYS_PER_SECTION 128
#define IDCU_CONFIG_KEY_MAX              128
#define IDCU_CONFIG_VALUE_MAX            512
#define IDCU_CONFIG_SECTION_MAX          128
#define IDCU_CONFIG_MAX_LIST_ITEMS       32
#define IDCU_CONFIG_LIST_ITEM_MAX        128
#define IDCU_CONFIG_PATH_MAX             1024

typedef struct
{
    char key[IDCU_CONFIG_KEY_MAX];
    char value[IDCU_CONFIG_VALUE_MAX];
} idcu_ConfigEntry;

typedef struct
{
    char             name[IDCU_CONFIG_SECTION_MAX];
    idcu_ConfigEntry entries[IDCU_CONFIG_MAX_KEYS_PER_SECTION];
    uint32_t         entry_count;
} idcu_ConfigSection;

typedef struct
{
    idcu_ConfigSection sections[IDCU_CONFIG_MAX_SECTIONS];
    uint32_t           section_count;
    idcu_Mutex         lock;
    int                loaded;
    int                env_var_enabled;
    int                validation_enabled;
} idcu_ConfigManager;

typedef struct
{
    char items[IDCU_CONFIG_MAX_LIST_ITEMS][IDCU_CONFIG_LIST_ITEM_MAX];
    int  count;
} idcu_ConfigList;

int  idcu_config_init(const char* file_path);
void idcu_config_shutdown(void);
int  idcu_config_is_loaded(void);
int  idcu_config_reload(void);
int  idcu_config_save(const char* file_path);

const char* idcu_config_get_string(const char* section, const char* key,
                                   const char* default_value);
int         idcu_config_get_int(const char* section, const char* key, int default_value);
int64_t     idcu_config_get_int64(const char* section, const char* key, int64_t default_value);
double      idcu_config_get_double(const char* section, const char* key, double default_value);
int         idcu_config_get_bool(const char* section, const char* key, int default_value);

int idcu_config_set_string(const char* section, const char* key, const char* value);
int idcu_config_set_int(const char* section, const char* key, int value);
int idcu_config_set_int64(const char* section, const char* key, int64_t value);
int idcu_config_set_double(const char* section, const char* key, double value);
int idcu_config_set_bool(const char* section, const char* key, int value);

int idcu_config_has_section(const char* section);
int idcu_config_has_key(const char* section, const char* key);
int idcu_config_remove_key(const char* section, const char* key);
int idcu_config_remove_section(const char* section);

int idcu_config_get_list(const char* section, const char* key, const char* delimiter,
                         idcu_ConfigList* out_list);
int idcu_config_list_contains(const char* section, const char* key, const char* delimiter,
                              const char* value);

int idcu_config_get_nested_bool(const char* section, const char* prefix, const char* subkey,
                                int default_value);
int idcu_config_get_nested_int(const char* section, const char* prefix, const char* subkey,
                               int default_value);
const char* idcu_config_get_nested_string(const char* section, const char* prefix,
                                          const char* subkey, const char* default_value);

void idcu_config_enable_env_var(int enable);
void idcu_config_enable_validation(int enable);
int  idcu_config_validate(void);

int idcu_config_load_profile(const char* profile_name);

typedef void (*idcu_ConfigChangeCallback)(const char* section, const char* key,
                                          const char* old_value, const char* new_value,
                                          void* user_data);

int  idcu_config_register_change_callback(idcu_ConfigChangeCallback callback, void* user_data);
int  idcu_config_unregister_change_callback(idcu_ConfigChangeCallback callback);
void idcu_config_notify_changes(void);

int idcu_config_get_file_path(char* buffer, size_t buffer_size);
int idcu_config_get_last_modified_time(uint64_t* timestamp);

int  idcu_config_watch_start(uint32_t interval_ms);
void idcu_config_watch_stop(void);
int  idcu_config_watch_is_running(void);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `libs/idcu-config/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-config VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-config STATIC
    src/idcu/config/config.c
)

target_include_directories(idcu-config PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-config PRIVATE
    idcu::common
    idcu::json
    idcu::yaml
)

add_library(idcu::config ALIAS idcu-config)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-config/module.yaml`：

```yaml
name: idcu-config
version: 1.0.0
description: Configuration management library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-json
  - idcu-yaml

build:
  type: cmake
  targets:
    - idcu-config

headers:
  - idcu/config/config.h

features:
  - yaml_support: YAML format configuration
  - json_support: JSON format configuration
  - hot_reload: Hot configuration reloading
  - multi_env: Multi-environment configuration support
  - type_safe: Type-safe configuration access
  - change_notify: Configuration change notifications
  - file_watch: File monitoring and auto-reload

testing:
  enabled: true
  framework: internal
```

### 5. 创建示例配置文件 (agent.yaml)

创建 `config/default/agent.yaml`（示例）：

```yaml
agent:
  name: "idcu-agent"
  version: "1.0.0"
  log_level: "info"

log:
  level: "info"
  output: ["console", "file"]
  file: "logs/agent.log"
  max_size: 104857600
  max_backups: 10

modules:
  enabled:
    - core
    - log
    - metrics

metrics:
  enabled: true
  interval: 60

network:
  listen_port: 8080
  max_connections: 100
```

### 6. 创建 README.md

创建 `libs/idcu-config/README.md`：

```markdown
# idcu-config

IDCU Agent 的配置管理库。

## 功能特性

- **多格式支持**: YAML 和 JSON 格式
- **热重载**: 支持配置热重载
- **多环境**: 支持多环境配置
- **类型安全**: 类型安全的配置访问
- **变更通知**: 配置变更回调通知
- **文件监控**: 自动监控配置文件变更

## 快速开始

### 基本使用

```c
#include "idcu/config/config.h"

int ret = idcu_config_init("config/agent.yaml");
if (ret != IDCU_ERR_OK) {
    printf("Config load failed: %s\n", idcu_err_to_str(ret));
    return -1;
}

const char* name = idcu_config_get_string("agent", "name", "default");
int port = idcu_config_get_int("network", "listen_port", 8080);

printf("Agent name: %s\n", name);
printf("Listen port: %d\n", port);

idcu_config_shutdown();
```

### 配置热重载

```c
idcu_config_register_change_callback(my_config_change_handler, user_data);
idcu_config_watch_start(5000);

idcu_config_reload();
```

## 配置文件格式

### YAML 格式

```yaml
section:
  key: value
  number: 42
  boolean: true
```

## API 文档

详见 [include/idcu/config/config.h](include/idcu/config/config.h)
```

## 验证检查清单

- [ ] 配置头文件已创建
- [ ] 配置实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以加载 YAML 配置文件
- [ ] 可以读取各种类型的配置值
- [ ] 配置热重载功能正常

## Git 提交

```bash
git add libs/idcu-config/
git commit -m "feat: add idcu-config library

- Add YAML and JSON config support
- Add hot configuration reload
- Add multi-environment support
- Add type-safe config access
- Add change notifications
- Add file monitoring
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 配置加载失败 | 文件格式错误或路径不对 | 检查配置文件格式和路径 |
| 热重载不生效 | 未正确启动监控 | 确保调用了 idcu_config_watch_start |
| 类型转换错误 | 配置值类型不匹配 | 确保使用正确的 get_* 函数 |
