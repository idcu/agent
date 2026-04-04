# Security - 安全模块

本模块提供沙箱安全机制，保护系统免受恶意模块的影响。

## 模块内容

### sandbox.h/c - 基础沙箱
基础的模块隔离和权限控制：
- 模块 ID 和权限掩码
- 权限位定义 (SEND, RECV, RUN, HW 等)
- 配额管理
- 栈上下文保存
- 权限检查函数 `idcu_sandbox_perm_check()`

### sandbox_enhanced.h/c - 增强沙箱
更强大的安全控制机制：
- 内存区域保护 (可读、可写、可执行)
- 系统调用白名单
- CPU 时间限制
- 内存使用限制
- 文件描述符限制
- 资源使用统计
- 最大支持 32 个内存区域和 64 个系统调用

## 权限位定义

| 权限位 | 说明 |
|--------|------|
| IDCU_PERM_SEND | 允许发送消息 |
| IDCU_PERM_RECV | 允许接收消息 |
| IDCU_PERM_RUN | 允许运行代码 |
| IDCU_PERM_HW | 允许访问硬件 |
| IDCU_PERM_CONFIG | 允许修改配置 |
| IDCU_PERM_LOG | 允许访问日志 |
| IDCU_PERM_DEBUG | 允许调试操作 |
| IDCU_PERM_MODULE_MGR | 允许管理模块 |

## 使用示例

### 使用基础沙箱
```c
#include "security/sandbox.h"

idcu_Sandbox sb;
sb.module_id = 1;
sb.perm = IDCU_PERM_SEND | IDCU_PERM_RECV | IDCU_PERM_RUN;
sb.quota = 1000;

// 检查权限
if (idcu_sandbox_perm_check(&sb, IDCU_PERM_SEND) == 0) {
    // 有权限发送消息
}
```

### 使用增强沙箱
```c
#include "security/sandbox_enhanced.h"

idcu_EnhancedSandbox sb;
idcu_enhanced_sandbox_init(&sb, 1, IDCU_PERM_RUN);

// 添加内存区域
idcu_enhanced_sandbox_add_memory_region(&sb, buffer, size, 
    IDCU_MEM_REGION_READ | IDCU_MEM_REGION_WRITE);

// 添加允许的系统调用
idcu_enhanced_sandbox_add_syscall(&sb, IDCU_SYS_read, "read");
idcu_enhanced_sandbox_add_syscall(&sb, IDCU_SYS_write, "write");

// 设置资源限制
idcu_enhanced_sandbox_set_cpu_limit(&sb, 1000);  // 1秒
idcu_enhanced_sandbox_set_memory_limit(&sb, 1024*1024);  // 1MB

// 检查内存访问
idcu_enhanced_sandbox_check_memory_access(&sb, ptr, size, IDCU_MEM_REGION_READ);

// 检查系统调用
idcu_enhanced_sandbox_check_syscall(&sb, IDCU_SYS_read);

idcu_enhanced_sandbox_destroy(&sb);
```
