# Utils - 工具模块

本模块提供各种实用工具函数和类。

## 模块内容

### log.h/c - 日志系统
提供灵活的日志记录功能：
- 多个日志级别 (DEBUG, INFO, WARN, ERROR, FATAL)
- 可配置的日志输出文件
- 格式化日志输出
- 带文件名和行号的日志宏

### memory_pool.h/c - 内存池
高效的内存池管理：
- 多种大小类别的内存块
- 线程安全的分配和释放
- 内存使用统计
- 峰值使用量追踪

### json_parser.h/c - JSON 解析器
轻量级 JSON 解析和生成：
- JSON 值类型 (null, bool, int, double, string, array, object)
- 从字符串解析 JSON
- 生成 JSON 字符串
- 类型安全的访问接口

### config_manager.h/c - 配置管理器
INI 格式配置文件管理：
- 读取和解析配置文件
- 支持多个 section 和 key-value
- 获取各种类型的配置值 (string, int, int64, double, bool)
- 修改和保存配置
- 线程安全的配置访问

## 使用示例

### 使用日志
```c
#include "utils/log.h"

idcu_log_init("agent.log", IDCU_LOG_INFO);
IDCU_LOG_INFO("系统启动，版本: %s", "1.0.0");
IDCU_LOG_DEBUG("调试信息");
IDCU_LOG_ERROR("发生错误: %d", error_code);
```

### 使用内存池
```c
#include "utils/memory_pool.h"

idcu_MemoryPool pool;
idcu_mem_pool_init(&pool);

void* ptr = idcu_mem_pool_alloc(&pool, 128);
idcu_mem_pool_free(&pool, ptr);

idcu_mem_pool_destroy(&pool);
```

### 使用配置管理器
```c
#include "utils/config_manager.h"

idcu_config_init("config/agent.cfg");
const char* log_level = idcu_config_get_string("general", "log_level", "info");
int max_modules = idcu_config_get_int("general", "max_modules", 32);
idcu_config_shutdown();
```
