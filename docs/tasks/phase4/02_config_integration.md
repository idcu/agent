# 任务 4.2: 配置集成模块

## 目标

创建配置集成模块，支持：
- 统一配置管理
- 多格式配置（YAML/JSON）
- 配置热重载
- 多环境配置
- 配置验证
- 配置默认值
- 配置文件监视

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/config-integration/include/idcu/config_integration
mkdir -p modules/config-integration/src/idcu/config_integration
mkdir -p modules/config-integration/tests
mkdir -p modules/config-integration/examples
```

### 2. 创建配置集成头文件 (config_integration.h)

创建 `modules/config-integration/include/idcu/config_integration/config_integration.h`：

```c
#ifndef IDCU_CONFIG_INTEGRATION_CONFIG_INTEGRATION_H
#define IDCU_CONFIG_INTEGRATION_CONFIG_INTEGRATION_H

#include "idcu/common/error_code.h"
#include "idcu/config/config.h"
#include "idcu/yaml/yaml.h"
#include "idcu/json/json.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_ConfigSectionId;

typedef enum
{
    IDCU_CONFIG_FORMAT_AUTO = 0,
    IDCU_CONFIG_FORMAT_YAML,
    IDCU_CONFIG_FORMAT_JSON
} idcu_ConfigFormat;

typedef enum
{
    IDCU_CONFIG_ENV_DEVELOPMENT = 0,
    IDCU_CONFIG_ENV_TESTING,
    IDCU_CONFIG_ENV_STAGING,
    IDCU_CONFIG_ENV_PRODUCTION
} idcu_ConfigEnvironment;

typedef void (*idcu_ConfigChangeCallback)(const char* key, const char* old_value, const char* new_value, void* user_data);

typedef struct
{
    idcu_ConfigSectionId id;
    char name[128];
    char path[1024];
    idcu_ConfigFormat format;
    idcu_ConfigManager* manager;
    idcu_YamlValue* yaml_config;
    idcu_JsonValue* json_config;
    int hot_reload_enabled;
    int watch_enabled;
    idcu_ConfigChangeCallback change_callback;
    void* callback_user_data;
    uint64_t last_modified;
    int initialized;
} idcu_ConfigSection;

typedef struct
{
    idcu_Vector sections;
    idcu_HashMap sections_by_id;
    idcu_HashMap sections_by_name;
    idcu_Mutex lock;
    
    idcu_ConfigEnvironment environment;
    char config_dir[1024];
    char env_name[64];
    
    idcu_Thread watch_thread;
    int watch_running;
    int initialized;
} idcu_ConfigIntegration;

typedef struct
{
    char config_dir[1024];
    idcu_ConfigEnvironment environment;
    char env_name[64];
    int enable_watch;
} idcu_ConfigIntegrationConfig;

int  idcu_config_integration_config_init(idcu_ConfigIntegrationConfig* config);

int  idcu_config_integration_init(idcu_ConfigIntegration* ci, const idcu_ConfigIntegrationConfig* config);
void idcu_config_integration_destroy(idcu_ConfigIntegration* ci);
int  idcu_config_integration_load(idcu_ConfigIntegration* ci);
int  idcu_config_integration_reload(idcu_ConfigIntegration* ci);

idcu_ConfigSectionId idcu_config_integration_add_section(idcu_ConfigIntegration* ci, const char* name, const char* path);
idcu_ConfigSectionId idcu_config_integration_add_section_with_format(idcu_ConfigIntegration* ci, const char* name, 
                                                                     const char* path, idcu_ConfigFormat format);
int  idcu_config_integration_remove_section(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id);
int  idcu_config_integration_load_section(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id);
int  idcu_config_integration_reload_section(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id);
int  idcu_config_integration_enable_hot_reload(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id);
int  idcu_config_integration_disable_hot_reload(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id);
int  idcu_config_integration_set_change_callback(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id,
                                                   idcu_ConfigChangeCallback callback, void* user_data);

idcu_ConfigSection* idcu_config_integration_get_section(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id);
idcu_ConfigSection* idcu_config_integration_get_section_by_name(idcu_ConfigIntegration* ci, const char* name);
idcu_ConfigManager* idcu_config_integration_get_config_manager(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id);

int  idcu_config_integration_get(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id, const char* key, 
                                  char* buffer, size_t buffer_size);
int  idcu_config_integration_get_int(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id, const char* key, 
                                       int64_t* value, int64_t default_value);
int  idcu_config_integration_get_double(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id, const char* key, 
                                          double* value, double default_value);
int  idcu_config_integration_get_bool(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id, const char* key, 
                                        int* value, int default_value);

int  idcu_config_integration_set(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id, const char* key, const char* value);
int  idcu_config_integration_set_int(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id, const char* key, int64_t value);
int  idcu_config_integration_set_double(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id, const char* key, double value);
int  idcu_config_integration_set_bool(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id, const char* key, int value);

int  idcu_config_integration_exists(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id, const char* key);
int  idcu_config_integration_remove(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id, const char* key);
int  idcu_config_integration_clear(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id);

int  idcu_config_integration_save(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id);
int  idcu_config_integration_save_all(idcu_ConfigIntegration* ci);

int  idcu_config_integration_set_environment(idcu_ConfigIntegration* ci, idcu_ConfigEnvironment env);
int  idcu_config_integration_set_environment_name(idcu_ConfigIntegration* ci, const char* name);
idcu_ConfigEnvironment idcu_config_integration_get_environment(idcu_ConfigIntegration* ci);
const char* idcu_config_integration_get_environment_name(idcu_ConfigIntegration* ci);

int  idcu_config_integration_load_env_file(idcu_ConfigIntegration* ci, const char* path);
int  idcu_config_integration_load_env_var(idcu_ConfigIntegration* ci, const char* name, const char* key);

int  idcu_config_section_init(idcu_ConfigSection* section, const char* name, const char* path);
void idcu_config_section_destroy(idcu_ConfigSection* section);
int  idcu_config_section_load(idcu_ConfigSection* section);
int  idcu_config_section_reload(idcu_ConfigSection* section);
int  idcu_config_section_save(idcu_ConfigSection* section);

int  idcu_config_section_get(const idcu_ConfigSection* section, const char* key, char* buffer, size_t buffer_size);
int  idcu_config_section_get_int(const idcu_ConfigSection* section, const char* key, int64_t* value, int64_t default_value);
int  idcu_config_section_get_double(const idcu_ConfigSection* section, const char* key, double* value, double default_value);
int  idcu_config_section_get_bool(const idcu_ConfigSection* section, const char* key, int* value, int default_value);

int  idcu_config_section_set(idcu_ConfigSection* section, const char* key, const char* value);
int  idcu_config_section_set_int(idcu_ConfigSection* section, const char* key, int64_t value);
int  idcu_config_section_set_double(idcu_ConfigSection* section, const char* key, double value);
int  idcu_config_section_set_bool(idcu_ConfigSection* section, const char* key, int value);

int  idcu_config_section_exists(const idcu_ConfigSection* section, const char* key);
int  idcu_config_section_remove(idcu_ConfigSection* section, const char* key);
int  idcu_config_section_clear(idcu_ConfigSection* section);

int  idcu_config_format_from_extension(const char* path, idcu_ConfigFormat* format);
const char* idcu_config_format_to_extension(idcu_ConfigFormat format);

const char* idcu_config_environment_to_string(idcu_ConfigEnvironment env);
idcu_ConfigEnvironment idcu_config_environment_from_string(const char* str);

int  idcu_config_integration_validate(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id, const char* schema_path);
int  idcu_config_integration_set_defaults(idcu_ConfigIntegration* ci, idcu_ConfigSectionId id, const char* defaults_path);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `modules/config-integration/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(config-integration VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(config-integration STATIC
    src/idcu/config_integration/config_integration.c
)

target_include_directories(config-integration PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(config-integration PRIVATE
    idcu::common
    idcu::config
    idcu::yaml
    idcu::json
    idcu::log
    idcu::utils
)

add_library(idcu::config-integration ALIAS config-integration)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `modules/config-integration/module.yaml`：

```yaml
name: config-integration
version: 1.0.0
description: Config integration module for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-config
  - idcu-yaml
  - idcu-json
  - idcu-log
  - idcu-utils

build:
  type: cmake
  targets:
    - config-integration

headers:
  - idcu/config_integration/config_integration.h

features:
  - unified: Unified config management
  - multi_format: Multi-format config (YAML/JSON)
  - hot_reload: Config hot reload
  - multi_env: Multi-environment config
  - validation: Config validation
  - defaults: Config defaults
  - watch: Config file watching
  - env_vars: Environment variable integration

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `modules/config-integration/README.md`：

```markdown
# config-integration

IDCU Agent 的配置集成模块。

## 功能特性

- **统一管理**: 统一配置管理
- **多格式**: 多格式配置（YAML/JSON）
- **热重载**: 配置热重载
- **多环境**: 多环境配置
- **配置验证**: 配置验证
- **默认值**: 配置默认值
- **文件监视**: 配置文件监视
- **环境变量**: 环境变量集成

## 快速开始

### 初始化配置集成

```c
#include "idcu/config_integration/config_integration.h"

idcu_ConfigIntegrationConfig config;
idcu_config_integration_config_init(&config);

strncpy(config.config_dir, "./config", sizeof(config.config_dir));
config.environment = IDCU_CONFIG_ENV_DEVELOPMENT;
config.enable_watch = 1;

idcu_ConfigIntegration ci;
idcu_config_integration_init(&ci, &config);
```

### 添加配置段

```c
idcu_ConfigSectionId app_id = idcu_config_integration_add_section(&ci, "app", "./config/app.yaml");
idcu_ConfigSectionId db_id = idcu_config_integration_add_section(&ci, "database", "./config/database.json");
```

### 加载配置

```c
idcu_config_integration_load(&ci);
```

### 读取配置

```c
char buffer[256];
idcu_config_integration_get(&ci, app_id, "app.name", buffer, sizeof(buffer));
printf("App name: %s\n", buffer);

int64_t port;
idcu_config_integration_get_int(&ci, app_id, "app.port", &port, 8080);
printf("Port: %" PRId64 "\n", port);

double timeout;
idcu_config_integration_get_double(&ci, app_id, "app.timeout", &timeout, 30.0);
printf("Timeout: %f\n", timeout);

int debug;
idcu_config_integration_get_bool(&ci, app_id, "app.debug", &debug, 0);
printf("Debug: %d\n", debug);
```

### 写入配置

```c
idcu_config_integration_set(&ci, app_id, "app.version", "1.0.0");
idcu_config_integration_set_int(&ci, app_id, "app.max_connections", 100);
idcu_config_integration_set_double(&ci, app_id, "app.threshold", 0.75);
idcu_config_integration_set_bool(&ci, app_id, "app.feature_enabled", 1);
```

### 保存配置

```c
idcu_config_integration_save(&ci, app_id);
idcu_config_integration_save_all(&ci);
```

### 启用热重载

```c
idcu_config_integration_enable_hot_reload(&ci, app_id);
```

### 设置变更回调

```c
void config_changed(const char* key, const char* old_value, const char* new_value, void* user_data)
{
    printf("Config changed: %s: %s -> %s\n", key, old_value, new_value);
}

idcu_config_integration_set_change_callback(&ci, app_id, config_changed, NULL);
```

### 重新加载配置

```c
idcu_config_integration_reload(&ci);
idcu_config_integration_reload_section(&ci, app_id);
```

### 多环境配置

```c
idcu_config_integration_set_environment(&ci, IDCU_CONFIG_ENV_PRODUCTION);
idcu_config_integration_set_environment_name(&ci, "prod");

idcu_ConfigEnvironment env = idcu_config_integration_get_environment(&ci);
const char* env_name = idcu_config_integration_get_environment_name(&ci);

printf("Environment: %s\n", idcu_config_environment_to_string(env));
```

### 环境变量集成

```c
idcu_config_integration_load_env_file(&ci, ".env");
idcu_config_integration_load_env_var(&ci, "DATABASE_URL", "database.url");
```

### 设置默认值

```c
idcu_config_integration_set_defaults(&ci, app_id, "./config/app.defaults.yaml");
```

### 验证配置

```c
idcu_config_integration_validate(&ci, app_id, "./config/app.schema.json");
```

### 获取配置段

```c
idcu_ConfigSection* section = idcu_config_integration_get_section(&ci, app_id);
idcu_ConfigSection* section_by_name = idcu_config_integration_get_section_by_name(&ci, "app");
```

### 直接操作配置段

```c
idcu_config_section_get(section, "key", buffer, sizeof(buffer));
idcu_config_section_get_int(section, "count", &count, 0);
idcu_config_section_set(section, "key", "value");
idcu_config_section_save(section);
```

### 清理

```c
idcu_config_integration_destroy(&ci);
```

## 配置环境

| 环境 | 说明 |
|-----|------|
| DEVELOPMENT | 开发环境 |
| TESTING | 测试环境 |
| STAGING | 预发布环境 |
| PRODUCTION | 生产环境 |

## 配置格式

| 格式 | 说明 |
|-----|------|
| AUTO | 自动检测 |
| YAML | YAML 格式 |
| JSON | JSON 格式 |

## 配置文件结构

### app.yaml

```yaml
app:
  name: MyApp
  version: 1.0.0
  port: 8080
  debug: true
  max_connections: 100
```

### app.defaults.yaml

```yaml
app:
  port: 8080
  debug: false
  max_connections: 50
  timeout: 30.0
```

### .env

```
DATABASE_URL=postgres://user:pass@localhost/db
REDIS_URL=redis://localhost:6379
API_KEY=secret
```

## API 文档

详见 [include/idcu/config_integration/config_integration.h](include/idcu/config_integration/config_integration.h)
```

## 验证检查清单

- [ ] 配置集成头文件已创建
- [ ] 配置集成实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以加载和保存配置
- [ ] 多环境配置正常工作
- [ ] 热重载正常工作

## Git 提交

```bash
git add modules/config-integration/
git commit -m "feat: add config-integration module

- Add unified config management
- Add multi-format config (YAML/JSON)
- Add config hot reload
- Add multi-environment config
- Add config validation
- Add config defaults
- Add config file watching
- Add environment variable integration
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 配置未加载 | 文件不存在 | 检查配置文件路径 |
| 热重载不工作 | 监视未启用 | 确保启用了文件监视 |
| 格式错误 | 配置格式不正确 | 检查 YAML/JSON 语法 |
