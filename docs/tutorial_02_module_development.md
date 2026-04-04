# 教程二：模块开发入门

在 [教程一](./tutorial_01_quick_start.md) 中，你已经学会了如何创建一个简单的模块。现在让我们深入学习模块开发的核心概念和高级技巧。

---

## 学习目标

完成本教程后，你将学会：
- 模块生命周期的完整理解
- 如何使用配置系统
- 如何使用日志系统
- 模块间的基本交互
- 最佳实践和常见陷阱

---

## 一、模块生命周期详解

每个模块都有三个关键阶段：初始化、运行和停止。让我们深入了解每个阶段。

### 1.1 初始化阶段 (init)

`init()` 函数在模块加载时调用一次，用于：
- 分配资源
- 初始化数据结构
- 读取配置
- 注册消息订阅

**示例：**
```c
#include "module/module_def.h"
#include "utils/config_manager.h"
#include "utils/log.h"
#include <stdlib.h>

// 模块私有数据
static struct {
    int max_items;
    char* buffer;
} my_module_data;

static int my_module_init(void)
{
    idcu_log_info("[my_module] Initializing...");
    
    // 读取配置
    my_module_data.max_items = idcu_config_get_int("my_module", "max_items", 100);
    
    // 分配内存
    my_module_data.buffer = (char*)malloc(1024);
    if (!my_module_data.buffer) {
        idcu_log_error("[my_module] Failed to allocate memory!");
        return -1;  // 返回非零表示初始化失败
    }
    
    idcu_log_info("[my_module] Initialized with max_items=%d", my_module_data.max_items);
    return 0;  // 返回 0 表示成功
}
```

### 1.2 运行阶段 (run)

`run()` 函数在内核主循环中被反复调用，注意：
- **不要写死循环！** 这会阻塞其他模块
- 函数应该快速返回
- 使用状态机来管理复杂逻辑

**好的示例：**
```c
static int my_module_run(void)
{
    static int state = 0;
    
    switch (state) {
        case 0:
            // 第一步
            do_something();
            state = 1;
            break;
        case 1:
            // 第二步
            do_something_else();
            state = 0;
            break;
    }
    
    return 0;
}
```

**坏的示例：**
```c
static int my_module_run(void)
{
    // ❌ 不要这样做！这会阻塞整个系统
    while (1) {
        do_something();
    }
    return 0;
}
```

### 1.3 停止阶段 (stop)

`stop()` 函数在模块卸载时调用，用于：
- 释放资源
- 保存数据
- 清理状态

**示例：**
```c
static int my_module_stop(void)
{
    idcu_log_info("[my_module] Stopping...");
    
    // 释放内存
    if (my_module_data.buffer) {
        free(my_module_data.buffer);
        my_module_data.buffer = NULL;
    }
    
    idcu_log_info("[my_module] Stopped");
    return 0;
}
```

---

## 二、使用配置系统

IDCU Agent 提供了便捷的配置管理系统。

### 2.1 配置文件格式

在 `config/agent.cfg` 中添加你的模块配置：

```ini
[my_module]
enabled = true
max_items = 200
data_path = /var/lib/my_module
debug_mode = false
```

### 2.2 读取配置

```c
#include "utils/config_manager.h"

static void read_config(void)
{
    // 读取整数
    int max_items = idcu_config_get_int("my_module", "max_items", 100);
    
    // 读取字符串
    const char* data_path = idcu_config_get_string("my_module", "data_path", "/tmp");
    
    // 读取布尔值
    bool debug_mode = idcu_config_get_bool("my_module", "debug_mode", false);
    
    idcu_log_info("Config: max_items=%d, path=%s, debug=%d", 
                  max_items, data_path, debug_mode);
}
```

---

## 三、使用日志系统

良好的日志记录对于调试和监控至关重要。

### 3.1 日志级别

```c
#include "utils/log.h"

void log_examples(void)
{
    // 调试信息 - 最详细的日志
    idcu_log_debug("[my_module] Processing item %d", item_id);
    
    // 一般信息 - 正常运行时的重要事件
    idcu_log_info("[my_module] Started successfully");
    
    // 警告 - 可能有问题但不影响运行
    idcu_log_warn("[my_module] Low memory, performance may be affected");
    
    // 错误 - 需要关注的问题
    idcu_log_error("[my_module] Failed to open file: %s", filename);
}
```

### 3.2 配置日志级别

在 `config/agent.cfg` 中：
```ini
[general]
log_level = info  # 可选: debug, info, warn, error
```

---

## 四、完整的模块示例

让我们创建一个实用的计数器模块。

### 4.1 创建 `counter_module.c`

```c
#include "module/module_def.h"
#include "utils/config_manager.h"
#include "utils/log.h"
#include <stdio.h>
#include <time.h>

// 模块状态
typedef struct {
    int counter;
    int interval;     // 打印间隔（秒）
    time_t last_print;
    bool running;
} CounterModule;

static CounterModule g_counter;

static int counter_init(void)
{
    idcu_log_info("[counter] Initializing counter module...");
    
    // 读取配置
    g_counter.interval = idcu_config_get_int("counter", "interval", 5);
    
    // 初始化状态
    g_counter.counter = 0;
    g_counter.last_print = time(NULL);
    g_counter.running = true;
    
    idcu_log_info("[counter] Initialized with interval=%d seconds", g_counter.interval);
    return 0;
}

static int counter_run(void)
{
    if (!g_counter.running) {
        return 0;
    }
    
    // 增加计数
    g_counter.counter++;
    
    // 检查是否到了打印时间
    time_t now = time(NULL);
    if (now - g_counter.last_print >= g_counter.interval) {
        idcu_log_info("[counter] Count: %d (elapsed: %ld seconds)", 
                      g_counter.counter, now - g_counter.last_print);
        g_counter.last_print = now;
    }
    
    return 0;
}

static int counter_stop(void)
{
    idcu_log_info("[counter] Stopping... Final count: %d", g_counter.counter);
    g_counter.running = false;
    idcu_log_info("[counter] Stopped");
    return 0;
}

IDCU_REGISTER_MODULE(counter, counter_init, counter_run, counter_stop);
```

### 4.2 添加配置

在 `config/agent.cfg` 中添加：
```ini
[counter]
interval = 3  # 每 3 秒打印一次
```

### 4.3 更新 CMakeLists.txt

不要忘记把模块添加到构建系统中！

---

## 五、最佳实践

### 5.1 代码组织

```c
// ✅ 好的做法：使用静态变量保持模块状态
static struct {
    int state;
    void* data;
} g_my_module;

// ❌ 避免：使用过多全局变量
int g_state;
void* g_data;
```

### 5.2 错误处理

```c
static int do_something(void)
{
    FILE* fp = fopen("data.txt", "r");
    if (!fp) {
        idcu_log_error("[my_module] Failed to open file");
        return -1;
    }
    
    // 使用文件...
    
    fclose(fp);
    return 0;
}
```

### 5.3 性能考虑

```c
static int my_module_run(void)
{
    static int skip_count = 0;
    
    // 跳过一些调用，减少 CPU 使用率
    if (++skip_count < 10) {
        return 0;
    }
    skip_count = 0;
    
    // 实际工作...
    do_real_work();
    
    return 0;
}
```

---

## 六、常见陷阱

### 陷阱 1: 阻塞 run() 函数

```c
// ❌ 错误：阻塞了整个系统
static int bad_run(void)
{
    sleep(1);  // 不要这样做！
    return 0;
}

// ✅ 正确：使用非阻塞方式
static int good_run(void)
{
    static time_t last_time = 0;
    time_t now = time(NULL);
    
    if (now - last_time >= 1) {
        do_work();
        last_time = now;
    }
    return 0;
}
```

### 陷阱 2: 忘记释放资源

```c
// ❌ 错误：内存泄漏
static int bad_init(void)
{
    char* buf = malloc(1024);
    // 没有在 stop() 中释放
    return 0;
}

// ✅ 正确：配对的分配和释放
static char* g_buf = NULL;

static int good_init(void)
{
    g_buf = malloc(1024);
    return 0;
}

static int good_stop(void)
{
    if (g_buf) {
        free(g_buf);
        g_buf = NULL;
    }
    return 0;
}
```

### 陷阱 3: init() 中返回错误

```c
// ✅ 正确：初始化失败时返回非零
static int init_with_error_check(void)
{
    if (something_failed()) {
        idcu_log_error("Initialization failed");
        return -1;  // 模块不会被加载
    }
    return 0;
}
```

---

## 下一步

恭喜你完成了模块开发入门教程！接下来你可以：

- 阅读 [教程三：消息总线使用](./tutorial_03_message_bus.md)，学习模块间通信
- 查看现有模块的源代码，学习更多技巧
- 尝试创建自己的业务模块

---

祝你开发愉快！🚀
