# 任务 5.7: alert-module（告警业务模块）

## 目标

创建告警业务模块，提供告警管理功能，包括：
- 告警规则管理
- 告警通知
- 告警历史记录
- 告警级别管理
- 告警屏蔽

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/business/alert-module/src
mkdir -p modules/business/alert-module/include
```

### 2. 创建模块代码

创建 `modules/business/alert-module/src/alert_module.c`：

```c
#include "sdk.h"
#include "idcu/alert/alert.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    idcu_Vector alert_rules;
    idcu_Vector alert_history;
    bool enabled;
} AlertModuleData;

static int alert_init(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Initializing alert module");
    
    AlertModuleData* data = malloc(sizeof(AlertModuleData));
    if (!data) {
        return IDCU_ERR_NO_MEMORY;
    }
    
    idcu_vector_init(&data->alert_rules, sizeof(void*), 16);
    idcu_vector_init(&data->alert_history, sizeof(void*), 100);
    data->enabled = true;
    
    idcu_sdk_set_user_data(ctx, data);
    
    return IDCU_ERR_SUCCESS;
}

static int alert_start(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Starting alert module");
    return IDCU_ERR_SUCCESS;
}

static int alert_run(idcu_SdkContext* ctx) {
    AlertModuleData* data = idcu_sdk_get_user_data(ctx);
    if (!data || !data->enabled) {
        return IDCU_ERR_SUCCESS;
    }
    
    // Check alert rules and trigger alerts
    return IDCU_ERR_SUCCESS;
}

static int alert_stop(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Stopping alert module");
    return IDCU_ERR_SUCCESS;
}

static void alert_destroy(idcu_SdkContext* ctx) {
    AlertModuleData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        idcu_vector_destroy(&data->alert_rules);
        idcu_vector_destroy(&data->alert_history);
        free(data);
    }
    idcu_sdk_log_info(ctx, "Destroying alert module");
}

IDCU_SDK_MODULE_DEFINE(
    alert_module,
    "1.0.0",
    "Alert management module",
    alert_init,
    alert_start,
    alert_run,
    alert_stop,
    alert_destroy
);
```

### 3. 创建 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-business-alert C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-business-alert STATIC
    src/alert_module.c
)

target_include_directories(idcu-business-alert PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/modules/core/sdk/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-business-alert PRIVATE
    idcu::common
    idcu::log
    idcu::alert
    idcu::sdk
)
```

### 4. 创建模块配置文件 (module.yaml)

```yaml
name: alert-module
version: 1.0.0
description: Alert management business module
author: IDCU Team
license: MIT

dependencies:
  - idcu-sdk
  - idcu-alert

build:
  type: cmake
  targets:
    - idcu-business-alert

features:
  - alert_rules: Alert rule management
  - alert_notification: Alert notification
  - alert_history: Alert history recording
  - alert_level: Alert level management
  - alert_silencing: Alert silencing

testing:
  enabled: true
  framework: internal
```

## 验证检查清单

- [ ] 模块代码已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建（YAML 默认格式）
- [ ] README.md 已创建
- [ ] 代码可以成功编译
- [ ] 基本告警功能正常
- [ ] 安全检查清单已通过

## Git 提交

```bash
git add modules/business/alert-module/
git commit -m "feat: add alert business module

- Add alert rule management
- Add alert notification
- Add alert history
- Add CMake build configuration
- Add module.yaml metadata with YAML format"
```
