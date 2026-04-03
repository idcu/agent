# Scheduler - 调度模块

本模块提供协程调度和消息传递功能。

## 模块内容

### coroutine.h/c - 协程调度器
轻量级协程调度系统：
- 多优先级协程 (0-63)
- 时间片轮转调度
- 协程挂起和恢复
- 协程性能统计 (运行时间、切换次数等)
- 最大支持 256 个协程

### msg_bus.h/c - 消息总线
模块间通信的基础设施：
- 4 级优先级消息队列 (LOW, NORMAL, HIGH, REALTIME)
- 零拷贝消息传输
- 点对点消息发送
- 消息广播
- 批量消息处理
- 消息重试机制

### context.h - 栈上下文
消息传递的数据结构：
- 模块 ID 和消息 ID
- 时间戳
- 数据缓冲区
- 便捷的初始化函数

## 使用示例

### 使用协程
```c
#include "scheduler/coroutine.h"

CoroScheduler sched;
coro_sched_init(&sched);

CoroState my_coro_func(Coroutine* coro) {
    // 协程逻辑
    return CORO_FINISHED;
}

int coro_id = coro_create(&sched, my_coro_func, 0, 10, NULL);
coro_sched_run(&sched);

coro_sched_destroy(&sched);
```

### 使用消息总线
```c
#include "scheduler/msg_bus.h"

MessageBus bus;
msg_bus_init(&bus);

StackContext ctx;
ctx_init(&ctx, module_a_id, msg_id);
msg_send(&bus, module_a_id, module_b_id, MSG_PRIO_NORMAL, &ctx);

Message msg;
msg_recv(&bus, module_b_id, &msg);

msg_bus_destroy(&bus);
```
