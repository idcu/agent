# 任务 4.1: log-integration - 日志集成模块

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-08  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建 log-integration 集成模块，将 idcu-log 集成到微内核架构中，支持：
- 通过 SDK 使用日志功能
- 模块生命周期管理（init/start/stop/destroy）
- YAML 配置热重载
- 与消息总线集成
- 日志初始化时间 ≤ 30ms，支持并发 100 个模块的日志输出

### 1.2 不做什么
- 不修改 idcu-log 独立库的核心代码
- 不实现日志分析和可视化功能
- 不支持日志远程发送（留到后续阶段）

### 1.3 输入
- idcu-log 独立库（phase3 已完成）
- SDK 基础（phase2 已完成）
- YAML 配置文件：config/default/log_integration.yaml

### 1.4 输出
- log-integration 集成模块
- 模块可以正常初始化、启动、停止、销毁
- 日志输出到文件或 stdout

### 1.5 前置依赖
- phase3 01_idcu_log.md 任务已完成
- phase2 SDK 基础已完成
- 微内核初始化完成

---

## 2. 技术实现方案

### 2.1 核心选型
- 集成架构：基于 SDK 的封装层
- 配置格式：YAML（默认）
- 构建系统：idcu-module-build
- 日志库：idcu-log

### 2.2 核心逻辑
1. 创建 log-integration 目录结构
2. 实现集成模块代码，封装 idcu-log 接口
3. 实现模块生命周期函数（init/start/stop/destroy）
4. 从 YAML 配置读取日志级别和文件路径
5. 初始化 idcu-log
6. 提供 SDK 日志接口

### 2.3 数据结构/接口
```c
typedef struct {
    bool initialized;
    idcu_LogLevel level;
    char* log_file;
} LogIntegrationData;

static idcu_ErrorCode log_integration_init(idcu_SdkContext* ctx);
static idcu_ErrorCode log_integration_start(idcu_SdkContext* ctx);
static void log_integration_stop(idcu_SdkContext* ctx);
static void log_integration_destroy(idcu_SdkContext* ctx);
```

### 2.4 跨平台适配
- Windows：日志文件路径使用 `\` 分隔符
- Linux：日志文件路径使用 `/` 分隔符
- 动态库加载：Windows 用 LoadLibrary，Linux 用 dlopen

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 模块可以正常初始化
- [ ] 日志输出功能正常（stdout 和文件）
- [ ] YAML 配置可以正确加载
- [ ] 模块生命周期管理正常（init/start/stop/destroy）
- [ ] 支持日志级别配置（debug/info/warn/error）

### 3.2 性能验收
- [ ] 模块初始化时间 ≤ 30ms
- [ ] 支持 100 个模块并发写日志无锁竞争
- [ ] 单条日志输出耗时 ≤ 1ms
- [ ] 内存占用 ≤ 256KB

### 3.3 异常验收
- [ ] 配置文件不存在时使用默认值，模块正常启动
- [ ] 日志文件无法写入时降级到 stdout
- [ ] 模块初始化失败返回错误码，核心无崩溃
- [ ] 日志输出完整的错误码和原因

---

## 4. 执行计划

### 4.1 工期
0.5-1 天/人

### 4.2 里程碑
- D1：完成目录结构和集成代码
- D1：完成 CMakeLists.txt 和配置文件
- D1：完成测试和验证

### 4.3 人力
1 人（技能要求：C 语言 + 熟悉 idcu-log）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目的 .clang-format 规范
- 函数名小写+下划线，结构体前缀 Log_
- 所有头文件使用 include guard

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 测试 5 种异常场景（配置缺失、文件不可写等）
- 至少 3 个测试用例

### 5.3 部署指引
- 编译命令：`cmake --build build --target log_integration`
- 部署路径：`modules/integrations/log-integration/`
- 配置使用 YAML 格式

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：日志配置热重载时性能下降  
应对：使用配置缓存，减少文件 I/O

### 6.2 风险2
描述：多模块并发写日志导致锁竞争  
应对：使用无锁队列或读写锁优化

---

## 7. 详细实现步骤

### 7.1 创建目录结构

```bash
mkdir -p modules/integrations/log-integration/src
mkdir -p modules/integrations/log-integration/include
mkdir -p modules/integrations/log-integration/tests
```

### 7.2 实现集成模块

创建 `modules/integrations/log-integration/src/log_integration.c`：

```c
#include "sdk.h"
#include "idcu/log/log.h"
#include "idcu/config/config.h"
#include &lt;stdio.h&gt;
#include &lt;stdlib.h&gt;

typedef struct {
    bool initialized;
    idcu_LogLevel level;
    char* log_file;
} LogIntegrationData;

static idcu_ErrorCode log_integration_init(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Initializing log integration");

    LogIntegrationData* data = malloc(sizeof(LogIntegrationData));
    if (!data) {
        return IDCU_ERR_NO_MEMORY;
    }
    memset(data, 0, sizeof(LogIntegrationData));

    // 从配置读取
    const char* level_str = NULL;
    idcu_sdk_get_config_string(ctx, "log.level", &amp;level_str);
    data-&gt;level = IDCU_LOG_INFO;
    if (level_str) {
        if (strcmp(level_str, "debug") == 0) data-&gt;level = IDCU_LOG_DEBUG;
        else if (strcmp(level_str, "warn") == 0) data-&gt;level = IDCU_LOG_WARN;
        else if (strcmp(level_str, "error") == 0) data-&gt;level = IDCU_LOG_ERROR;
    }

    const char* log_file = NULL;
    idcu_sdk_get_config_string(ctx, "log.file", &amp;log_file);
    data-&gt;log_file = log_file ? strdup(log_file) : NULL;

    // 初始化日志
    idcu_ErrorCode err = idcu_log_init(data-&gt;log_file, data-&gt;level);
    if (err != IDCU_ERR_SUCCESS) {
        free(data-&gt;log_file);
        free(data);
        return err;
    }

    data-&gt;initialized = true;
    idcu_sdk_set_user_data(ctx, data);

    idcu_sdk_log_info(ctx, "Log integration initialized");
    return IDCU_ERR_SUCCESS;
}

static idcu_ErrorCode log_integration_start(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Starting log integration");
    return IDCU_ERR_SUCCESS;
}

static void log_integration_stop(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Stopping log integration");
}

static void log_integration_destroy(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Destroying log integration");

    LogIntegrationData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        if (data-&gt;initialized) {
            idcu_log_shutdown();
        }
        free(data-&gt;log_file);
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    log_integration,
    "1.0.0",
    "Log system integration module",
    log_integration_init,
    log_integration_start,
    log_integration_stop,
    log_integration_destroy
);
```

### 7.3 创建 CMakeLists.txt

```cmake
include(idcu_module)

idcu_add_library(log_integration
    VERSION 1.0.0
    SOURCES
        src/log_integration.c
    HEADERS
        # 暂无公开头文件
    DEPENDS
        idcu::sdk
        idcu::log
        idcu::config
)
```

### 7.4 YAML 配置示例（默认格式）

创建配置示例 `config/default/log_integration.yaml`：

```yaml
# 日志集成配置
log:
  # 日志级别: debug, info, warn, error, fatal
  level: "info"

  # 日志文件路径（可选，不填则输出到 stdout）
  file: ""

  # 文件轮转配置
  rotation:
    enabled: true
    max_size: 10485760  # 10MB
    max_files: 10

  # 格式配置
  format:
    include_timestamp: true
    include_level: true
    include_module: true
```

### 7.5 创建 README.md

```markdown
# log-integration

日志系统集成模块。

## 配置

使用 YAML 配置（默认）：

```yaml
log:
  level: "info"
  file: "/var/log/idcu/agent.log"
```

## API

通过 SDK 使用：

```c
idcu_sdk_log_info(ctx, "Hello, World!");
idcu_sdk_log_error(ctx, "Error occurred");
```
```

---

## 8. 验证检查清单

- [ ] 目录结构已创建
- [ ] 集成模块代码已创建
- [ ] CMakeLists.txt 已创建
- [ ] YAML 配置示例已创建
- [ ] README.md 已创建
- [ ] 模块可以正常编译
- [ ] 模块可以正常初始化
- [ ] 日志输出功能正常
- [ ] 配置可以正确加载
- [ ] 代码已格式化（clang-format）
- [ ] 静态分析通过（clang-tidy）
- [ ] 单元测试通过

---

## 9. Git 提交

```bash
git add modules/integrations/log-integration/
git add config/default/log_integration.yaml
git commit -m "feat(integrations): add log integration

- Add log integration module
- Add SDK logging wrappers
- Add YAML config example (default format)
- Add CMakeLists.txt using idcu-module-build
- Add README"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 模块初始化失败 | 配置文件路径错误 | 检查 YAML 配置文件路径 |
| 日志不输出 | 日志级别设置过高 | 降低日志级别到 debug |
| 日志文件无法写入 | 权限不足 | 检查目录权限 |
