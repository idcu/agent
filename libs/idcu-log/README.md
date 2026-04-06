# idcu-log

IDCU 项目的独立日志库，提供灵活高效的日志功能，支持控制台输出和文件输出。

## 特性

- 跨平台支持（Windows、Linux、macOS）
- 多级别日志（DEBUG、INFO、WARN、ERROR、FATAL）
- 支持同时输出到控制台和文件
- 自动添加时间戳、文件名、行号信息
- 易于使用的宏封装

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
find_package(idcu-common REQUIRED)
find_package(idcu-log REQUIRED)
add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE idcu::log)
```

## API 文档

### 初始化和清理

```c
#include <idcu/log/log.h>

// 初始化日志
int idcu_log_init(const char* filename, idcu_LogLevel level);

// 关闭日志
void idcu_log_shutdown(void);
```

### 日志级别控制

```c
// 设置日志级别
void idcu_log_set_level(idcu_LogLevel level);

// 获取当前日志级别
idcu_LogLevel idcu_log_get_level(void);
```

### 输出配置

```c
// 设置日志文件
int idcu_log_set_file(const char* filename);
```

### 日志宏

```c
// 输出调试信息
IDCU_LOG_DEBUG("This is a debug message: %d", 42);

// 输出一般信息
IDCU_LOG_INFO("Application started");

// 输出警告信息
IDCU_LOG_WARN("Memory usage is high: %d MB", 90);

// 输出错误信息
IDCU_LOG_ERROR("Failed to open file: %s", filename);

// 输出致命错误（会终止程序）
IDCU_LOG_FATAL("Critical error, exiting");
```

### 日志级别

```c
typedef enum {
    IDCU_LOG_DEBUG   = 0,  // 调试信息
    IDCU_LOG_INFO    = 1,  // 一般信息
    IDCU_LOG_WARN    = 2,  // 警告信息
    IDCU_LOG_ERROR   = 3,  // 错误信息
    IDCU_LOG_FATAL   = 4   // 致命错误
} idcu_LogLevel;
```

## 测试

```bash
cd build
ctest
```

## 示例

库提供了以下示例代码：

- [简单日志示例](examples/example_log.c) - 演示日志的基本使用
- [文件日志示例](examples/example_file_log.c) - 演示如何输出日志到文件
- [多级别日志示例](examples/example_multi_level.c) - 演示不同日志级别的用法

### 编译和运行示例

```bash
cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build .

# 运行简单日志示例
./example_log

# 运行文件日志示例
./example_file_log

# 运行多级别日志示例
./example_multi_level
```

## API 文档

详细的 API 文档请参考：[idcu-log API 文档](../../docs/api/idcu-log.md)

## 依赖

- idcu-common - IDCU 基础通用组件库

## 许可证

详见项目根目录的 LICENSE 文件。
