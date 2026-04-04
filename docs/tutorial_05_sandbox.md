# 教程五：沙箱安全机制

沙箱是 IDCU Agent 的安全基石，它确保模块在受限环境中运行，防止恶意或错误的模块破坏系统。本教程将深入讲解沙箱的原理和使用方法。

---

## 学习目标

完成本教程后，你将学会：
- 沙箱的安全模型
- 如何配置模块权限
- 如何进行权限检查
- 资源配额管理
- 安全最佳实践

---

## 一、沙箱概述

### 1.1 为什么需要沙箱？

在模块化系统中，沙箱提供了以下安全保障：

- **权限隔离**：每个模块只能访问被允许的资源
- **故障隔离**：一个模块的崩溃不会影响整个系统
- **资源限制**：防止模块消耗过多资源
- **审计追踪**：记录模块的行为便于调试

### 1.2 核心概念

```c
#include "security/sandbox.h"

// 沙箱结构体
typedef struct {
    uint32_t module_id;     // 模块 ID
    uint32_t perm;          // 权限位掩码
    uint32_t quota;         // 资源配额
    idcu_StackContext ctx;  // 上下文
} idcu_Sandbox;
```

---

## 二、权限系统

### 2.1 权限类型

IDCU Agent 定义了以下权限：

```c
#include "security/sandbox.h"

// 权限定义
#define IDCU_PERM_SEND     (1U << 0)  // 发送消息
#define IDCU_PERM_RECV     (1U << 1)  // 接收消息
#define IDCU_PERM_RUN      (1U << 2)  // 运行模块
#define IDCU_PERM_HW       (1U << 3)  // 访问硬件
#define IDCU_PERM_FILE     (1U << 4)  // 文件操作
#define IDCU_PERM_NETWORK  (1U << 5)  // 网络访问
#define IDCU_PERM_ALL      (IDCU_PERM_SEND | IDCU_PERM_RECV | IDCU_PERM_RUN | \
                           IDCU_PERM_HW | IDCU_PERM_FILE | IDCU_PERM_NETWORK)
```

### 2.2 权限组合

使用位运算组合多个权限：

```c
// 赋予消息发送和接收权限
uint32_t msg_perms = IDCU_PERM_SEND | IDCU_PERM_RECV;

// 赋予所有权限
uint32_t all_perms = IDCU_PERM_ALL;

// 基础运行权限
uint32_t basic_perms = IDCU_PERM_RUN;
```

---

## 三、沙箱基本操作

### 3.1 初始化沙箱

```c
#include "security/sandbox.h"
#include "utils/log.h"

void sandbox_init_example(void)
{
    idcu_Sandbox sb;
    uint32_t module_id = 1;
    
    // 初始化沙箱，赋予基础权限
    uint32_t initial_perms = IDCU_PERM_RUN | IDCU_PERM_SEND | IDCU_PERM_RECV;
    int ret = idcu_sandbox_init(&sb, module_id, initial_perms);
    
    if (ret == IDCU_ERR_OK) {
        idcu_log_info("Sandbox initialized for module %u", module_id);
    } else {
        idcu_log_error("Failed to initialize sandbox: %d", ret);
    }
    
    // 使用完后销毁
    idcu_sandbox_destroy(&sb);
}
```

### 3.2 设置和修改权限

```c
void permission_management_example(void)
{
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, IDCU_PERM_RUN);
    
    // 获取当前权限
    uint32_t current_perm = idcu_sandbox_get_perm(&sb);
    idcu_log_info("Current permissions: 0x%x", current_perm);
    
    // 添加文件权限
    idcu_sandbox_add_perm(&sb, IDCU_PERM_FILE);
    idcu_log_info("After add: 0x%x", idcu_sandbox_get_perm(&sb));
    
    // 移除发送权限
    idcu_sandbox_remove_perm(&sb, IDCU_PERM_SEND);
    idcu_log_info("After remove: 0x%x", idcu_sandbox_get_perm(&sb));
    
    // 设置全新的权限
    idcu_sandbox_set_perm(&sb, IDCU_PERM_RUN | IDCU_PERM_RECV);
    idcu_log_info("After set: 0x%x", idcu_sandbox_get_perm(&sb));
    
    idcu_sandbox_destroy(&sb);
}
```

### 3.3 权限检查

```c
void permission_check_example(void)
{
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, IDCU_PERM_RUN | IDCU_PERM_SEND);
    
    // 检查是否有发送权限
    if (idcu_sandbox_perm_check(&sb, IDCU_PERM_SEND) == IDCU_ERR_OK) {
        idcu_log_info("Module has SEND permission");
    } else {
        idcu_log_warn("Module does NOT have SEND permission");
    }
    
    // 检查是否有文件权限
    if (idcu_sandbox_perm_check(&sb, IDCU_PERM_FILE) == IDCU_ERR_OK) {
        idcu_log_info("Module has FILE permission");
    } else {
        idcu_log_warn("Module does NOT have FILE permission");
    }
    
    // 检查多项权限（同时需要）
    uint32_t required = IDCU_PERM_SEND | IDCU_PERM_RECV;
    if (idcu_sandbox_perm_check(&sb, required) == IDCU_ERR_OK) {
        idcu_log_info("Module has both SEND and RECV");
    } else {
        idcu_log_warn("Module is missing some permissions");
    }
    
    idcu_sandbox_destroy(&sb);
}
```

---

## 四、资源配额

### 4.1 设置和获取配额

```c
void quota_management_example(void)
{
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, IDCU_PERM_RUN);
    
    // 设置配额（例如：1MB = 1048576 字节）
    uint32_t quota = 1048576;
    idcu_sandbox_set_quota(&sb, quota);
    
    // 获取当前配额
    uint32_t current_quota = idcu_sandbox_get_quota(&sb);
    idcu_log_info("Current quota: %u bytes", current_quota);
    
    idcu_sandbox_destroy(&sb);
}
```

---

## 五、沙箱注册中心

### 5.1 注册中心初始化

沙箱注册中心用于管理多个模块的沙箱：

```c
#include "security/sandbox.h"

void registry_example(void)
{
    // 初始化注册中心
    int ret = idcu_sandbox_registry_init();
    if (ret != IDCU_ERR_OK) {
        idcu_log_error("Failed to initialize sandbox registry");
        return;
    }
    
    // 创建并添加沙箱
    idcu_Sandbox sb1, sb2;
    idcu_sandbox_init(&sb1, 1, IDCU_PERM_RUN | IDCU_PERM_SEND);
    idcu_sandbox_init(&sb2, 2, IDCU_PERM_ALL);
    
    idcu_sandbox_registry_add(&sb1);
    idcu_sandbox_registry_add(&sb2);
    
    // 通过模块 ID 获取沙箱
    idcu_Sandbox* sb = idcu_sandbox_registry_get(1);
    if (sb) {
        idcu_log_info("Got sandbox for module 1");
    }
    
    // 检查模块权限
    if (idcu_sandbox_registry_check_perm(1, IDCU_PERM_SEND) == IDCU_ERR_OK) {
        idcu_log_info("Module 1 has SEND permission");
    }
    
    // 移除沙箱
    idcu_sandbox_registry_remove(2);
    
    // 销毁注册中心
    idcu_sandbox_registry_destroy();
    
    idcu_sandbox_destroy(&sb1);
    idcu_sandbox_destroy(&sb2);
}
```

---

## 六、完整示例：安全的文件操作模块

让我们创建一个使用沙箱进行权限检查的文件操作模块。

### 6.1 安全文件操作包装器

```c
#include "security/sandbox.h"
#include "utils/log.h"
#include "common/error_code.h"
#include <stdio.h>

// 安全文件打开函数
FILE* safe_fopen(idcu_Sandbox* sb, const char* filename, const char* mode)
{
    // 检查文件权限
    if (idcu_sandbox_perm_check(sb, IDCU_PERM_FILE) != IDCU_ERR_OK) {
        idcu_log_error("[Sandbox] Permission denied: FILE access required");
        return NULL;
    }
    
    idcu_log_info("[Sandbox] Allowing file operation: %s", filename);
    return fopen(filename, mode);
}

// 安全文件关闭函数
void safe_fclose(idcu_Sandbox* sb, FILE* fp)
{
    if (fp) {
        fclose(fp);
        idcu_log_info("[Sandbox] File closed");
    }
}
```

### 6.2 使用沙箱的模块

```c
#include "module/module_def.h"
#include "security/sandbox.h"
#include "utils/log.h"
#include "utils/config_manager.h"

static idcu_Sandbox g_module_sandbox;

static int secure_module_init(void)
{
    idcu_log_info("[secure_module] Initializing...");
    
    // 从配置读取权限
    const char* perm_str = idcu_config_get_string("secure_module", "permissions", "run,send,recv");
    
    // 解析权限字符串
    uint32_t perms = IDCU_PERM_RUN;
    if (strstr(perm_str, "send")) perms |= IDCU_PERM_SEND;
    if (strstr(perm_str, "recv")) perms |= IDCU_PERM_RECV;
    if (strstr(perm_str, "file")) perms |= IDCU_PERM_FILE;
    if (strstr(perm_str, "network")) perms |= IDCU_PERM_NETWORK;
    
    // 初始化沙箱
    int ret = idcu_sandbox_init(&g_module_sandbox, 10, perms);
    if (ret != IDCU_ERR_OK) {
        idcu_log_error("[secure_module] Failed to initialize sandbox");
        return -1;
    }
    
    // 设置配额
    uint32_t quota = idcu_config_get_int("secure_module", "quota", 1048576);
    idcu_sandbox_set_quota(&g_module_sandbox, quota);
    
    idcu_log_info("[secure_module] Initialized with permissions: 0x%x, quota: %u", 
                  perms, quota);
    return 0;
}

static int secure_module_run(void)
{
    // 模块运行逻辑
    // 所有敏感操作都需要经过沙箱权限检查
    
    static int count = 0;
    if (++count % 100 == 0) {
        // 检查权限示例
        if (idcu_sandbox_perm_check(&g_module_sandbox, IDCU_PERM_FILE) == IDCU_ERR_OK) {
            idcu_log_info("[secure_module] Can perform file operations");
        } else {
            idcu_log_debug("[secure_module] No file permission");
        }
    }
    
    return 0;
}

static int secure_module_stop(void)
{
    idcu_log_info("[secure_module] Stopping...");
    idcu_sandbox_destroy(&g_module_sandbox);
    idcu_log_info("[secure_module] Stopped");
    return 0;
}

IDCU_REGISTER_MODULE(secure_module, secure_module_init, secure_module_run, secure_module_stop);
```

### 6.3 配置文件示例

在 `config/agent.cfg` 中添加：

```ini
[secure_module]
permissions = run,send,recv,file
quota = 2097152
```

---

## 七、安全最佳实践

### 7.1 最小权限原则

```c
// ✅ 好的做法：只授予必要的权限
uint32_t minimal_perms = IDCU_PERM_RUN;  // 只给运行权限

// ❌ 避免：给予过多权限
uint32_t too_many_perms = IDCU_PERM_ALL;  // 不要这样做！
```

### 7.2 权限检查

```c
// ✅ 好的做法：在每次敏感操作前检查
int safe_send_message(idcu_Sandbox* sb, Message* msg)
{
    if (idcu_sandbox_perm_check(sb, IDCU_PERM_SEND) != IDCU_ERR_OK) {
        idcu_log_error("Permission denied: SEND required");
        return IDCU_ERR_PERMISSION;
    }
    return do_send_message(msg);
}

// ❌ 避免：不检查权限直接操作
int unsafe_send_message(Message* msg)
{
    return do_send_message(msg);  // 危险！
}
```

### 7.3 动态权限调整

```c
void dynamic_permission_example(idcu_Sandbox* sb)
{
    // 默认权限
    idcu_sandbox_set_perm(sb, IDCU_PERM_RUN);
    
    // 临时添加文件权限
    idcu_sandbox_add_perm(sb, IDCU_PERM_FILE);
    
    // 执行文件操作
    do_file_operation();
    
    // 操作完成后移除权限
    idcu_sandbox_remove_perm(sb, IDCU_PERM_FILE);
}
```

### 7.4 审计日志

```c
// ✅ 好的做法：记录权限检查
int audited_perm_check(idcu_Sandbox* sb, uint32_t perm, const char* operation)
{
    int ret = idcu_sandbox_perm_check(sb, perm);
    if (ret == IDCU_ERR_OK) {
        idcu_log_info("[Audit] Module %u allowed: %s", sb->module_id, operation);
    } else {
        idcu_log_warn("[Audit] Module %u denied: %s", sb->module_id, operation);
    }
    return ret;
}
```

---

## 八、常见安全场景

### 8.1 第三方模块

```c
// 加载第三方模块时使用严格的沙箱
void load_untrusted_module(uint32_t module_id)
{
    idcu_Sandbox sb;
    
    // 只给最基本的权限
    uint32_t restricted_perms = IDCU_PERM_RUN;
    
    idcu_sandbox_init(&sb, module_id, restricted_perms);
    idcu_sandbox_set_quota(&sb, 65536);  // 64KB 配额
    
    idcu_sandbox_registry_add(&sb);
    
    idcu_log_info("Untrusted module %u loaded with restricted permissions", module_id);
}
```

### 8.2 网络访问控制

```c
// 网络操作前检查
int safe_network_connect(idcu_Sandbox* sb, const char* host, int port)
{
    if (idcu_sandbox_perm_check(sb, IDCU_PERM_NETWORK) != IDCU_ERR_OK) {
        idcu_log_error("Network access denied for module %u", sb->module_id);
        return IDCU_ERR_PERMISSION;
    }
    
    idcu_log_info("[Network] Module %u connecting to %s:%d", sb->module_id, host, port);
    return do_network_connect(host, port);
}
```

---

## 下一步

恭喜你完成了所有教程！现在你已经掌握了 IDCU Agent 的核心功能：

- [教程一：快速入门](./tutorial_01_quick_start.md)
- [教程二：模块开发入门](./tutorial_02_module_development.md)
- [教程三：消息总线使用](./tutorial_03_message_bus.md)
- [教程四：协程调度详解](./tutorial_04_coroutine.md)
- [教程五：沙箱安全机制](./tutorial_05_sandbox.md)

接下来你可以：
- 深入阅读源代码，理解实现细节
- 开发自己的模块
- 参与社区贡献
- 查看 [架构文档](./architecture.md) 了解更多设计细节

---

祝你安全编程愉快！🛡️
