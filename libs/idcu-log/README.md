# idcu-log

IDCU Agent 的日志库。

## 功能特性

- 多级日志（DEBUG、INFO、WARN、ERROR、FATAL）
- 控制台和文件输出
- 线程安全的互斥锁日志
- 时间戳、文件和行号信息
- 彩色控制台输出

## 使用方法

```c
#include <idcu/log/log.h>

int main() {
    idcu_log_init("app.log", IDCU_LOG_INFO);
    
    IDCU_LOG_INFO("应用程序已启动");
    IDCU_LOG_WARN("内存不足警告");
    IDCU_LOG_ERROR("无法打开文件: %s", "data.txt");
    
    idcu_log_shutdown();
    return 0;
}
```

## 构建

```bash
cmake -B build
cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
