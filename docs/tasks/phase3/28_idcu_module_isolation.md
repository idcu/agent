# 任务 3.28: idcu-module-isolation - 模块隔离

## 目标

创建模块隔离库，支持：
- 模块内存隔离
- 模块权限控制
- 模块资源限制
- 模块故障隔离
- 模块间通信安全
- 模块沙箱执行

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-module-isolation/include/idcu/module_isolation
mkdir -p libs/idcu-module-isolation/src/idcu/module_isolation
mkdir -p libs/idcu-module-isolation/tests
mkdir -p libs/idcu-module-isolation/examples
```

### 2. 创建模块隔离头文件 (module_isolation.h)

创建 `libs/idcu-module-isolation/include/idcu/module_isolation/module_isolation.h`：

```c
#ifndef IDCU_MODULE_ISOLATION_MODULE_ISOLATION_H
#define IDCU_MODULE_ISOLATION_MODULE_ISOLATION_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include "idcu/sandbox/sandbox.h"
#include "idcu/permission/permission.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_IsolatedModuleId;

typedef enum
{
    IDCU_ISOLATION_LEVEL_NONE = 0,
    IDCU_ISOLATION_LEVEL_LIGHT,
    IDCU_ISOLATION_LEVEL_MEDIUM,
    IDCU_ISOLATION_LEVEL_STRICT,
    IDCU_ISOLATION_LEVEL_FULL
} idcu_IsolationLevel;

typedef enum
{
    IDCU_RESOURCE_CPU = 0,
    IDCU_RESOURCE_MEMORY,
    IDCU_RESOURCE_DISK,
    IDCU_RESOURCE_NETWORK,
    IDCU_RESOURCE_FILE_DESCRIPTORS
} idcu_ResourceType;

typedef struct
{
    idcu_ResourceType type;
    uint64_t limit;
    uint64_t current_usage;
    int enabled;
} idcu_ResourceLimit;

typedef struct
{
    uint64_t max_memory_bytes;
    uint64_t max_cpu_percent;
    uint64_t max_disk_bytes;
    uint64_t max_network_bps;
    uint32_t max_fd_count;
    uint64_t max_execution_time_ms;
} idcu_ResourceConstraints;

typedef enum
{
    IDCU_PERM_READ_FILE = 0,
    IDCU_PERM_WRITE_FILE,
    IDCU_PERM_EXECUTE,
    IDCU_PERM_NETWORK,
    IDCU_PERM_PROCESS,
    IDCU_PERM_MEMORY,
    IDCU_PERM_SIGNAL,
    IDCU_PERM_MODULE_CALL,
    IDCU_PERM_MESSAGE_SEND,
    IDCU_PERM_MESSAGE_RECEIVE
} idcu_ModulePermission;

typedef struct
{
    char path[1024];
    int read_allowed;
    int write_allowed;
    int execute_allowed;
} idcu_FileAccessRule;

typedef struct
{
    char allowed_ips[1024];
    uint16_t min_port;
    uint16_t max_port;
    int outbound_allowed;
    int inbound_allowed;
} idcu_NetworkAccessRule;

typedef struct
{
    idcu_IsolatedModuleId module_id;
    char module_name[128];
    idcu_IsolationLevel isolation_level;
    
    idcu_ResourceConstraints resource_limits;
    idcu_Vector file_rules;
    idcu_NetworkAccessRule network_rule;
    
    uint32_t allowed_permissions;
    
    idcu_Sandbox* sandbox;
    idcu_PermissionManager* perm_manager;
    
    int running;
    int crashed;
    uint64_t start_time;
    uint64_t last_heartbeat;
    
    void* module_handle;
    char error_message[512];
} idcu_IsolatedModule;

typedef int (*idcu_ModuleInitFunc)(void* context);
typedef int (*idcu_ModuleRunFunc)(void* context);
typedef int (*idcu_ModuleStopFunc)(void* context);
typedef void (*idcu_ModuleCrashCallback)(idcu_IsolatedModule* module, const char* reason, void* user_data);

typedef struct
{
    idcu_IsolationLevel default_isolation_level;
    idcu_ResourceConstraints default_resources;
    uint64_t watchdog_timeout_ms;
    int enable_crash_recovery;
    int max_restart_count;
    uint64_t restart_cooldown_ms;
} idcu_ModuleIsolationConfig;

typedef struct
{
    idcu_ModuleIsolationConfig config;
    idcu_Vector modules;
    idcu_HashMap module_map;
    idcu_Mutex lock;
    
    idcu_ModuleCrashCallback crash_callback;
    void* crash_user_data;
    
    int initialized;
    int running;
} idcu_ModuleIsolator;

int  idcu_module_isolation_config_init(idcu_ModuleIsolationConfig* config);

int  idcu_module_isolator_init(idcu_ModuleIsolator* isolator, const idcu_ModuleIsolationConfig* config);
void idcu_module_isolator_destroy(idcu_ModuleIsolator* isolator);

int  idcu_module_isolator_start(idcu_ModuleIsolator* isolator);
int  idcu_module_isolator_stop(idcu_ModuleIsolator* isolator);

int  idcu_isolated_module_init(idcu_IsolatedModule* module, const char* name, 
                                idcu_IsolationLevel level);
void idcu_isolated_module_destroy(idcu_IsolatedModule* module);

int  idcu_module_isolator_load_module(idcu_ModuleIsolator* isolator, 
                                        const char* path,
                                        const idcu_IsolatedModule* template,
                                        idcu_IsolatedModuleId* out_id);
int  idcu_module_isolator_unload_module(idcu_ModuleIsolator* isolator, idcu_IsolatedModuleId module_id);

idcu_IsolatedModule* idcu_module_isolator_get_module(idcu_ModuleIsolator* isolator, 
                                                       idcu_IsolatedModuleId module_id);
idcu_IsolatedModule* idcu_module_isolator_get_module_by_name(idcu_ModuleIsolator* isolator, 
                                                                const char* name);

size_t idcu_module_isolator_get_module_count(idcu_ModuleIsolator* isolator);
int  idcu_module_isolator_get_all_modules(idcu_ModuleIsolator* isolator, idcu_Vector* modules);

int  idcu_module_isolator_start_module(idcu_ModuleIsolator* isolator, idcu_IsolatedModuleId module_id);
int  idcu_module_isolator_stop_module(idcu_ModuleIsolator* isolator, idcu_IsolatedModuleId module_id);
int  idcu_module_isolator_restart_module(idcu_ModuleIsolator* isolator, idcu_IsolatedModuleId module_id);

int  idcu_module_set_resource_limits(idcu_IsolatedModule* module, 
                                      const idcu_ResourceConstraints* limits);
int  idcu_module_add_file_rule(idcu_IsolatedModule* module, const idcu_FileAccessRule* rule);
int  idcu_module_set_network_rule(idcu_IsolatedModule* module, 
                                   const idcu_NetworkAccessRule* rule);

int  idcu_module_grant_permission(idcu_IsolatedModule* module, idcu_ModulePermission perm);
int  idcu_module_revoke_permission(idcu_IsolatedModule* module, idcu_ModulePermission perm);
int  idcu_module_has_permission(idcu_IsolatedModule* module, idcu_ModulePermission perm);

int  idcu_module_check_file_access(idcu_IsolatedModule* module, const char* path, int write);
int  idcu_module_check_network_access(idcu_IsolatedModule* module, const char* host, uint16_t port);

int  idcu_module_get_resource_usage(idcu_IsolatedModule* module, idcu_ResourceConstraints* usage);

int  idcu_module_isolator_set_crash_callback(idcu_ModuleIsolator* isolator,
                                              idcu_ModuleCrashCallback callback,
                                              void* user_data);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-module-isolation/module.yaml`：

```yaml
name: idcu-module-isolation
version: 1.0.0
description: Module isolation library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-sandbox
  - idcu-permission
  - idcu-log

build:
  type: cmake
  targets:
    - idcu-module-isolation

headers:
  - idcu/module_isolation/module_isolation.h

features:
  - memory_isolation: Module memory isolation
  - permission_control: Module permission control
  - resource_limits: Module resource limits
  - fault_isolation: Module fault isolation
  - communication_security: Inter-module communication security
  - sandbox: Module sandbox execution

testing:
  enabled: true
  framework: internal
```

### 4. 创建 README.md

创建 `libs/idcu-module-isolation/README.md`：

```markdown
# idcu-module-isolation

IDCU Agent 的模块隔离库。

## 功能特性

- **内存隔离**: 模块内存隔离
- **权限控制**: 模块权限控制
- **资源限制**: 模块资源限制
- **故障隔离**: 模块故障隔离
- **通信安全**: 模块间通信安全
- **沙箱执行**: 模块沙箱执行

## 快速开始

### 初始化隔离器

```c
#include "idcu/module_isolation/module_isolation.h"

idcu_ModuleIsolationConfig config;
idcu_module_isolation_config_init(&config);

config.default_isolation_level = IDCU_ISOLATION_LEVEL_MEDIUM;
config.default_resources.max_memory_bytes = 256 * 1024 * 1024;
config.default_resources.max_cpu_percent = 50;
config.watchdog_timeout_ms = 30000;
config.enable_crash_recovery = 1;
config.max_restart_count = 3;

idcu_ModuleIsolator isolator;
idcu_module_isolator_init(&isolator, &config);
```

### 配置模块模板

```c
idcu_IsolatedModule module_template;
idcu_isolated_module_init(&module_template, "my-module", IDCU_ISOLATION_LEVEL_MEDIUM);

module_template.resource_limits.max_memory_bytes = 128 * 1024 * 1024;
module_template.resource_limits.max_fd_count = 1024;

idcu_FileAccessRule file_rule = {
    .path = "/tmp",
    .read_allowed = 1,
    .write_allowed = 1,
    .execute_allowed = 0
};
idcu_module_add_file_rule(&module_template, &file_rule);

idcu_module_grant_permission(&module_template, IDCU_PERM_READ_FILE);
idcu_module_grant_permission(&module_template, IDCU_PERM_WRITE_FILE);
idcu_module_grant_permission(&module_template, IDCU_PERM_MESSAGE_SEND);
idcu_module_grant_permission(&module_template, IDCU_PERM_MESSAGE_RECEIVE);
```

### 加载模块

```c
idcu_IsolatedModuleId module_id;
idcu_module_isolator_load_module(&isolator, "./modules/my_module.so", 
                                 &module_template, &module_id);
```

### 启动模块

```c
idcu_module_isolator_start_module(&isolator, module_id);
```

### 检查资源使用

```c
idcu_ResourceConstraints usage;
idcu_module_get_resource_usage(idcu_module_isolator_get_module(&isolator, module_id), &usage);

printf("Memory used: %" PRIu64 " bytes\n", usage.max_memory_bytes);
```

### 设置崩溃回调

```c
void on_module_crash(idcu_IsolatedModule* module, const char* reason, void* user_data)
{
    printf("Module %s crashed: %s\n", module->module_name, reason);
}

idcu_module_isolator_set_crash_callback(&isolator, on_module_crash, NULL);
```

### 停止模块

```c
idcu_module_isolator_stop_module(&isolator, module_id);
idcu_module_isolator_unload_module(&isolator, module_id);
```

### 停止隔离器

```c
idcu_module_isolator_stop(&isolator);
idcu_module_isolator_destroy(&isolator);
```

## 隔离级别

| 级别 | 说明 |
|-----|------|
| NONE | 无隔离 |
| LIGHT | 轻量级隔离 |
| MEDIUM | 中等隔离 |
| STRICT | 严格隔离 |
| FULL | 完全隔离 |

## 资源类型

| 类型 | 说明 |
|-----|------|
| CPU | CPU使用率 |
| MEMORY | 内存使用 |
| DISK | 磁盘使用 |
| NETWORK | 网络带宽 |
| FILE_DESCRIPTORS | 文件描述符 |

## 模块权限

| 权限 | 说明 |
|-----|------|
| READ_FILE | 读取文件 |
| WRITE_FILE | 写入文件 |
| EXECUTE | 执行 |
| NETWORK | 网络访问 |
| PROCESS | 进程管理 |
| MEMORY | 内存管理 |
| SIGNAL | 信号处理 |
| MODULE_CALL | 模块调用 |
| MESSAGE_SEND | 发送消息 |
| MESSAGE_RECEIVE | 接收消息 |

## API 文档

详见 [include/idcu/module_isolation/module_isolation.h](include/idcu/module_isolation/module_isolation.h)
```

## 验证检查清单

- [ ] 模块隔离头文件已创建
- [ ] 模块隔离实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以加载和隔离模块
- [ ] 资源限制正常工作
- [ ] 权限控制正常工作

## Git 提交

```bash
git add libs/idcu-module-isolation/
git commit -m "feat: add idcu-module-isolation library

- Add module memory isolation
- Add module permission control
- Add module resource limits
- Add module fault isolation
- Add inter-module communication security
- Add module sandbox execution
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 模块无法加载 | 权限或路径问题 | 检查文件权限和路径 |
| 模块崩溃 | 隔离级别太严格 | 适当降低隔离级别 |
| 资源超限 | 限制设置过低 | 调整资源限制配置 |
