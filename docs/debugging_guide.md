# IDCU Agent 调试指南

本指南将帮助你调试 IDCU Agent 项目，包括编译问题、运行时错误、内存问题等的排查和解决方法。

## 目录
- [编译调试](#编译调试)
- [运行时调试](#运行时调试)
- [内存调试](#内存调试)
- [线程调试](#线程调试)
- [日志调试](#日志调试)
- [常用调试工具](#常用调试工具)

---

## 编译调试

### 常见编译错误

#### 1. 找不到头文件
**错误信息：**
```
fatal error: idcu/common/xxx.h: No such file or directory
```

**解决方案：**
- 检查 CMakeLists.txt 中的 include 目录是否正确设置
- 确保依赖的库已正确编译和安装
- 验证头文件路径是否正确

```cmake
target_include_directories(your_target PUBLIC
    ${CMAKE_SOURCE_DIR}/libs/idcu-common/include
)
```

#### 2. 链接错误 - 未定义的引用
**错误信息：**
```
undefined reference to `idcu_xxx_function'
```

**解决方案：**
- 确保链接了正确的库
- 检查库的链接顺序（依赖库放在被依赖库后面）
- 验证库是否已正确编译

```cmake
target_link_libraries(your_target PRIVATE
    idcu::common
    idcu::log
    idcu::network
)
```

#### 3. Windows 特定编译错误

**MinGW 相关问题：**
- 确保使用正确的 CMake 生成器：`-G "MinGW Makefiles"`
- 检查 MinGW 是否在 PATH 中

**MSVC 相关问题：**
- 使用 Visual Studio 开发人员命令提示符
- 确保 CMake 生成器正确：`-G "Visual Studio 16 2019"`

### 调试编译过程

#### 启用详细输出
```bash
# CMake 详细输出
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON

# Make 详细输出
make VERBOSE=1
```

#### 检查编译标志
```cmake
# 调试构建
cmake .. -DCMAKE_BUILD_TYPE=Debug

# 发布构建
cmake .. -DCMAKE_BUILD_TYPE=Release

# 带调试信息的发布构建
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

---

## 运行时调试

### 使用 GDB 调试（Linux）

#### 基本调试步骤
```bash
# 编译调试版本
cmake .. -DCMAKE_BUILD_TYPE=Debug
make

# 启动 GDB
gdb ./out/idcu_agent

# GDB 常用命令
(gdb) break main              # 在 main 函数设置断点
(gdb) run                     # 运行程序
(gdb) next                    # 单步执行（不进入函数）
(gdb) step                    # 单步执行（进入函数）
(gdb) print variable_name     # 打印变量值
(gdb) backtrace               # 查看调用栈
(gdb) continue                # 继续执行
(gdb) quit                    # 退出 GDB
```

#### 调试崩溃
```bash
# 启用核心转储
ulimit -c unlimited

# 程序崩溃后，使用核心转储调试
gdb ./out/idcu_agent core
```

### 使用 Visual Studio 调试（Windows）

1. 生成 Visual Studio 解决方案：
```cmd
cmake .. -G "Visual Studio 16 2019" -A x64
```

2. 打开生成的 `.sln` 文件
3. 设置 `idcu_agent` 为启动项目
4. 按 F5 开始调试

### 调试模块加载问题

#### 启用模块加载日志
在配置文件中设置：
```ini
[general]
log_level = debug
```

#### 检查模块依赖
- 确保所有依赖的模块已启用
- 检查模块初始化顺序
- 验证模块版本兼容性

---

## 内存调试

### 使用 AddressSanitizer（ASAN）

#### 编译启用 ASAN
```bash
# Linux
cmake .. -DCMAKE_BUILD_TYPE=Debug -DUSE_ASAN=ON
make

# 或者使用提供的脚本
./scripts/build_with_asan.sh
```

#### Windows (MinGW)
```cmd
scripts\build_with_asan.bat
```

#### ASAN 常见检测问题
- 内存泄漏
- 堆缓冲区溢出
- 栈缓冲区溢出
- 释放后使用
- 重复释放

### 使用 Valgrind（Linux）

#### 基本使用
```bash
valgrind --leak-check=full ./out/idcu_agent

# 更详细的输出
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./out/idcu_agent
```

#### Valgrind 工具
```bash
# 内存检查
valgrind --tool=memcheck ./out/idcu_agent

# 缓存分析
valgrind --tool=cachegrind ./out/idcu_agent

# 调用图分析
valgrind --tool=callgrind ./out/idcu_agent
```

### 内存调试最佳实践

1. **始终初始化内存**
```c
// 好的做法
void* ptr = malloc(size);
if (ptr) {
    memset(ptr, 0, size);
}

// 或者使用 calloc
void* ptr = calloc(1, size);
```

2. **检查内存分配返回值**
```c
void* ptr = malloc(size);
if (!ptr) {
    // 处理内存分配失败
    return IDCU_ERR_NO_MEMORY;
}
```

3. **配对分配和释放**
```c
// 每个 malloc 都要有对应的 free
void* ptr = malloc(size);
// ... 使用 ptr ...
free(ptr);
ptr = NULL;  // 避免悬空指针
```

---

## 线程调试

### 使用 ThreadSanitizer（TSAN）

#### 编译启用 TSAN
```bash
# Linux
cmake .. -DCMAKE_BUILD_TYPE=Debug -DUSE_TSAN=ON
make

# 或者使用提供的脚本
./scripts/build_with_tsan.sh
```

#### Windows (MinGW)
```cmd
scripts\build_with_tsan.bat
```

#### TSAN 检测的问题
- 数据竞争
- 死锁
- 线程泄漏

### 死锁检测

#### 使用项目自带的死锁检测器
```c
#include <idcu/common/deadlock_detector.h>

// 初始化死锁检测器
idcu_deadlock_detector_init();

// ... 使用锁 ...

// 检查死锁
if (idcu_deadlock_detector_check()) {
    printf("Deadlock detected!\n");
}

// 清理
idcu_deadlock_detector_destroy();
```

### 线程调试最佳实践

1. **避免嵌套锁**
```c
// 不好的做法
void bad_function() {
    idcu_mutex_lock(&lock1);
    idcu_mutex_lock(&lock2);  // 可能导致死锁
    // ...
    idcu_mutex_unlock(&lock2);
    idcu_mutex_unlock(&lock1);
}

// 好的做法 - 始终按相同顺序获取锁
void good_function() {
    idcu_mutex_lock(&lock1);
    idcu_mutex_lock(&lock2);
    // ...
    idcu_mutex_unlock(&lock2);
    idcu_mutex_unlock(&lock1);
}
```

2. **使用 RAII 风格的锁管理**
```c
// 确保锁总是被释放
#define LOCK_GUARD(lock) \
    for (int _locked = (idcu_mutex_lock(&lock), 1); _locked; _locked = 0, idcu_mutex_unlock(&lock))

LOCK_GUARD(my_lock) {
    // 临界区代码
}
```

---

## 日志调试

### 日志级别配置

在 `config/agent.cfg` 中设置：
```ini
[general]
log_level = debug    # debug, info, warn, error, fatal
log_file = agent.log
```

### 日志宏使用

```c
#include <idcu/log/log.h>

IDCU_LOG_DEBUG("Debug message: %d", value);
IDCU_LOG_INFO("Info message");
IDCU_LOG_WARN("Warning: %s", warning_text);
IDCU_LOG_ERROR("Error: %s", error_message);
IDCU_LOG_FATAL("Fatal error - exiting");
```

### 添加调试日志

在关键位置添加日志：
```c
static int my_module_init(void) {
    IDCU_LOG_DEBUG("my_module: Initializing...");
    
    int ret = some_function();
    IDCU_LOG_DEBUG("my_module: some_function returned %d", ret);
    
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("my_module: Initialization failed: %s", idcu_err_to_str(ret));
        return ret;
    }
    
    IDCU_LOG_INFO("my_module: Initialized successfully");
    return IDCU_ERR_OK;
}
```

---

## 常用调试工具

### Linux 工具

| 工具 | 用途 |
|------|------|
| gdb | 命令行调试器 |
| valgrind | 内存和性能分析 |
| strace | 系统调用追踪 |
| ltrace | 库调用追踪 |
| top/htop | 进程监控 |
| ps | 进程列表 |
| pmap | 内存映射 |

### Windows 工具

| 工具 | 用途 |
|------|------|
| Visual Studio Debugger | IDE 调试器 |
| Process Explorer | 进程监控 |
| Process Monitor | 系统活动监控 |
| WinDbg | 高级调试器 |

### 跨平台工具

| 工具 | 用途 |
|------|------|
| CMake | 构建系统 |
| clang-format | 代码格式化 |
| clang-tidy | 静态分析 |
| AddressSanitizer | 内存错误检测 |
| ThreadSanitizer | 线程错误检测 |

---

## 故障排除 checklist

当遇到问题时，按以下顺序检查：

1. [ ] 检查日志输出（设置 log_level = debug）
2. [ ] 验证配置文件是否正确
3. [ ] 确认所有依赖库已正确编译
4. [ ] 检查内存问题（使用 ASAN）
5. [ ] 检查线程问题（使用 TSAN）
6. [ ] 验证模块初始化顺序
7. [ ] 检查资源泄漏
8. [ ] 确认网络连接（如适用）

---

## 获取帮助

如果以上方法都无法解决问题：

1. 查看 [常见问题 FAQ](./faq.md)
2. 检查 [GitHub Issues](https://github.com/your-org/idcu-agent/issues)
3. 提交新的 Issue，包含：
   - 详细的错误描述
   - 复现步骤
   - 日志输出
   - 系统环境信息
   - 配置文件内容（敏感信息请脱敏）

---

**祝你调试顺利！** 🐛
