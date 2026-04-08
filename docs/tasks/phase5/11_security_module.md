# 任务 5.11: security-module - 安全业务模块

## 目标

创建安全业务模块，支持：
- 身份认证
- 权限管理
- 数据加密
- 安全审计
- 访问控制
- 安全策略
- 威胁检测

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/business/security-module/include/idcu/security_module
mkdir -p modules/business/security-module/src/idcu/security_module
mkdir -p modules/business/security-module/tests
mkdir -p modules/business/security-module/examples
```

### 2. 创建安全模块头文件 (security_module.h)

创建 `modules/business/security-module/include/idcu/security_module/security_module.h`：

```c
#ifndef IDCU_SECURITY_MODULE_SECURITY_MODULE_H
#define IDCU_SECURITY_MODULE_SECURITY_MODULE_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include "idcu/sdk/sdk.h"
#include "idcu/sandbox/sandbox.h"
#include "idcu/permission/permission.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_UserId;
typedef uint64_t idcu_RoleId;
typedef uint64_t idcu_PolicyId;
typedef uint64_t idcu_AuditId;

typedef enum
{
    IDCU_AUTH_METHOD_PASSWORD = 0,
    IDCU_AUTH_METHOD_TOKEN,
    IDCU_AUTH_METHOD_CERTIFICATE,
    IDCU_AUTH_METHOD_OAUTH,
    IDCU_AUTH_METHOD_API_KEY
} idcu_AuthMethod;

typedef enum
{
    IDCU_PERMISSION_READ = 1 << 0,
    IDCU_PERMISSION_WRITE = 1 << 1,
    IDCU_PERMISSION_EXECUTE = 1 << 2,
    IDCU_PERMISSION_DELETE = 1 << 3,
    IDCU_PERMISSION_ADMIN = 1 << 4,
    IDCU_PERMISSION_ALL = 0x1F
} idcu_PermissionFlag;

typedef enum
{
    IDCU_AUDIT_EVENT_LOGIN = 0,
    IDCU_AUDIT_EVENT_LOGOUT,
    IDCU_AUDIT_EVENT_ACCESS,
    IDCU_AUDIT_EVENT_MODIFY,
    IDCU_AUDIT_EVENT_DELETE,
    IDCU_AUDIT_EVENT_DENIED,
    IDCU_AUDIT_EVENT_ALERT
} idcu_AuditEventType;

typedef struct
{
    idcu_UserId user_id;
    char username[128];
    char email[256];
    char password_hash[256];
    char salt[128];
    idcu_AuthMethod auth_method;
    idcu_RoleId role_id;
    uint64_t created_time;
    uint64_t last_login;
    int enabled;
    int locked;
} idcu_User;

typedef struct
{
    idcu_RoleId role_id;
    char role_name[128];
    char description[512];
    uint32_t permissions;
    idcu_Vector policies;
} idcu_Role;

typedef struct
{
    idcu_PolicyId policy_id;
    char policy_name[128];
    char description[512];
    char resource[256];
    char action[64];
    char condition[1024];
    int enabled;
    uint64_t created_time;
} idcu_SecurityPolicy;

typedef struct
{
    idcu_AuditId audit_id;
    idcu_AuditEventType event_type;
    idcu_UserId user_id;
    char username[128];
    char resource[256];
    char action[64];
    char details[1024];
    char ip_address[64];
    char user_agent[512];
    uint64_t timestamp;
    int success;
} idcu_AuditRecord;

typedef struct
{
    char config_path[1024];
    int enable_authentication;
    int enable_authorization;
    int enable_encryption;
    int enable_audit;
    int enable_threat_detection;
    char encryption_key[256];
    uint64_t session_timeout_ms;
    uint64_t password_min_length;
    int password_require_complexity;
    int enable_password_expiry;
    uint64_t password_expiry_days;
    char audit_log_path[1024];
} idcu_SecurityModuleConfig;

typedef struct
{
    idcu_SdkContext* sdk;
    idcu_SecurityModuleConfig config;
    
    idcu_HashMap users;
    idcu_HashMap roles;
    idcu_HashMap policies;
    idcu_Mutex security_lock;
    
    idcu_Vector audit_records;
    idcu_Mutex audit_lock;
    
    idcu_Sandbox* sandbox;
    idcu_PermissionManager* perm_manager;
    
    int initialized;
} idcu_SecurityModule;

int  idcu_security_module_config_init(idcu_SecurityModuleConfig* config);

int  idcu_security_module_init(idcu_SecurityModule* module, idcu_SdkContext* sdk,
                                 const idcu_SecurityModuleConfig* config);
void idcu_security_module_destroy(idcu_SecurityModule* module);

int  idcu_security_module_start(idcu_SecurityModule* module);
int  idcu_security_module_stop(idcu_SecurityModule* module);

int  idcu_security_module_create_user(idcu_SecurityModule* module, const idcu_User* user,
                                        idcu_UserId* out_user_id);
int  idcu_security_module_update_user(idcu_SecurityModule* module, idcu_UserId user_id,
                                        const idcu_User* user);
int  idcu_security_module_delete_user(idcu_SecurityModule* module, idcu_UserId user_id);
idcu_User* idcu_security_module_get_user(idcu_SecurityModule* module, idcu_UserId user_id);
idcu_User* idcu_security_module_get_user_by_name(idcu_SecurityModule* module, 
                                                     const char* username);

int  idcu_security_module_create_role(idcu_SecurityModule* module, const idcu_Role* role,
                                        idcu_RoleId* out_role_id);
int  idcu_security_module_update_role(idcu_SecurityModule* module, idcu_RoleId role_id,
                                        const idcu_Role* role);
int  idcu_security_module_delete_role(idcu_SecurityModule* module, idcu_RoleId role_id);
idcu_Role* idcu_security_module_get_role(idcu_SecurityModule* module, idcu_RoleId role_id);

int  idcu_security_module_authenticate(idcu_SecurityModule* module, const char* username,
                                          const char* credential, idcu_UserId* out_user_id);
int  idcu_security_module_logout(idcu_SecurityModule* module, idcu_UserId user_id);

int  idcu_security_module_check_permission(idcu_SecurityModule* module, idcu_UserId user_id,
                                             const char* resource, uint32_t permission);
int  idcu_security_module_grant_permission(idcu_SecurityModule* module, idcu_UserId user_id,
                                             const char* resource, uint32_t permission);
int  idcu_security_module_revoke_permission(idcu_SecurityModule* module, idcu_UserId user_id,
                                              const char* resource);

int  idcu_security_module_create_policy(idcu_SecurityModule* module, 
                                          const idcu_SecurityPolicy* policy,
                                          idcu_PolicyId* out_policy_id);
int  idcu_security_module_delete_policy(idcu_SecurityModule* module, idcu_PolicyId policy_id);
int  idcu_security_module_enable_policy(idcu_SecurityModule* module, idcu_PolicyId policy_id);
int  idcu_security_module_disable_policy(idcu_SecurityModule* module, idcu_PolicyId policy_id);

int  idcu_security_module_encrypt(idcu_SecurityModule* module, const void* plaintext,
                                    size_t plaintext_size, void* ciphertext, 
                                    size_t* ciphertext_size);
int  idcu_security_module_decrypt(idcu_SecurityModule* module, const void* ciphertext,
                                    size_t ciphertext_size, void* plaintext,
                                    size_t* plaintext_size);

int  idcu_security_module_log_audit(idcu_SecurityModule* module, 
                                       const idcu_AuditRecord* record);
int  idcu_security_module_get_audit_records(idcu_SecurityModule* module,
                                              uint64_t start_time, uint64_t end_time,
                                              idcu_Vector* records);
int  idcu_security_module_export_audit(idcu_SecurityModule* module, const char* path);

int  idcu_security_module_hash_password(idcu_SecurityModule* module, const char* password,
                                           char* hash, size_t hash_size,
                                           char* salt, size_t salt_size);
int  idcu_security_module_verify_password(idcu_SecurityModule* module, const char* password,
                                             const char* hash, const char* salt);

int  idcu_security_module_lock_user(idcu_SecurityModule* module, idcu_UserId user_id);
int  idcu_security_module_unlock_user(idcu_SecurityModule* module, idcu_UserId user_id);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建模块配置文件 (module.yaml)

创建 `modules/business/security-module/module.yaml`：

```yaml
name: security-module
version: 1.0.0
description: Security business module for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-sdk
  - idcu-sandbox
  - idcu-permission
  - idcu-log

build:
  type: cmake
  targets:
    - security-module

headers:
  - idcu/security_module/security_module.h

features:
  - auth: User authentication
  - authorization: Permission management
  - encryption: Data encryption
  - audit: Security audit
  - access_control: Access control
  - policies: Security policies
  - threat_detection: Threat detection

testing:
  enabled: true
  framework: internal
```

### 4. 创建 README.md

创建 `modules/business/security-module/README.md`：

```markdown
# security-module

IDCU Agent 的安全业务模块。

## 功能特性

- **身份认证**: 用户身份认证
- **权限管理**: 权限管理
- **数据加密**: 数据加密
- **安全审计**: 安全审计
- **访问控制**: 访问控制
- **安全策略**: 安全策略
- **威胁检测**: 威胁检测

## 快速开始

### 初始化安全模块

```c
#include "idcu/security_module/security_module.h"

idcu_SecurityModuleConfig config;
idcu_security_module_config_init(&config);

config.enable_authentication = 1;
config.enable_authorization = 1;
config.enable_encryption = 1;
config.enable_audit = 1;
config.session_timeout_ms = 3600000;
config.password_min_length = 8;
config.password_require_complexity = 1;

idcu_SecurityModule module;
idcu_security_module_init(&module, sdk_context, &config);
```

### 启动安全模块

```c
idcu_security_module_start(&module);
```

### 创建用户

```c
idcu_User user = {
    .username = "admin",
    .email = "admin@example.com",
    .auth_method = IDCU_AUTH_METHOD_PASSWORD,
    .enabled = 1,
    .locked = 0
};

idcu_UserId user_id;
idcu_security_module_create_user(&module, &user, &user_id);
```

### 创建角色

```c
idcu_Role role = {
    .role_name = "administrator",
    .description = "System administrator",
    .permissions = IDCU_PERMISSION_ALL
};

idcu_RoleId role_id;
idcu_security_module_create_role(&module, &role, &role_id);
```

### 用户认证

```c
idcu_UserId auth_user_id;
if (idcu_security_module_authenticate(&module, "admin", "password123", 
                                        &auth_user_id) == IDCU_ERR_OK) {
    printf("Authentication successful\n");
}
```

### 检查权限

```c
if (idcu_security_module_check_permission(&module, auth_user_id, 
                                            "/api/data", IDCU_PERMISSION_READ) == IDCU_ERR_OK) {
    printf("Access granted\n");
}
```

### 数据加密

```c
const char* plaintext = "sensitive data";
unsigned char ciphertext[1024];
size_t ciphertext_size = sizeof(ciphertext);

idcu_security_module_encrypt(&module, plaintext, strlen(plaintext),
                               ciphertext, &ciphertext_size);
```

### 数据解密

```c
char decrypted[1024];
size_t decrypted_size = sizeof(decrypted);

idcu_security_module_decrypt(&module, ciphertext, ciphertext_size,
                               decrypted, &decrypted_size);
```

### 记录审计日志

```c
idcu_AuditRecord record = {
    .event_type = IDCU_AUDIT_EVENT_LOGIN,
    .user_id = auth_user_id,
    .resource = "/api/login",
    .action = "login",
    .timestamp = time(NULL),
    .success = 1
};

idcu_security_module_log_audit(&module, &record);
```

### 用户登出

```c
idcu_security_module_logout(&module, auth_user_id);
```

### 停止安全模块

```c
idcu_security_module_stop(&module);
idcu_security_module_destroy(&module);
```

## 认证方式

| 方式 | 说明 |
|-----|------|
| PASSWORD | 密码认证 |
| TOKEN | Token认证 |
| CERTIFICATE | 证书认证 |
| OAUTH | OAuth认证 |
| API_KEY | API Key认证 |

## 权限标志

| 标志 | 说明 |
|-----|------|
| READ | 读取 |
| WRITE | 写入 |
| EXECUTE | 执行 |
| DELETE | 删除 |
| ADMIN | 管理 |
| ALL | 所有权限 |

## 审计事件类型

| 类型 | 说明 |
|-----|------|
| LOGIN | 登录 |
| LOGOUT | 登出 |
| ACCESS | 访问 |
| MODIFY | 修改 |
| DELETE | 删除 |
| DENIED | 拒绝 |
| ALERT | 告警 |

## API 文档

详见 [include/idcu/security_module/security_module.h](include/idcu/security_module/security_module.h)
```

## 验证检查清单

- [ ] 安全模块头文件已创建
- [ ] 安全模块实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 用户认证功能正常工作
- [ ] 权限管理功能正常工作
- [ ] 审计日志功能正常

## Git 提交

```bash
git add modules/business/security-module/
git commit -m "feat: add security-module business module

- Add user authentication
- Add permission management
- Add data encryption
- Add security audit
- Add access control
- Add security policies
- Add threat detection
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 认证失败 | 密码错误或用户被锁定 | 检查密码和用户状态 |
| 权限拒绝 | 权限配置错误 | 检查用户角色和权限 |
| 加密失败 | 加密密钥配置错误 | 检查加密密钥配置 |
