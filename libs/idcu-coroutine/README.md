# idcu-coroutine

IDCU Agent 的协程调度器库。

## 功能特性

- **协程上下文切换**: Windows Fiber / Linux ucontext 支持
- **调度策略**: 轮询 (Round-Robin) 调度算法
- **协程管理**: 创建、销毁、挂起、恢复协程
- **跨平台**: 支持 Windows、Linux、macOS

## 快速开始

### 创建和运行协程

```c
#include <idcu/coroutine/coroutine.h>
#include <stdio.h>

void my_coroutine(void* arg) {
    int count = *(int*)arg;
    for (int i = 0; i < count; i++) {
        printf("Coroutine: %d\n", i);
        idcu_coro_yield();
    }
}

int main() {
    idcu_CoroutineScheduler* scheduler;
    idcu_coro_scheduler_init(&scheduler);

    int count1 = 3;
    int count2 = 5;

    idcu_Coroutine* coro1;
    idcu_Coroutine* coro2;

    idcu_CoroutineConfig config = {
        .stack_size = IDCU_COROUTINE_DEFAULT_STACK_SIZE,
        .name = "coro1"
    };

    idcu_coro_create(scheduler, &coro1, my_coroutine, &count1, &config);
    
    config.name = "coro2";
    idcu_coro_create(scheduler, &coro2, my_coroutine, &count2, &config);

    idcu_coro_scheduler_run(scheduler);
    idcu_coro_scheduler_destroy(scheduler);

    return 0;
}
```

## API 文档

### 调度器 API

- `idcu_coro_scheduler_init()`: 初始化协程调度器
- `idcu_coro_scheduler_destroy()`: 销毁协程调度器
- `idcu_coro_scheduler_run()`: 运行调度器
- `idcu_coro_scheduler_stop()`: 停止调度器

### 协程 API

- `idcu_coro_create()`: 创建协程
- `idcu_coro_destroy()`: 销毁协程
- `idcu_coro_yield()`: 让出 CPU
- `idcu_coro_resume()`: 恢复协程
- `idcu_coro_get_state()`: 获取协程状态
- `idcu_coro_get_name()`: 获取协程名称
- `idcu_coro_current()`: 获取当前协程

## 配置选项

| 选项 | 默认值 | 说明 |
|-----|-------|------|
| `IDCU_COROUTINE_DEFAULT_STACK_SIZE` | 64KB | 默认协程栈大小 |
| `IDCU_COROUTINE_MAX_COROUTINES` | 128 | 最大协程数 |

## 构建

```bash
cd build
cmake ..
cmake --build .
```
