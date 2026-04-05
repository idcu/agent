# Scheduler 模块

## 模块信息

| 属性 | 值 |
|------|-----|
| **名称** | scheduler_module |
| **类别** | core |
| **版本** | 1.0.0 |
| **描述** | Scheduler module providing coroutine scheduling, message bus, and context management |
| **作者** | IDCU Team |
| **许可证** | MIT |
| **优先级** | high |
| **类型** | builtin |
| **依赖** | common_module, utils_module |

## 功能说明

调度器模块，提供：
- 协程调度器
- 消息总线
- 上下文管理

## API 文档

### 协程 (coroutine.h)
- 轻量级协程调度
- 多优先级协程
- 时间片轮转调度
- 协程挂起和恢复
- 性能统计

### 消息总线 (msg_bus.h)
- 模块间通信
- 多优先级消息队列
- 零拷贝消息传输
- 消息广播
- 批量消息处理

### 上下文 (context.h)
- 协程上下文管理
- 上下文切换

## 依赖关系

- idcu_core_common
- idcu_core_utils

## 使用示例

```c
#include "scheduler/coroutine.h"
#include "scheduler/msg_bus.h"
```
