# 任务：创建 log-integration 集成模块

> **阶段**: 4 - 模块系统完善
> **优先级**: ⭐⭐⭐⭐
> **依赖**: SDK, idcu-log
> **预计工作量**: 0.5-1 天

---

## 🎯 目标

将 idcu-log 独立库集成到微内核架构中，支持：
- 通过 SDK 使用日志功能
- 日志模块生命周期管理
- 配置热重载
- 与消息总线集成

---

## 📝 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/integrations/log-integration/src
mkdir -p modules/integrations/log-integration/include
mkdir -p modules/integrations/log-integration/tests
```

### 2. 实现集成模块

创建 `modules/integrations/log-integration/src/log_integration.c`：

```c
#include "sdk.h"
#include "idcu/log/log.h"
#include "idcu/config/config.h"
#include <stdio.h>
#include <stdlib.h>

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
    idcu_sdk_get_config_string(ctx, "log.level", &level_str);
    data->level = IDCU_LOG_INFO;
    if (level_str) {
        if (strcmp(level_str, "debug") == 0) data->level = IDCU_LOG_DEBUG;
        else if (strcmp(level_str, "warn") == 0) data->level = IDCU_LOG_WARN;
        else if (strcmp(level_str, "error") == 0) data->level = IDCU_LOG_ERROR;
    }

    const char* log_file = NULL;
    idcu_sdk_get_config_string(ctx, "log.file", &log_file);
    data->log_file = log_file ? strdup(log_file) : NULL;

    // 初始化日志
    idcu_ErrorCode err = idcu_log_init(data->log_file, data->level);
    if (err != IDCU_ERR_SUCCESS) {
        free(data->log_file);
        free(data);
        return err;
    }

    data->initialized = true;
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
        if (data->initialized) {
            idcu_log_shutdown();
        }
        free(data->log_file);
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

### 3. 创建 CMakeLists.txt

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

### 4. YAML 配置示例（默认格式）

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

### 5. 创建 README.md

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

## ✅ 验证检查清单

- [ ] 模块可以正常初始化
- [ ] 日志输出功能正常
- [ ] 配置可以正确加载
- [ ] 模块生命周期管理正常
- [ ] 代码已格式化（clang-format）
- [ ] 静态分析通过（clang-tidy）
- [ ] YAML 配置示例已创建
- [ ] README.md 已创建
- [ ] 已使用 idcu-module-build

---

## 📦 Git 提交

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

**最后更新**: 2026-04-08
