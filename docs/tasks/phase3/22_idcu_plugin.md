# 任务 3.22: idcu-plugin - 插件系统库

## 目标

创建插件系统库，支持：
- 动态加载插件
- 插件生命周期管理
- 插件依赖管理
- 插件通信
- 插件沙箱
- 插件配置
- 插件版本管理

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-plugin/include/idcu/plugin
mkdir -p libs/idcu-plugin/src/idcu/plugin
mkdir -p libs/idcu-plugin/tests
mkdir -p libs/idcu-plugin/examples
```

### 2. 创建插件系统头文件 (plugin.h)

创建 `libs/idcu-plugin/include/idcu/plugin/plugin.h`：

```c
#ifndef IDCU_PLUGIN_PLUGIN_H
#define IDCU_PLUGIN_PLUGIN_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include "idcu/yaml/yaml.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t idcu_PluginVersion;
typedef uint64_t idcu_PluginId;

#define IDCU_PLUGIN_MAKE_VERSION(major, minor, patch) \
    (((major) << 16) | ((minor) << 8) | (patch))
#define IDCU_PLUGIN_VERSION_MAJOR(version) (((version) >> 16) & 0xFF)
#define IDCU_PLUGIN_VERSION_MINOR(version) (((version) >> 8) & 0xFF)
#define IDCU_PLUGIN_VERSION_PATCH(version) ((version) & 0xFF)

typedef enum
{
    IDCU_PLUGIN_STATE_UNLOADED = 0,
    IDCU_PLUGIN_STATE_LOADED,
    IDCU_PLUGIN_STATE_INITIALIZED,
    IDCU_PLUGIN_STATE_RUNNING,
    IDCU_PLUGIN_STATE_PAUSED,
    IDCU_PLUGIN_STATE_ERROR
} idcu_PluginState;

typedef int (*idcu_PluginInitFunc)(void* plugin_context, void* user_data);
typedef int (*idcu_PluginStartFunc)(void* plugin_context, void* user_data);
typedef int (*idcu_PluginStopFunc)(void* plugin_context, void* user_data);
typedef int (*idcu_PluginDestroyFunc)(void* plugin_context, void* user_data);
typedef int (*idcu_PluginPauseFunc)(void* plugin_context, void* user_data);
typedef int (*idcu_PluginResumeFunc)(void* plugin_context, void* user_data);

typedef struct
{
    char name[128];
    char version[64];
    idcu_PluginVersion version_num;
    char author[128];
    char description[512];
    char license[128];
    char dependencies[1024];
} idcu_PluginInfo;

typedef struct
{
    idcu_PluginInfo info;
    char path[1024];
    idcu_PluginId id;
    idcu_PluginState state;
    void* handle;
    void* context;
    idcu_PluginInitFunc init_func;
    idcu_PluginStartFunc start_func;
    idcu_PluginStopFunc stop_func;
    idcu_PluginDestroyFunc destroy_func;
    idcu_PluginPauseFunc pause_func;
    idcu_PluginResumeFunc resume_func;
    void* user_data;
    idcu_YamlValue* config;
    uint64_t loaded_at;
    uint64_t started_at;
    int enabled;
} idcu_Plugin;

typedef struct
{
    idcu_Vector plugins;
    idcu_HashMap plugins_by_id;
    idcu_HashMap plugins_by_name;
    idcu_Mutex lock;
    char plugin_dir[1024];
    void* host_context;
    int initialized;
} idcu_PluginManager;

typedef struct
{
    char plugin_dir[1024];
} idcu_PluginManagerConfig;

int  idcu_plugin_manager_config_init(idcu_PluginManagerConfig* config);

int  idcu_plugin_manager_init(idcu_PluginManager* manager, const idcu_PluginManagerConfig* config);
void idcu_plugin_manager_destroy(idcu_PluginManager* manager);
int  idcu_plugin_manager_set_plugin_dir(idcu_PluginManager* manager, const char* dir);
int  idcu_plugin_manager_set_host_context(idcu_PluginManager* manager, void* context);

int  idcu_plugin_manager_load(idcu_PluginManager* manager, const char* path, idcu_PluginId* out_id);
int  idcu_plugin_manager_load_by_name(idcu_PluginManager* manager, const char* name, idcu_PluginId* out_id);
int  idcu_plugin_manager_unload(idcu_PluginManager* manager, idcu_PluginId id);
int  idcu_plugin_manager_unload_all(idcu_PluginManager* manager);

int  idcu_plugin_manager_init_plugin(idcu_PluginManager* manager, idcu_PluginId id);
int  idcu_plugin_manager_start_plugin(idcu_PluginManager* manager, idcu_PluginId id);
int  idcu_plugin_manager_stop_plugin(idcu_PluginManager* manager, idcu_PluginId id);
int  idcu_plugin_manager_pause_plugin(idcu_PluginManager* manager, idcu_PluginId id);
int  idcu_plugin_manager_resume_plugin(idcu_PluginManager* manager, idcu_PluginId id);

int  idcu_plugin_manager_init_all(idcu_PluginManager* manager);
int  idcu_plugin_manager_start_all(idcu_PluginManager* manager);
int  idcu_plugin_manager_stop_all(idcu_PluginManager* manager);
int  idcu_plugin_manager_pause_all(idcu_PluginManager* manager);
int  idcu_plugin_manager_resume_all(idcu_PluginManager* manager);

int  idcu_plugin_manager_enable(idcu_PluginManager* manager, idcu_PluginId id);
int  idcu_plugin_manager_disable(idcu_PluginManager* manager, idcu_PluginId id);

idcu_Plugin* idcu_plugin_manager_get(idcu_PluginManager* manager, idcu_PluginId id);
idcu_Plugin* idcu_plugin_manager_get_by_name(idcu_PluginManager* manager, const char* name);
idcu_PluginState idcu_plugin_manager_get_state(idcu_PluginManager* manager, idcu_PluginId id);
size_t idcu_plugin_manager_get_count(idcu_PluginManager* manager);
int  idcu_plugin_manager_get_all(idcu_PluginManager* manager, idcu_Vector* plugins);

int  idcu_plugin_manager_load_config(idcu_PluginManager* manager, idcu_PluginId id, const char* config_path);
int  idcu_plugin_manager_reload_config(idcu_PluginManager* manager, idcu_PluginId id);

int  idcu_plugin_info_init(idcu_PluginInfo* info);
void idcu_plugin_info_destroy(idcu_PluginInfo* info);
int  idcu_plugin_info_from_yaml(idcu_PluginInfo* info, const idcu_YamlValue* yaml);
int  idcu_plugin_info_to_yaml(const idcu_PluginInfo* info, idcu_YamlValue* yaml);

int  idcu_plugin_init(idcu_Plugin* plugin);
void idcu_plugin_destroy(idcu_Plugin* plugin);
int  idcu_plugin_get_info(const idcu_Plugin* plugin, idcu_PluginInfo* info);
void* idcu_plugin_get_context(const idcu_Plugin* plugin);
void* idcu_plugin_get_symbol(const idcu_Plugin* plugin, const char* name);

typedef struct
{
    char name[128];
    char version[64];
    idcu_PluginVersion required_version;
} idcu_PluginDependency;

int  idcu_plugin_dependency_init(idcu_PluginDependency* dep);
void idcu_plugin_dependency_destroy(idcu_PluginDependency* dep);
int  idcu_plugin_check_dependency(idcu_PluginManager* manager, const idcu_PluginDependency* dep);
int  idcu_plugin_resolve_dependencies(idcu_PluginManager* manager, idcu_PluginId id);

typedef struct idcu_PluginMessage
{
    idcu_PluginId sender_id;
    idcu_PluginId target_id;
    char type[128];
    void* data;
    size_t data_size;
    uint64_t timestamp;
} idcu_PluginMessage;

typedef void (*idcu_PluginMessageHandler)(const idcu_PluginMessage* message, void* user_data);

int  idcu_plugin_send_message(idcu_PluginManager* manager, const idcu_PluginMessage* message);
int  idcu_plugin_broadcast_message(idcu_PluginManager* manager, const idcu_PluginMessage* message);
int  idcu_plugin_register_message_handler(idcu_PluginManager* manager, const char* type, 
                                          idcu_PluginMessageHandler handler, void* user_data);
int  idcu_plugin_unregister_message_handler(idcu_PluginManager* manager, const char* type);

typedef struct
{
    idcu_PluginId id;
    char name[128];
    char version[64];
    idcu_PluginState state;
    int enabled;
    uint64_t uptime_ms;
} idcu_PluginStats;

int  idcu_plugin_get_stats(idcu_PluginManager* manager, idcu_PluginId id, idcu_PluginStats* stats);
int  idcu_plugin_get_all_stats(idcu_PluginManager* manager, idcu_Vector* stats);

const char* idcu_plugin_state_to_string(idcu_PluginState state);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `libs/idcu-plugin/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-plugin VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-plugin STATIC
    src/idcu/plugin/plugin.c
)

target_include_directories(idcu-plugin PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-plugin PRIVATE
    idcu::common
    idcu::yaml
    idcu::log
)

if(WIN32)
    target_link_libraries(idcu-plugin PRIVATE)
else()
    target_link_libraries(idcu-plugin PRIVATE dl)
endif()

add_library(idcu::plugin ALIAS idcu-plugin)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-plugin/module.yaml`：

```yaml
name: idcu-plugin
version: 1.0.0
description: Plugin system library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-yaml
  - idcu-log

build:
  type: cmake
  targets:
    - idcu-plugin

headers:
  - idcu/plugin/plugin.h

features:
  - dynamic_load: Dynamic plugin loading
  - lifecycle: Plugin lifecycle management
  - dependencies: Plugin dependency management
  - communication: Plugin communication
  - sandbox: Plugin sandbox
  - config: Plugin configuration
  - version: Plugin version management
  - message: Plugin message passing

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `libs/idcu-plugin/README.md`：

```markdown
# idcu-plugin

IDCU Agent 的插件系统库。

## 功能特性

- **动态加载**: 动态插件加载
- **生命周期管理**: 插件生命周期管理
- **依赖管理**: 插件依赖管理
- **插件通信**: 插件通信
- **插件沙箱**: 插件沙箱
- **插件配置**: 插件配置
- **版本管理**: 插件版本管理
- **消息传递**: 插件消息传递

## 快速开始

### 初始化插件管理器

```c
#include "idcu/plugin/plugin.h"

idcu_PluginManagerConfig config;
idcu_plugin_manager_config_init(&config);

strncpy(config.plugin_dir, "./plugins", sizeof(config.plugin_dir));

idcu_PluginManager manager;
idcu_plugin_manager_init(&manager, &config);
```

### 加载插件

```c
idcu_PluginId plugin_id;
int ret = idcu_plugin_manager_load(&manager, "./plugins/libmy_plugin.so", &plugin_id);

if (ret == IDCU_ERR_OK) {
    printf("Plugin loaded with ID: %" PRIu64 "\n", plugin_id);
}
```

### 初始化和启动插件

```c
idcu_plugin_manager_init_plugin(&manager, plugin_id);
idcu_plugin_manager_start_plugin(&manager, plugin_id);
```

### 停止和卸载插件

```c
idcu_plugin_manager_stop_plugin(&manager, plugin_id);
idcu_plugin_manager_unload(&manager, plugin_id);
```

### 获取插件信息

```c
idcu_Plugin* plugin = idcu_plugin_manager_get(&manager, plugin_id);
if (plugin) {
    printf("Plugin: %s, Version: %s\n", plugin->info.name, plugin->info.version);
    printf("State: %s\n", idcu_plugin_state_to_string(plugin->state));
}
```

### 插件间通信

```c
idcu_PluginMessage msg;
msg.sender_id = my_plugin_id;
msg.target_id = target_plugin_id;
strncpy(msg.type, "custom_message", sizeof(msg.type));
msg.data = data_ptr;
msg.data_size = data_size;

idcu_plugin_send_message(&manager, &msg);
```

### 广播消息

```c
idcu_PluginMessage msg;
msg.sender_id = my_plugin_id;
msg.target_id = 0;
strncpy(msg.type, "broadcast_message", sizeof(msg.type));

idcu_plugin_broadcast_message(&manager, &msg);
```

### 消息处理器

```c
void message_handler(const idcu_PluginMessage* message, void* user_data)
{
    printf("Received message from %" PRIu64 ": %s\n", 
           message->sender_id, message->type);
}

idcu_plugin_register_message_handler(&manager, "custom_message", message_handler, NULL);
```

### 批量操作

```c
idcu_plugin_manager_load_all(&manager);
idcu_plugin_manager_init_all(&manager);
idcu_plugin_manager_start_all(&manager);

idcu_plugin_manager_stop_all(&manager);
idcu_plugin_manager_unload_all(&manager);
```

### 获取插件统计

```c
idcu_PluginStats stats;
idcu_plugin_get_stats(&manager, plugin_id, &stats);

printf("Plugin: %s\n", stats.name);
printf("Version: %s\n", stats.version);
printf("State: %s\n", idcu_plugin_state_to_string(stats.state));
printf("Uptime: %" PRIu64 " ms\n", stats.uptime_ms);
```

### 插件配置

```c
idcu_plugin_manager_load_config(&manager, plugin_id, "./plugins/my_plugin_config.yaml");
idcu_plugin_manager_reload_config(&manager, plugin_id);
```

### 销毁插件管理器

```c
idcu_plugin_manager_destroy(&manager);
```

## 插件开发

### 插件结构

```c
#include <idcu/plugin/plugin.h>

static void* plugin_context = NULL;

int plugin_init(void* context, void* user_data)
{
    plugin_context = context;
    printf("Plugin initialized\n");
    return IDCU_ERR_OK;
}

int plugin_start(void* context, void* user_data)
{
    printf("Plugin started\n");
    return IDCU_ERR_OK;
}

int plugin_stop(void* context, void* user_data)
{
    printf("Plugin stopped\n");
    return IDCU_ERR_OK;
}

int plugin_destroy(void* context, void* user_data)
{
    printf("Plugin destroyed\n");
    return IDCU_ERR_OK;
}

idcu_PluginInfo plugin_get_info(void)
{
    idcu_PluginInfo info;
    idcu_plugin_info_init(&info);
    
    strncpy(info.name, "my_plugin", sizeof(info.name));
    strncpy(info.version, "1.0.0", sizeof(info.version));
    info.version_num = IDCU_PLUGIN_MAKE_VERSION(1, 0, 0);
    strncpy(info.author, "Your Name", sizeof(info.author));
    strncpy(info.description, "My awesome plugin", sizeof(info.description));
    
    return info;
}
```

### plugin.yaml

```yaml
name: my_plugin
version: 1.0.0
author: Your Name
description: My awesome plugin
license: MIT

dependencies:
  - other_plugin >= 2.0.0

config:
  enabled: true
  log_level: info
```

## 插件状态

| 状态 | 说明 |
|-----|------|
| UNLOADED | 未加载 |
| LOADED | 已加载 |
| INITIALIZED | 已初始化 |
| RUNNING | 运行中 |
| PAUSED | 已暂停 |
| ERROR | 错误 |

## API 文档

详见 [include/idcu/plugin/plugin.h](include/idcu/plugin/plugin.h)
```

## 验证检查清单

- [ ] 插件系统头文件已创建
- [ ] 插件系统实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以加载和卸载插件
- [ ] 插件生命周期正常工作
- [ ] 插件间通信正常工作

## Git 提交

```bash
git add libs/idcu-plugin/
git commit -m "feat: add idcu-plugin library

- Add dynamic plugin loading
- Add plugin lifecycle management
- Add plugin dependency management
- Add plugin communication
- Add plugin sandbox
- Add plugin configuration
- Add plugin version management
- Add plugin message passing
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 插件加载失败 | 依赖缺失 | 检查插件依赖 |
| 符号未找到 | 插件导出不正确 | 检查插件导出符号 |
| 崩溃 | 插件有错误 | 检查插件代码和日志 |
