# 任务 3.20: idcu-sandbox - 沙箱安全库

## 目标

创建沙箱安全库，支持：
- 资源限制（CPU、内存、磁盘）
- 系统调用过滤
- 文件系统访问控制
- 网络访问控制
- 进程隔离
- 安全审计

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-sandbox/include/idcu/sandbox
mkdir -p libs/idcu-sandbox/src/idcu/sandbox
mkdir -p libs/idcu-sandbox/tests
mkdir -p libs/idcu-sandbox/examples
```

### 2. 创建沙箱头文件 (sandbox.h)

创建 `libs/idcu-sandbox/include/idcu/sandbox/sandbox.h`：

```c
#ifndef IDCU_SANDBOX_SANDBOX_H
#define IDCU_SANDBOX_SANDBOX_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_SandboxId;

typedef enum
{
    IDCU_SANDBOX_PERM_ALLOW = 0,
    IDCU_SANDBOX_PERM_DENY,
    IDCU_SANDBOX_PERM_AUDIT
} idcu_SandboxPermission;

typedef enum
{
    IDCU_SANDBOX_RULE_PATH = 0,
    IDCU_SANDBOX_RULE_SYSCALL,
    IDCU_SANDBOX_RULE_NETWORK,
    IDCU_SANDBOX_RULE_RESOURCE
} idcu_SandboxRuleType;

typedef struct
{
    idcu_SandboxRuleType type;
    idcu_SandboxPermission permission;
    char pattern[512];
    char description[256];
} idcu_SandboxRule;

typedef struct
{
    uint64_t max_cpu_time_ms;
    uint64_t max_memory_bytes;
    uint64_t max_disk_bytes;
    uint64_t max_open_files;
    uint64_t max_processes;
    uint64_t max_network_connections;
} idcu_SandboxResourceLimits;

typedef struct
{
    idcu_Vector allowed_paths;
    idcu_Vector denied_paths;
    idcu_Vector read_only_paths;
} idcu_SandboxFileSystemConfig;

typedef struct
{
    int allow_loopback;
    int allow_private_network;
    int allow_public_network;
    idcu_Vector allowed_addresses;
    idcu_Vector denied_addresses;
    idcu_Vector allowed_ports;
    idcu_Vector denied_ports;
} idcu_SandboxNetworkConfig;

typedef struct
{
    idcu_Vector allowed_syscalls;
    idcu_Vector denied_syscalls;
} idcu_SandboxSyscallConfig;

typedef struct
{
    idcu_SandboxId id;
    char name[128];
    char description[512];
    
    idcu_SandboxResourceLimits resource_limits;
    idcu_SandboxFileSystemConfig fs_config;
    idcu_SandboxNetworkConfig network_config;
    idcu_SandboxSyscallConfig syscall_config;
    
    idcu_Vector rules;
    
    int enabled;
    int audit_enabled;
} idcu_SandboxConfig;

typedef struct
{
    idcu_SandboxId id;
    idcu_SandboxConfig config;
    
    int active;
    uint64_t created_at;
    uint64_t last_violation_at;
    uint64_t violation_count;
    
    idcu_Mutex lock;
} idcu_Sandbox;

typedef struct
{
    idcu_Vector sandboxes;
    idcu_HashMap sandboxes_by_id;
    idcu_Mutex lock;
    
    idcu_Vector audit_log;
    int audit_enabled;
    
    int initialized;
} idcu_SandboxManager;

typedef struct
{
    uint64_t id;
    idcu_SandboxId sandbox_id;
    uint64_t timestamp;
    idcu_SandboxRuleType rule_type;
    char violation[512];
    char details[1024];
    char process_name[256];
} idcu_SandboxViolation;

int  idcu_sandbox_manager_init(idcu_SandboxManager* manager);
void idcu_sandbox_manager_destroy(idcu_SandboxManager* manager);

int  idcu_sandbox_config_init(idcu_SandboxConfig* config, const char* name);
void idcu_sandbox_config_destroy(idcu_SandboxConfig* config);

int  idcu_sandbox_config_set_resource_limits(idcu_SandboxConfig* config, const idcu_SandboxResourceLimits* limits);
int  idcu_sandbox_config_add_allowed_path(idcu_SandboxConfig* config, const char* path);
int  idcu_sandbox_config_add_denied_path(idcu_SandboxConfig* config, const char* path);
int  idcu_sandbox_config_add_read_only_path(idcu_SandboxConfig* config, const char* path);
int  idcu_sandbox_config_add_allowed_address(idcu_SandboxConfig* config, const char* address);
int  idcu_sandbox_config_add_denied_address(idcu_SandboxConfig* config, const char* address);
int  idcu_sandbox_config_add_allowed_port(idcu_SandboxConfig* config, uint16_t port);
int  idcu_sandbox_config_add_denied_port(idcu_SandboxConfig* config, uint16_t port);
int  idcu_sandbox_config_add_allowed_syscall(idcu_SandboxConfig* config, const char* syscall);
int  idcu_sandbox_config_add_denied_syscall(idcu_SandboxConfig* config, const char* syscall);
int  idcu_sandbox_config_add_rule(idcu_SandboxConfig* config, const idcu_SandboxRule* rule);

idcu_SandboxId idcu_sandbox_manager_create(idcu_SandboxManager* manager, const idcu_SandboxConfig* config);
int  idcu_sandbox_manager_destroy(idcu_SandboxManager* manager, idcu_SandboxId sandbox_id);
int  idcu_sandbox_manager_enable(idcu_SandboxManager* manager, idcu_SandboxId sandbox_id);
int  idcu_sandbox_manager_disable(idcu_SandboxManager* manager, idcu_SandboxId sandbox_id);

idcu_Sandbox* idcu_sandbox_manager_get(idcu_SandboxManager* manager, idcu_SandboxId sandbox_id);
size_t idcu_sandbox_manager_count(idcu_SandboxManager* manager);
int  idcu_sandbox_manager_get_all(idcu_SandboxManager* manager, idcu_Vector* sandboxes);

int  idcu_sandbox_check_path_access(idcu_Sandbox* sandbox, const char* path, int mode);
int  idcu_sandbox_check_network_access(idcu_Sandbox* sandbox, const char* address, uint16_t port);
int  idcu_sandbox_check_syscall(idcu_Sandbox* sandbox, const char* syscall);
int  idcu_sandbox_check_resource_usage(idcu_Sandbox* sandbox);

int  idcu_sandbox_log_violation(idcu_Sandbox* sandbox, idcu_SandboxRuleType type, 
                                const char* violation, const char* details, const char* process_name);

int  idcu_sandbox_manager_get_violations(idcu_SandboxManager* manager, idcu_SandboxId sandbox_id, idcu_Vector* violations);
int  idcu_sandbox_manager_get_all_violations(idcu_SandboxManager* manager, idcu_Vector* violations);
int  idcu_sandbox_manager_clear_violations(idcu_SandboxManager* manager, idcu_SandboxId sandbox_id);
int  idcu_sandbox_manager_clear_all_violations(idcu_SandboxManager* manager);

int  idcu_sandbox_resource_limits_init(idcu_SandboxResourceLimits* limits);
int  idcu_sandbox_file_system_config_init(idcu_SandboxFileSystemConfig* config);
void idcu_sandbox_file_system_config_destroy(idcu_SandboxFileSystemConfig* config);
int  idcu_sandbox_network_config_init(idcu_SandboxNetworkConfig* config);
void idcu_sandbox_network_config_destroy(idcu_SandboxNetworkConfig* config);
int  idcu_sandbox_syscall_config_init(idcu_SandboxSyscallConfig* config);
void idcu_sandbox_syscall_config_destroy(idcu_SandboxSyscallConfig* config);

int  idcu_sandbox_rule_init(idcu_SandboxRule* rule, idcu_SandboxRuleType type, 
                             idcu_SandboxPermission permission, const char* pattern);
void idcu_sandbox_rule_destroy(idcu_SandboxRule* rule);

int  idcu_sandbox_violation_init(idcu_SandboxViolation* violation);
void idcu_sandbox_violation_destroy(idcu_SandboxViolation* violation);
int  idcu_sandbox_violation_to_json(const idcu_SandboxViolation* violation, char* buffer, size_t buffer_size);

int  idcu_sandbox_enable_audit(idcu_SandboxManager* manager);
int  idcu_sandbox_disable_audit(idcu_SandboxManager* manager);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `libs/idcu-sandbox/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-sandbox VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-sandbox STATIC
    src/idcu/sandbox/sandbox.c
)

target_include_directories(idcu-sandbox PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-sandbox PRIVATE
    idcu::common
    idcu::json
    idcu::log
)

add_library(idcu::sandbox ALIAS idcu-sandbox)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-sandbox/module.yaml`：

```yaml
name: idcu-sandbox
version: 1.0.0
description: Sandbox security library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-json
  - idcu-log

build:
  type: cmake
  targets:
    - idcu-sandbox

headers:
  - idcu/sandbox/sandbox.h

features:
  - resource_limits: Resource limits (CPU, memory, disk)
  - syscall_filter: System call filtering
  - fs_access: File system access control
  - network_access: Network access control
  - process_isolation: Process isolation
  - audit: Security audit logging
  - rules: Custom security rules

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `libs/idcu-sandbox/README.md`：

```markdown
# idcu-sandbox

IDCU Agent 的沙箱安全库。

## 功能特性

- **资源限制**: 资源限制（CPU、内存、磁盘）
- **系统调用过滤**: 系统调用过滤
- **文件系统访问**: 文件系统访问控制
- **网络访问**: 网络访问控制
- **进程隔离**: 进程隔离
- **安全审计**: 安全审计
- **自定义规则**: 自定义安全规则

## 快速开始

### 初始化沙箱管理器

```c
#include "idcu/sandbox/sandbox.h"

idcu_SandboxManager manager;
idcu_sandbox_manager_init(&manager);
idcu_sandbox_enable_audit(&manager);
```

### 创建沙箱配置

```c
idcu_SandboxConfig config;
idcu_sandbox_config_init(&config, "my-sandbox");
```

### 设置资源限制

```c
idcu_SandboxResourceLimits limits;
idcu_sandbox_resource_limits_init(&limits);

limits.max_cpu_time_ms = 60000;
limits.max_memory_bytes = 100 * 1024 * 1024;
limits.max_disk_bytes = 1024 * 1024 * 1024;
limits.max_open_files = 100;
limits.max_processes = 10;
limits.max_network_connections = 50;

idcu_sandbox_config_set_resource_limits(&config, &limits);
```

### 配置文件系统访问

```c
idcu_sandbox_config_add_allowed_path(&config, "/tmp/app/");
idcu_sandbox_config_add_read_only_path(&config, "/etc/");
idcu_sandbox_config_add_denied_path(&config, "/etc/passwd");
idcu_sandbox_config_add_denied_path(&config, "/root/");
```

### 配置网络访问

```c
config.network_config.allow_loopback = 1;
config.network_config.allow_private_network = 1;
config.network_config.allow_public_network = 0;

idcu_sandbox_config_add_allowed_address(&config, "192.168.1.0/24");
idcu_sandbox_config_add_denied_address(&config, "10.0.0.0/8");
idcu_sandbox_config_add_allowed_port(&config, 80);
idcu_sandbox_config_add_allowed_port(&config, 443);
idcu_sandbox_config_add_denied_port(&config, 22);
```

### 配置系统调用

```c
idcu_sandbox_config_add_allowed_syscall(&config, "read");
idcu_sandbox_config_add_allowed_syscall(&config, "write");
idcu_sandbox_config_add_allowed_syscall(&config, "open");
idcu_sandbox_config_add_denied_syscall(&config, "mount");
idcu_sandbox_config_add_denied_syscall(&config, "reboot");
```

### 创建沙箱

```c
idcu_SandboxId sandbox_id = idcu_sandbox_manager_create(&manager, &config);
idcu_sandbox_manager_enable(&manager, sandbox_id);
```

### 检查访问权限

```c
idcu_Sandbox* sandbox = idcu_sandbox_manager_get(&manager, sandbox_id);

if (idcu_sandbox_check_path_access(sandbox, "/tmp/app/data.txt", 0) == IDCU_ERR_OK) {
    printf("Path access allowed\n");
}

if (idcu_sandbox_check_network_access(sandbox, "192.168.1.100", 80) == IDCU_ERR_OK) {
    printf("Network access allowed\n");
}
```

### 获取违规记录

```c
idcu_Vector violations;
idcu_vector_init(&violations, sizeof(idcu_SandboxViolation));

idcu_sandbox_manager_get_violations(&manager, sandbox_id, &violations);

for (size_t i = 0; i < violations.count; i++) {
    idcu_SandboxViolation* v = (idcu_SandboxViolation*)idcu_vector_get(&violations, i);
    printf("Violation: %s\n", v->violation);
}

idcu_vector_destroy(&violations);
```

### 销毁沙箱

```c
idcu_sandbox_manager_disable(&manager, sandbox_id);
idcu_sandbox_manager_destroy(&manager, sandbox_id);
```

### 销毁沙箱管理器

```c
idcu_sandbox_manager_destroy(&manager);
```

## 权限类型

| 权限 | 说明 |
|-----|------|
| ALLOW | 允许 |
| DENY | 拒绝 |
| AUDIT | 审计 |

## 规则类型

| 类型 | 说明 |
|-----|------|
| PATH | 路径规则 |
| SYSCALL | 系统调用规则 |
| NETWORK | 网络规则 |
| RESOURCE | 资源规则 |

## API 文档

详见 [include/idcu/sandbox/sandbox.h](include/idcu/sandbox/sandbox.h)
```

## 验证检查清单

- [ ] 沙箱头文件已创建
- [ ] 沙箱实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 沙箱可以正常创建和启用
- [ ] 访问控制正常工作
- [ ] 违规记录正常工作

## Git 提交

```bash
git add libs/idcu-sandbox/
git commit -m "feat: add idcu-sandbox library

- Add resource limits (CPU, memory, disk)
- Add system call filtering
- Add file system access control
- Add network access control
- Add process isolation
- Add security audit logging
- Add custom security rules
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 合法访问被阻止 | 规则过于严格 | 调整规则配置 |
| 违规未记录 | 审计未启用 | 启用审计功能 |
| 系统调用过滤不工作 | 平台不支持 | 检查平台兼容性 |
