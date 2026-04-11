# 调试技巧与窍门

本指南提供用于调试 IDCU Agent 应用程序和库的技巧和技术。

## 目录
- [内存调试](#内存调试)
- [性能分析](#性能分析)
- [日志分析](#日志分析)
- [模块检查](#模块检查)
- [常见问题](#常见问题)

---

## 内存调试

### 使用内存调试器

IDCU Agent 包含一个内置的内存调试器，用于帮助跟踪内存泄漏和检测无效的内存操作。

#### 启用内存调试

要启用内存调试，请在包含任何 IDCU 头文件之前定义 `IDCU_DEBUG_MEMORY` 宏：

```c
#define IDCU_DEBUG_MEMORY
#include <idcu/common/config.h>
#include <idcu/memory/memory.h>
```

#### 内存调试器 API

```c
// 初始化内存调试器
int idcu_memory_debugger_init(void);

// 销毁内存调试器并打印泄漏报告
void idcu_memory_debugger_destroy(void);

// 获取当前分配计数
size_t idcu_memory_debugger_get_allocated_count(void);

// 获取总分配字节数
size_t idcu_memory_debugger_get_total_allocated(void);

// 打印内存泄漏报告
void idcu_memory_debugger_print_leaks(void);

// 重置所有分配
void idcu_memory_debugger_reset(void);
```

#### 使用示例

```c
#include "memory_debugger.h"

int main(void) {
    idcu_memory_debugger_init();
    
    // 您的应用程序代码在此处
    void* ptr = idcu_malloc(100);
    // ... 使用 ptr ...
    idcu_free(ptr);
    
    // 打印泄漏报告
    idcu_memory_debugger_print_leaks();
    
    idcu_memory_debugger_destroy();
    return 0;
}
```

#### 内存泄漏检测

内存调试器会自动跟踪所有分配，并在销毁时报告泄漏。示例输出：

```
=== 内存泄漏报告 ===
  泄漏 #1: 0x55aabbccddeeff (100 字节) at src/main.c:42
  泄漏 #2: 0x55aabbccddee00 (256 字节) at src/utils.c:15
  总计: 2 个泄漏，356 字节
==========================
```

---

## 性能分析

### 使用性能分析器

性能分析器通过测量函数和代码块的执行时间来帮助识别代码中的瓶颈。

#### 启用分析

要启用分析，请定义 `IDCU_PROFILE` 宏：

```c
#define IDCU_PROFILE
#include "performance_profiler.h"
```

#### 分析器 API

```c
// 初始化分析器
int idcu_performance_profiler_init(void);

// 销毁分析器并打印报告
void idcu_performance_profiler_destroy(void);

// 开始分析一个代码段
void idcu_performance_start(const char* name);

// 停止分析一个代码段
void idcu_performance_stop(const char* name);

// 打印性能报告
void idcu_performance_profiler_print_report(void);

// 重置所有指标
void idcu_performance_profiler_reset(void);
```

#### 使用示例

```c
#include "performance_profiler.h"

void expensive_function(void) {
    idcu_performance_start("expensive_function");
    
    // 您的耗时代码在此处
    for (int i = 0; i < 1000000; i++) {
        // 执行某些操作
    }
    
    idcu_performance_stop("expensive_function");
}

int main(void) {
    idcu_performance_profiler_init();
    
    expensive_function();
    expensive_function();
    
    idcu_performance_profiler_print_report();
    idcu_performance_profiler_destroy();
    return 0;
}
```

#### 分析器报告

示例分析器输出：

```
=== 性能分析器报告 ===
名称                           调用次数      总计 (毫秒)      平均 (纳秒)      最小 (纳秒)      最大 (纳秒)
--------------------------------------------------------------------------------------------------------
expensive_function                2          125.432       62716000       62500000       62932000
another_function                 10            8.234          823400          750000          950000
========================================================================================================
```

---

## 日志分析

### 使用日志分析器

日志分析器帮助解析、过滤和分析 IDCU Agent 生成的日志文件。

#### 日志分析器 API

```c
// 使用日志文件初始化日志分析器
int idcu_log_analyzer_init(const char* log_file);

// 销毁日志分析器
void idcu_log_analyzer_destroy(void);

// 获取日志条目总数
size_t idcu_log_analyzer_get_entry_count(void);

// 获取特定的日志条目
int idcu_log_analyzer_get_entry(size_t index, idcu_LogEntry* entry);

// 获取日志统计信息
void idcu_log_analyzer_get_stats(idcu_LogStats* stats);

// 打印日志统计信息
void idcu_log_analyzer_print_stats(void);

// 按日志级别过滤条目
size_t idcu_log_analyzer_filter_by_level(idcu_LogLevel level, 
                                          idcu_LogEntry** entries, 
                                          size_t max_entries);

// 按关键词搜索条目
size_t idcu_log_analyzer_search(const char* keyword, 
                                 idcu_LogEntry** entries, 
                                 size_t max_entries);

// 获取所有错误和致命错误条目
size_t idcu_log_analyzer_get_errors(idcu_LogEntry** entries, 
                                     size_t max_entries);
```

#### 使用示例

```c
#include "log_analyzer.h"

int main(void) {
    if (idcu_log_analyzer_init("app.log") != 0) {
        printf("无法打开日志文件\n");
        return 1;
    }
    
    // 打印统计信息
    idcu_log_analyzer_print_stats();
    
    // 获取所有错误
    idcu_LogEntry* errors[100];
    size_t error_count = idcu_log_analyzer_get_errors(errors, 100);
    
    printf("\n找到 %zu 个错误:\n", error_count);
    for (size_t i = 0; i < error_count; i++) {
        printf("  %.*s\n", (int)errors[i]->message_len, errors[i]->message);
    }
    
    idcu_log_analyzer_destroy();
    return 0;
}
```

---

## 模块检查

### 使用模块检查器

模块检查器提供对模块系统的可见性，显示已加载的模块、它们的状态和依赖关系。

#### 模块检查器 API

```c
// 初始化模块检查器
int idcu_module_inspector_init(void);

// 销毁模块检查器
void idcu_module_inspector_destroy(void);

// 获取模块总数
size_t idcu_module_inspector_get_module_count(void);

// 获取特定模块的信息
int idcu_module_inspector_get_module(size_t index, idcu_ModuleInfo* info);

// 按名称查找模块
int idcu_module_inspector_find_module(const char* name, idcu_ModuleInfo* info);

// 获取模块系统统计信息
void idcu_module_inspector_get_stats(idcu_ModuleSystemStats* stats);

// 打印模块系统统计信息
void idcu_module_inspector_print_stats(void);

// 打印所有模块
void idcu_module_inspector_print_all(void);

// 打印特定模块
void idcu_module_inspector_print_module(const char* name);

// 将模块状态转换为字符串
const char* idcu_module_state_to_string(idcu_ModuleState state);
```

#### 使用示例

```c
#include "module_inspector.h"
#include <idcu/module/module.h>

int main(void) {
    idcu_module_inspector_init();
    
    // 打印所有模块
    idcu_module_inspector_print_all();
    
    // 打印统计信息
    idcu_module_inspector_print_stats();
    
    // 检查特定模块
    idcu_module_inspector_print_module("core-module");
    
    idcu_module_inspector_destroy();
    return 0;
}
```

---

## 常见问题

### 内存泄漏

**症状**：内存使用量随时间增加，内存调试器报告泄漏。

**解决方案**：
1. 使用 `IDCU_DEBUG_MEMORY` 启用内存调试
2. 检查每个 `idcu_malloc` 是否有对应的 `idcu_free`
3. 使用 RAII 模式或清理函数
4. 检查是否有跳过清理的提前返回

### 模块加载失败

**症状**：模块无法初始化或启动。

**解决方案**：
1. 检查模块依赖是否已先加载
2. 验证模块配置是否正确
3. 检查日志文件中的错误消息
4. 使用模块检查器验证模块状态

### 性能问题

**症状**：响应时间慢，CPU 使用率高。

**解决方案**：
1. 使用性能分析器识别瓶颈
2. 检查不必要的内存分配
3. 考虑对频繁分配使用内存池
4. 优化消息总线使用
5. 检查并发代码中的锁争用

### 网络问题

**症状**：连接失败，发生超时。

**解决方案**：
1. 检查网络配置
2. 验证防火墙设置
3. 检查日志文件中的网络错误
4. 首先使用简单的网络示例进行测试
5. 验证超时设置是否适当

---

## 最佳实践

1. **在开发期间始终启用调试工具**
2. **为新功能编写测试**
3. **大量使用日志记录**
4. **在优化之前进行分析**
5. **审查代码的内存安全性**
6. **记录复杂的逻辑**

---

## 其他资源

- [API 文档](../api/)
- [快速入门指南](QUICKSTART.md)
- [架构概述](ARCHITECTURE.md)
- [模块系统指南](MODULE_SYSTEM.md)
