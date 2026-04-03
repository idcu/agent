# Kernel - 微内核核心

本模块是整个系统的核心，负责协调和管理所有其他模块。

## 模块内容

### micro_kernel.h/c - 微内核
系统的核心管理器，整合所有子系统：
- 协程调度器 (idcu_CoroScheduler)
- 消息总线 (idcu_MessageBus)
- 沙箱数组 (idcu_Sandbox[16])
- 全局栈上下文 (idcu_StackContext)

## 核心功能

### 1. idcu_kernel_init()
初始化微内核的所有子系统：
- 初始化协程调度器
- 初始化消息总线
- 初始化沙箱环境

### 2. idcu_kernel_start_modules()
启动所有注册的模块：
- 遍历模块列表
- 调用每个模块的 init() 函数
- 设置模块状态

### 3. idcu_kernel_run()
主事件循环：
- 运行协程调度器
- 处理消息总线
- 持续运行直到停止

## 使用示例

```c
#include "kernel/micro_kernel.h"

int main(void) {
    idcu_MicroKernel kernel;
    
    // 初始化内核
    idcu_kernel_init(&kernel);
    
    // 启动模块
    idcu_kernel_start_modules(&kernel);
    
    // 运行主循环
    idcu_kernel_run(&kernel);
    
    return 0;
}
```

## idcu_MicroKernel 结构体

```c
typedef struct {
    idcu_CoroScheduler coro;      // 协程调度器
    idcu_MessageBus    msg;       // 消息总线
    idcu_Sandbox       sandbox[16]; // 沙箱环境数组
    uint32_t           sb_cnt;    // 沙箱数量
    idcu_StackContext  global;    // 全局上下文
} idcu_MicroKernel;
```

## 工作流程

1. **初始化阶段**：`idcu_kernel_init()` 初始化所有子系统
2. **模块启动**：`idcu_kernel_start_modules()` 加载并初始化所有模块
3. **主循环**：`idcu_kernel_run()` 进入无限循环，调度协程和处理消息
4. **终止**：通过信号或其他机制退出主循环
