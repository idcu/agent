# Common 模块

## 模块信息

| 属性 | 值 |
|------|-----|
| **名称** | common_module |
| **类别** | core |
| **版本** | 1.0.0 |
| **描述** | Common utilities module providing atomic operations, locks, error codes, and config definitions |
| **作者** | IDCU Team |
| **许可证** | MIT |
| **优先级** | high |
| **类型** | builtin |

## 功能说明

提供项目通用的基础工具组件，包括：
- 原子操作 (atomic)
- 锁机制 (lock)
- 错误码定义 (error_code)
- 编译时配置 (config)

## API 文档

### 原子操作 (atomic.h)
- 原子变量操作
- 原子计数器
- 无锁数据结构支持

### 锁机制 (lock.h)
- 互斥锁
- 读写锁
- 条件变量

### 错误码 (error_code.h)
- 错误码定义
- 错误码转字符串
- 统一错误处理

## 依赖关系

无外部依赖，是最基础的模块。

## 使用示例

```c
#include "common/atomic.h"
#include "common/lock.h"
#include "common/error_code.h"
```
