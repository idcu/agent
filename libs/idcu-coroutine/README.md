# idcu-coroutine

IDCU 项目的协程调度器库，提供轻量级、多优先级的协程调度功能。

## 特性

- 轻量级协程实现
- 多优先级调度（支持 64 个优先级）
- 线程安全
- 跨平台支持（Windows、Linux、macOS）
- 协程状态管理（就绪、运行、暂停、完成）

## 快速开始

### 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 安装

```bash
cmake --install .
```

### 在其他项目中使用

```cmake
find_package(idcu-coroutine REQUIRED)
add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE idcu::coroutine)
```

## API 文档

### 调度器初始化和销毁

```c
#include <idcu/coroutine/coroutine.h>

void idcu_coro_sched_init(idcu_CoroScheduler *sched);
void idcu_coro_sched_destroy(idcu_CoroScheduler *sched);
```

### 协程管理

```c
int idcu_coro_create(idcu_CoroScheduler *sched, idcu_CoroState (*func)(idcu_Coroutine*), 
                     uint32_t prio, uint32_t timeslice, void *user_data);
int idcu_coro_destroy(idcu_CoroScheduler *sched, uint32_t id);
int idcu_coro_suspend(idcu_CoroScheduler *sched, uint32_t id);
int idcu_coro_resume(idcu_CoroScheduler *sched, uint32_t id);
idcu_Coroutine* idcu_coro_get(idcu_CoroScheduler *sched, uint32_t id);
```

### 调度执行

```c
idcu_CoroState idcu_coro_sched_run(idcu_CoroScheduler *sched);
uint32_t idcu_coro_get_ready_count(idcu_CoroScheduler *sched);
```

## 测试

```bash
cd build
ctest
```

## 示例

库提供了以下示例代码：

- [基础协程示例](examples/example_coroutine_basic.c) - 演示协程的基本创建和调度
- [优先级调度示例](examples/example_coroutine_priority.c) - 演示多优先级协程调度

### 编译和运行示例

```bash
cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build .

# 运行基础协程示例
./example_coroutine_basic

# 运行优先级调度示例
./example_coroutine_priority
```

## 许可证

详见项目根目录的 LICENSE 文件。
