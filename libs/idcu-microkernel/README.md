# idcu-microkernel

IDCU Agent 的微内核核心库，集成了所有核心组件，包括模块系统、协程调度器和消息总线。

## 功能特性

- 内核生命周期管理（init、start、run、stop、destroy）
- 集成模块系统、协程调度器和消息总线
- 主事件循环
- 线程安全操作

## 使用方法

```c
#include <idcu/microkernel/kernel.h>

int main() {
    idcu_KernelConfig config = {
        .config_path = "config.yaml",
        .enable_logging = true,
        .log_level = 1,
        .max_modules = 32
    };

    idcu_MicroKernel* kernel = NULL;
    idcu_kernel_init(&kernel, &config);
    idcu_kernel_start(kernel);
    idcu_kernel_run(kernel);
    idcu_kernel_stop(kernel);
    idcu_kernel_destroy(kernel);

    return 0;
}
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
