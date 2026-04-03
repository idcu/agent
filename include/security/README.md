# Security - 安全模块

本模块提供沙箱安全机制，保护系统免受恶意模块的影响。

## 模块内容

### sandbox.h/c - 基础沙箱
基础的模块隔离和权限控制：
- 模块 ID 和权限掩码
- 权限位定义 (SEND, RECV, RUN, HW 等)
- 配额管理
- 栈上下文保存
- 权限检查函数 `sandbox_perm_check()`

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
| PERM_SEND | 允许发送消息 |
| PERM_RECV | 允许接收消息 |
| PERM_RUN | 允许运行代码 |
| PERM_HW | 允许访问硬件 |
| PERM_CONFIG | 允许修改配置 |
| PERM_LOG | 允许访问日志 |
| PERM_DEBUG | 允许调试操作 |
| PERM_MODULE_MGR | 允许管理模块 |

## 使用示例

### 使用基础沙箱
```c
#include "security/sandbox.h"

Sandbox sb;
sb.module_id = 1;
sb.perm = PERM_SEND | PERM_RECV | PERM_RUN;
sb.quota = 1000;

// 检查权限
if (sandbox_perm_check(&sb, PERM_SEND) == 0) {
    // 有权限发送消息
}
```

### 使用增强沙箱
```c
#include "security/sandbox_enhanced.h"

EnhancedSandbox sb;
enhanced_sandbox_init(&sb, 1, PERM_RUN);

// 添加内存区域
enhanced_sandbox_add_memory_region(&sb, buffer, size, 
    MEM_REGION_READ | MEM_REGION_WRITE);

// 添加允许的系统调用
enhanced_sandbox_add_syscall(&sb, SYS_read, "read");
enhanced_sandbox_add_syscall(&sb, SYS_write, "write");

// 设置资源限制
enhanced_sandbox_set_cpu_limit(&sb, 1000);  // 1秒
enhanced_sandbox_set_memory_limit(&sb, 1024*1024);  // 1MB

// 检查内存访问
enhanced_sandbox_check_memory_access(&sb, ptr, size, MEM_REGION_READ);

// 检查系统调用
enhanced_sandbox_check_syscall(&sb, SYS_read);

enhanced_sandbox_destroy(&sb);
```
