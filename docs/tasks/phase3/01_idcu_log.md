# 任务 3.1: idcu-log - 日志系统库

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-08  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的、高性能的日志系统库，支持多级别日志（DEBUG、INFO、WARN、ERROR、FATAL）、多种输出目标（控制台、文件）、线程安全、自动时间戳和文件行号记录，满足 QPS ≥ 10,000 条日志/秒的性能要求。

### 1.2 不做什么
- 不实现远程日志输出（后续可扩展）
- 不实现日志异步写入（本阶段同步即可）
- 不实现日志分析和查询功能

### 1.3 输入
- 配置参数：日志级别、输出目标、文件名
- 日志消息：格式化字符串 + 参数

### 1.4 输出
- 控制台输出：带颜色的格式化日志
- 文件输出：带时间戳的日志文件
- 返回码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 基础库已可用（提供锁、错误码等）
- phase2 已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **线程安全**: 使用互斥锁（idcu_mutex）
- **日志级别**: 5 级枚举（DEBUG &lt; INFO &lt; WARN &lt; ERROR &lt; FATAL）
- **输出目标**: 位掩码组合（CONSOLE | FILE）
- **时间格式**: ISO 8601 格式（YYYY-MM-DD HH:MM:SS）

### 2.2 核心逻辑
```
1. 初始化：配置日志级别、输出目标、打开文件（如需要）
2. 日志写入：
   a. 获取当前时间戳
   b. 格式化日志消息
   c. 加锁保护
   d. 输出到控制台和/或文件
   e. 解锁
3. 关闭：刷新缓冲区、关闭文件、销毁锁
```

### 2.3 数据结构/接口
```c
typedef enum {
    IDCU_LOG_DEBUG = 0,
    IDCU_LOG_INFO  = 1,
    IDCU_LOG_WARN  = 2,
    IDCU_LOG_ERROR = 3,
    IDCU_LOG_FATAL = 4
} idcu_LogLevel;

typedef enum {
    IDCU_LOG_OUTPUT_CONSOLE = 1 &lt;&lt; 0,
    IDCU_LOG_OUTPUT_FILE    = 1 &lt;&lt; 1
} idcu_LogOutput;

typedef struct {
    char           filename[256];
    idcu_LogLevel  level;
    idcu_LogOutput output;
} idcu_LogConfig;

// 核心 API
int idcu_log_init(const char* filename, idcu_LogLevel level);
int idcu_log_init_with_config(const idcu_LogConfig* config);
void idcu_log_shutdown(void);
void idcu_log_set_level(idcu_LogLevel level);
void idcu_log_printf(idcu_LogLevel level, const char* file, int line, const char* fmt, ...);

// 便捷宏
#define IDCU_LOG_DEBUG(fmt, ...) idcu_log_printf(IDCU_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define IDCU_LOG_INFO(fmt, ...)  idcu_log_printf(IDCU_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define IDCU_LOG_WARN(fmt, ...)  idcu_log_printf(IDCU_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define IDCU_LOG_ERROR(fmt, ...) idcu_log_printf(IDCU_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define IDCU_LOG_FATAL(fmt, ...) idcu_log_printf(IDCU_LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
```

### 2.4 跨平台适配
- **Windows**: 使用 `fopen`、`localtime_s`
- **Linux**: 使用 `fopen`、`localtime_r`
- **控制台颜色**: Windows 使用 `SetConsoleTextAttribute`，Linux 使用 ANSI 转义序列

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以输出 5 个级别的日志（DEBUG、INFO、WARN、ERROR、FATAL）
- [ ] 可以同时输出到控制台和文件
- [ ] 日志包含时间戳、文件名、行号信息
- [ ] 可以动态调整日志级别
- [ ] 多线程环境下日志输出不乱序

### 3.2 性能验收
- 单线程日志写入 QPS ≥ 10,000 条/秒
- 多线程（4 线程）日志写入 QPS ≥ 5,000 条/秒
- 单条日志内存占用 ≤ 4KB
- 初始化时间 ≤ 10ms

### 3.3 异常验收
- [ ] 初始化失败（文件无法打开）返回明确错误码
- [ ] 传入 NULL 参数时安全处理
- [ ] 日志文件过大时能继续写入（不限制文件大小）
- [ ] 程序退出前未调用 shutdown 不崩溃

---

## 4. 执行计划

### 4.1 工期
2 小时/人

### 4.2 里程碑
- D1-00: 完成头文件定义（30 分钟）
- D1-30: 完成核心实现（1 小时）
- D1-90: 完成单元测试（30 分钟）

### 4.3 人力
1 人（技能要求：C 语言 + 多线程编程）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，结构体前缀 idcu_
- 所有公共 API 有 Doxygen 风格注释

### 5.2 测试要求
- 单元测试覆盖率 ≥ 85%
- 测试用例覆盖：初始化、各级别日志、多线程、异常场景
- 性能测试用例验证 QPS 指标

### 5.3 部署指引
- 编译命令：`cmake -B build &amp;&amp; cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::log)`

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：多线程环境下锁竞争导致性能下降  
应对：使用细粒度锁或考虑后续添加异步写入模式

### 6.2 风险2
描述：日志文件过大导致磁盘空间不足  
应对：添加日志轮转功能（可选扩展），当前阶段至少记录警告

---

## 7. 详细实现步骤

### 步骤 1: 创建目录结构
```bash
mkdir -p libs/idcu-log/include/idcu/log
mkdir -p libs/idcu-log/src/idcu/log
mkdir -p libs/idcu-log/tests
mkdir -p libs/idcu-log/examples
```

### 步骤 2: 创建头文件 log.h
定义日志级别、输出目标、配置结构体、核心 API 和便捷宏。

### 步骤 3: 创建实现文件 log.c
实现初始化、日志写入、级别设置、关闭等功能，使用 idcu-common 中的互斥锁保证线程安全。

### 步骤 4: 创建 CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-log VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-log STATIC src/idcu/log/log.c)
target_include_directories(idcu-log PUBLIC
    $&lt;BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include&gt;
    $&lt;INSTALL_INTERFACE:include&gt;
)
target_link_libraries(idcu-log PRIVATE idcu::common)
add_library(idcu::log ALIAS idcu-log)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 步骤 5: 创建 module.json
```json
{
  "name": "idcu-log",
  "version": "1.0.0",
  "description": "Logging library for IDCU Agent",
  "author": "IDCU Team",
  "license": "Apache-2.0",
  "dependencies": ["idcu-common"]
}
```

### 步骤 6: 创建 README.md
参考 libs/idcu-log/README.md 现有内容。

---

## 8. 验证检查清单

- [ ] 头文件 log.h 已创建
- [ ] 实现文件 log.c 已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.json 已创建
- [ ] README.md 已创建
- [ ] 可以正常编译通过
- [ ] 单元测试通过率 100%
- [ ] 性能测试达标（QPS ≥ 10,000）
- [ ] 跨平台测试通过（Windows + Linux）
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-log/
git commit -m "feat: add idcu-log library

- Add multi-level logging (DEBUG, INFO, WARN, ERROR, FATAL)
- Add console and file output
- Add thread-safe logging with mutex
- Add timestamp, file, and line number information
- Add CMake build configuration
- Add unit tests with 85%+ coverage"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 日志文件未写入 | 文件路径权限不足 | 检查文件路径和权限，使用绝对路径 |
| 日志级别不生效 | 初始化后未设置正确级别 | 确保在初始化时或之后调用 idcu_log_set_level |
| 多线程日志乱序 | 未正确加锁 | 检查互斥锁的使用，确保 log_printf 中有锁保护 |
| 控制台颜色不显示 | 平台不支持 ANSI 颜色 | Windows 需要启用虚拟终端处理 |
| 性能不达标 | 锁竞争严重 | 考虑减少日志量或后续实现异步写入 |
