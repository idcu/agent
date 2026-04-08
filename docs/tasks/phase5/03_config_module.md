# 任务 5.3: config-module - 配置业务模块

## 目标

创建配置业务模块，支持：
- 配置加载和管理
- 配置热重载
- 多环境配置
- 配置验证
- 配置默认值
- 配置备份
- 配置回滚
- 配置历史
- 配置变更通知
- 配置加密

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/config-module/include/idcu/config_module
mkdir -p modules/config-module/src/idcu/config_module
mkdir -p modules/config-module/tests
mkdir -p modules/config-module/examples
```

### 2. 创建配置业务模块头文件 (config_module.h)

创建 `modules/config-module/include/idcu/config_module/config_module.h`：

```c
#ifndef IDCU_CONFIG_MODULE_CONFIG_MODULE_H
#define IDCU_CONFIG_MODULE_CONFIG_MODULE_H

#include "idcu/common/error_code.h"
#include "idcu/config/config.h"
#include "idcu/yaml/yaml.h"
#include "idcu/json/json.h"
#include "idcu/msgbus/msg_bus.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_ConfigModuleId;

typedef enum
{
    IDCU_CONFIG_SOURCE_FILE = 0,
    IDCU_CONFIG_SOURCE_ENV,
    IDCU_CONFIG_SOURCE_ETCD,
    IDCU_CONFIG_SOURCE_CONSUL,
    IDCU_CONFIG_SOURCE_DATABASE
} idcu_ConfigSourceType;

typedef enum
{
    IDCU_CONFIG_CHANGE_TYPE_MODIFIED = 0,
    IDCU_CONFIG_CHANGE_TYPE_ADDED,
    IDCU_CONFIG_CHANGE_TYPE_REMOVED
} idcu_ConfigChangeType;

typedef struct
{
    char key[512];
    char old_value[4096];
    char new_value[4096];
    idcu_ConfigChangeType type;
    uint64_t timestamp;
    char user[128];
    char message[512];
} idcu_ConfigChange;

typedef struct
{
    uint64_t id;
    uint64_t timestamp;
    idcu_Vector changes;
    char message[512];
    char user[128];
    char backup_path[1024];
} idcu_ConfigVersion;

typedef void (*idcu_ConfigChangeCallback)(const idcu_ConfigChange* change, void* user_data);

typedef struct
{
    idcu_ConfigModuleId id;
    char name[128];
    
    idcu_ConfigManager config_manager;
    idcu_Vector versions;
    idcu_Mutex lock;
    
    char config_dir[1024];
    char config_path[1024];
    char backup_dir[1024];
    
    idcu_YamlValue* yaml_config;
    idcu_JsonValue* json_config;
    
    idcu_ConfigSourceType source_type;
    char source_url[1024];
    
    int hot_reload_enabled;
    int auto_backup_enabled;
    int encryption_enabled;
    char encryption_key[64];
    
    idcu_Vector change_callbacks;
    idcu_MsgBus* msg_bus;
    char msgbus_topic[256];
    
    idcu_Thread watch_thread;
    int watch_running;
    
    uint64_t current_version;
    int initialized;
} idcu_ConfigModule;

typedef struct
{
    char config_dir[1024];
    char config_path[1024];
    char backup_dir[1024];
    idcu_ConfigSourceType source_type;
    char source_url[1024];
    int enable_hot_reload;
    int enable_auto_backup;
    int enable_encryption;
    char encryption_key[64];
    uint64_t max_versions;
} idcu_ConfigModuleConfig;

int  idcu_config_module_config_init(idcu_ConfigModuleConfig* config);

int  idcu_config_module_init(idcu_ConfigModule* cm, const idcu_ConfigModuleConfig* config);
void idcu_config_module_destroy(idcu_ConfigModule* cm);
int  idcu_config_module_load(idcu_ConfigModule* cm);
int  idcu_config_module_reload(idcu_ConfigModule* cm);
int  idcu_config_module_save(idcu_ConfigModule* cm);

int  idcu_config_module_get(idcu_ConfigModule* cm, const char* key, char* buffer, size_t buffer_size);
int  idcu_config_module_get_int(idcu_ConfigModule* cm, const char* key, int64_t* value, int64_t default_value);
int  idcu_config_module_get_double(idcu_ConfigModule* cm, const char* key, double* value, double default_value);
int  idcu_config_module_get_bool(idcu_ConfigModule* cm, const char* key, int* value, int default_value);
idcu_ConfigManager* idcu_config_module_get_manager(idcu_ConfigModule* cm);

int  idcu_config_module_set(idcu_ConfigModule* cm, const char* key, const char* value, const char* message);
int  idcu_config_module_set_int(idcu_ConfigModule* cm, const char* key, int64_t value, const char* message);
int  idcu_config_module_set_double(idcu_ConfigModule* cm, const char* key, double value, const char* message);
int  idcu_config_module_set_bool(idcu_ConfigModule* cm, const char* key, int value, const char* message);

int  idcu_config_module_exists(idcu_ConfigModule* cm, const char* key);
int  idcu_config_module_remove(idcu_ConfigModule* cm, const char* key, const char* message);
int  idcu_config_module_clear(idcu_ConfigModule* cm, const char* message);

int  idcu_config_module_enable_hot_reload(idcu_ConfigModule* cm);
int  idcu_config_module_disable_hot_reload(idcu_ConfigModule* cm);

int  idcu_config_module_add_change_callback(idcu_ConfigModule* cm, idcu_ConfigChangeCallback callback, void* user_data);
int  idcu_config_module_remove_change_callback(idcu_ConfigModule* cm, idcu_ConfigChangeCallback callback);
int  idcu_config_module_set_msgbus(idcu_ConfigModule* cm, idcu_MsgBus* msg_bus, const char* topic);

int  idcu_config_module_backup(idcu_ConfigModule* cm, const char* message);
int  idcu_config_module_rollback(idcu_ConfigModule* cm, uint64_t version_id);
int  idcu_config_module_get_version(idcu_ConfigModule* cm, uint64_t version_id, idcu_ConfigVersion** version);
int  idcu_config_module_list_versions(idcu_ConfigModule* cm, idcu_Vector* versions);
int  idcu_config_module_delete_version(idcu_ConfigModule* cm, uint64_t version_id);
int  idcu_config_module_clear_history(idcu_ConfigModule* cm);

int  idcu_config_module_load_env(idcu_ConfigModule* cm, const char* prefix);
int  idcu_config_module_load_env_var(idcu_ConfigModule* cm, const char* env_name, const char* config_key);

int  idcu_config_module_validate(idcu_ConfigModule* cm, const char* schema_path);
int  idcu_config_module_validate_json(idcu_ConfigModule* cm, const char* schema_json);

int  idcu_config_module_load_defaults(idcu_ConfigModule* cm, const char* defaults_path);
int  idcu_config_module_merge(idcu_ConfigModule* cm, const char* override_path);

int  idcu_config_module_enable_encryption(idcu_ConfigModule* cm, const char* key);
int  idcu_config_module_disable_encryption(idcu_ConfigModule* cm);

int  idcu_config_module_export(idcu_ConfigModule* cm, char* buffer, size_t buffer_size, int format);
int  idcu_config_module_export_file(idcu_ConfigModule* cm, const char* path, int format);
int  idcu_config_module_import(idcu_ConfigModule* cm, const char* data, const char* message);
int  idcu_config_module_import_file(idcu_ConfigModule* cm, const char* path, const char* message);

int  idcu_config_module_watch(idcu_ConfigModule* cm);
int  idcu_config_module_unwatch(idcu_ConfigModule* cm);

int  idcu_config_change_init(idcu_ConfigChange* change);
void idcu_config_change_destroy(idcu_ConfigChange* change);

int  idcu_config_version_init(idcu_ConfigVersion* version);
void idcu_config_version_destroy(idcu_ConfigVersion* version);

int  idcu_config_module_get_info(idcu_ConfigModule* cm, char* buffer, size_t buffer_size);
int  idcu_config_module_get_info_json(idcu_ConfigModule* cm, char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `modules/config-module/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(config-module VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(config-module STATIC
    src/idcu/config_module/config_module.c
    src/idcu/config_module/config_version.c
    src/idcu/config_module/config_encryption.c
)

target_include_directories(config-module PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(config-module PRIVATE
    idcu::common
    idcu::config
    idcu::yaml
    idcu::json
    idcu::msgbus
    idcu::utils
    idcu::storage
    idcu::log
)

add_library(idcu::config-module ALIAS config-module)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `modules/config-module/module.yaml`：

```yaml
name: config-module
version: 1.0.0
description: Config business module for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-config
  - idcu-yaml
  - idcu-json
  - idcu-msgbus
  - idcu-utils
  - idcu-storage
  - idcu-log

build:
  type: cmake
  targets:
    - config-module

headers:
  - idcu/config_module/config_module.h

features:
  - load: Config loading and management
  - hot_reload: Config hot reload
  - multi_env: Multi-environment config
  - validation: Config validation
  - defaults: Config defaults
  - backup: Config backup
  - rollback: Config rollback
  - history: Config history
  - notification: Config change notification
  - encryption: Config encryption

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `modules/config-module/README.md`：

```markdown
# config-module

IDCU Agent 的配置业务模块。

## 功能特性

- **加载管理**: 配置加载和管理
- **热重载**: 配置热重载
- **多环境**: 多环境配置
- **验证**: 配置验证
- **默认值**: 配置默认值
- **备份**: 配置备份
- **回滚**: 配置回滚
- **历史**: 配置历史
- **通知**: 配置变更通知
- **加密**: 配置加密

## 快速开始

### 初始化配置模块

```c
#include "idcu/config_module/config_module.h"

idcu_ConfigModuleConfig config;
idcu_config_module_config_init(&config);

strncpy(config.config_dir, "./config", sizeof(config.config_dir));
strncpy(config.config_path, "./config/app.yaml", sizeof(config.config_path));
strncpy(config.backup_dir, "./config/backups", sizeof(config.backup_dir));
config.enable_hot_reload = 1;
config.enable_auto_backup = 1;
config.enable_encryption = 0;
config.max_versions = 100;

idcu_ConfigModule cm;
idcu_config_module_init(&cm, &config);
```

### 加载配置

```c
idcu_config_module_load(&cm);
```

### 读取配置

```c
char value[256];
idcu_config_module_get(&cm, "app.name", value, sizeof(value));
printf("App name: %s\n", value);

int64_t port;
idcu_config_module_get_int(&cm, "app.port", &port, 8080);
printf("Port: %" PRId64 "\n", port);

double timeout;
idcu_config_module_get_double(&cm, "app.timeout", &timeout, 30.0);
printf("Timeout: %f\n", timeout);

int debug;
idcu_config_module_get_bool(&cm, "app.debug", &debug, 0);
printf("Debug: %d\n", debug);
```

### 修改配置

```c
idcu_config_module_set(&cm, "app.version", "1.1.0", "Update version");
idcu_config_module_set_int(&cm, "app.max_connections", 200, "Increase connections");
idcu_config_module_set_double(&cm, "app.timeout", 60.0, "Increase timeout");
idcu_config_module_set_bool(&cm, "app.feature_enabled", 1, "Enable feature");
```

### 检查配置存在

```c
if (idcu_config_module_exists(&cm, "app.name")) {
    printf("Config exists\n");
}
```

### 删除配置

```c
idcu_config_module_remove(&cm, "app.temp", "Remove temp config");
```

### 清空配置

```c
idcu_config_module_clear(&cm, "Reset all config");
```

### 保存配置

```c
idcu_config_module_save(&cm);
```

### 热重载

```c
idcu_config_module_enable_hot_reload(&cm);
idcu_config_module_watch(&cm);
```

### 添加变更回调

```c
void config_changed(const idcu_ConfigChange* change, void* user_data)
{
    printf("Config changed: %s\n", change->key);
    printf("Change type: %d\n", change->type);
    printf("Old value: %s\n", change->old_value);
    printf("New value: %s\n", change->new_value);
}

idcu_config_module_add_change_callback(&cm, config_changed, NULL);
```

### 设置消息总线

```c
idcu_config_module_set_msgbus(&cm, msg_bus, "config_changes");
```

### 备份配置

```c
idcu_config_module_backup(&cm, "Backup before update");
```

### 回滚配置

```c
idcu_config_module_rollback(&cm, 1);
```

### 查看版本历史

```c
idcu_Vector versions;
idcu_vector_init(&versions, sizeof(idcu_ConfigVersion*));

idcu_config_module_list_versions(&cm, &versions);

for (size_t i = 0; i < versions.count; i++) {
    idcu_ConfigVersion** version_ptr = (idcu_ConfigVersion**)idcu_vector_get(&versions, i);
    idcu_ConfigVersion* version = *version_ptr;
    printf("Version %" PRIu64 ": %s\n", version->id, version->message);
}

idcu_vector_destroy(&versions);
```

### 获取特定版本

```c
idcu_ConfigVersion* version;
idcu_config_module_get_version(&cm, 1, &version);
printf("Message: %s\n", version->message);
printf("User: %s\n", version->user);
```

### 删除版本

```c
idcu_config_module_delete_version(&cm, 1);
```

### 清空历史

```c
idcu_config_module_clear_history(&cm);
```

### 加载环境变量

```c
idcu_config_module_load_env(&cm, "APP_");
idcu_config_module_load_env_var(&cm, "DATABASE_URL", "database.url");
```

### 验证配置

```c
idcu_config_module_validate(&cm, "./config/schema.yaml");
```

### 加载默认值

```c
idcu_config_module_load_defaults(&cm, "./config/defaults.yaml");
```

### 合并配置

```c
idcu_config_module_merge(&cm, "./config/override.yaml");
```

### 启用加密

```c
idcu_config_module_enable_encryption(&cm, "my-secret-key-32-bytes-long!");
```

### 禁用加密

```c
idcu_config_module_disable_encryption(&cm);
```

### 导出配置

```c
char buffer[8192];
idcu_config_module_export(&cm, buffer, sizeof(buffer), 0);
printf("%s\n", buffer);

idcu_config_module_export_file(&cm, "./config/export.yaml", 0);
```

### 导入配置

```c
const char* import_data = "app:\n  name: Imported\n";
idcu_config_module_import(&cm, import_data, "Import config");

idcu_config_module_import_file(&cm, "./config/import.yaml", "Import from file");
```

### 停止监视

```c
idcu_config_module_unwatch(&cm);
idcu_config_module_disable_hot_reload(&cm);
```

### 获取信息

```c
char info_buffer[2048];
idcu_config_module_get_info(&cm, info_buffer, sizeof(info_buffer));
printf("%s\n", info_buffer);

char json_buffer[4096];
idcu_config_module_get_info_json(&cm, json_buffer, sizeof(json_buffer));
printf("%s\n", json_buffer);
```

### 重新加载

```c
idcu_config_module_reload(&cm);
```

### 清理

```c
idcu_config_module_destroy(&cm);
```

## 配置源类型

| 类型 | 说明 |
|-----|------|
| FILE | 文件 |
| ENV | 环境变量 |
| ETCD | etcd |
| CONSUL | Consul |
| DATABASE | 数据库 |

## 配置变更类型

| 类型 | 说明 |
|-----|------|
| MODIFIED | 修改 |
| ADDED | 添加 |
| REMOVED | 删除 |

## API 文档

详见 [include/idcu/config_module/config_module.h](include/idcu/config_module/config_module.h)
```

## 验证检查清单

- [ ] 配置业务模块头文件已创建
- [ ] 配置业务模块实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以加载和保存配置
- [ ] 配置备份和回滚正常工作
- [ ] 配置加密正常工作

## Git 提交

```bash
git add modules/config-module/
git commit -m "feat: add config-module module

- Add config loading and management
- Add config hot reload
- Add multi-environment config
- Add config validation
- Add config defaults
- Add config backup
- Add config rollback
- Add config history
- Add config change notification
- Add config encryption
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 配置未加载 | 文件不存在 | 检查配置文件路径 |
| 热重载不工作 | 监视未启用 | 确保启用了文件监视 |
| 回滚失败 | 版本不存在 | 检查版本 ID |
| 加密失败 | 密钥长度不对 | 确保密钥长度正确 |
