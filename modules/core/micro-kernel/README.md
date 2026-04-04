# Micro Kernel 模块

## 功能说明

微内核模块，提供系统最核心的功能：
- 协程调度器的管理
- 消息总线的初始化和维护
- 沙箱环境的创建
- 模块的加载、初始化和启动
- 健康状态监控

## API 文档

### 微内核 (micro_kernel.h)
- `idcu_kernel_init()` - 初始化微内核
- `idcu_kernel_start_modules()` - 启动所有模块
- `idcu_kernel_run()` - 运行主循环
- `idcu_kernel_stop()` - 停止所有模块
- `idcu_kernel_hotplug_load()` - 热加载模块
- `idcu_kernel_hotplug_unload()` - 热卸载模块

## 依赖关系

- idcu_core_common
- idcu_core_utils
- idcu_core_scheduler
- idcu_core_module_system

## 使用示例

```c
#include "kernel/micro_kernel.h"

int main() {
    idcu_kernel_init();
    idcu_kernel_start_modules();
    idcu_kernel_run();
    idcu_kernel_stop();
    return 0;
}
```
