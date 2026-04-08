# 调试技巧

## 使用 GDB/LLDB 调试

### 启动调试

```bash
# 使用 GDB
gdb ./idcu_agent

# 使用 LLDB
lldb ./idcu_agent
```

### 常用调试命令

| 命令 | 说明 |
|-----|------|
| `break main` | 在 main 函数设置断点 |
| `run` | 运行程序 |
| `next` / `n` | 单步执行（不进入函数） |
| `step` / `s` | 单步执行（进入函数） |
| `print var` / `p var` | 打印变量值 |
| `backtrace` / `bt` | 查看调用栈 |
| `continue` / `c` | 继续执行 |

## 添加调试日志

在关键位置添加调试日志：

```c
#include "idcu/log/log.h"

void some_function() {
    idcu_log_debug("Entering some_function");
    
    // ... 代码 ...
    
    idcu_log_debug("Variable value: %d", some_var);
    idcu_log_debug("Exiting some_function");
}
```

## 使用内存检测工具

### Valgrind（Linux）

```bash
valgrind --leak-check=full ./idcu_agent
```

### AddressSanitizer（GCC/Clang）

编译时添加：

```cmake
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
```

运行时会自动检测内存问题。

## 核心转储分析

启用核心转储：

```bash
# Linux
ulimit -c unlimited

# 分析核心转储
gdb ./idcu_agent core
```
