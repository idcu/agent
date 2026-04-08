# 第四阶段：模块系统完善

## 阶段里程碑

完成本阶段后，你应该能够：
- ✅ 通过集成层使用所有基础库
- ✅ 使用 SDK 快速开发新模块
- ✅ 模块之间可以通过消息总线通信

## 每个集成模块的标准结构

第四阶段的每个集成模块都应该包含：
- 详细的集成代码
- 与微内核的对接
- SDK 接口封装
- 完整的测试

## 任务列表

| 序号 | 组件 | 状态 | 预计时间 | 依赖 | 功能 |
|-----|------|------|---------|------|------|
| 4.1 | [log-integration](./01_log_integration.md) | ⏳ 待开始 | 2小时 | 3.1 + SDK | 日志系统集成模块 |
| 4.2 | [config-integration](./02_config_integration.md) | ⏳ 待开始 | 2小时 | 3.6 + SDK | 配置系统集成模块 |
| 4.3 | [json-integration](./03_json_integration.md) | ⏳ 待开始 | 2小时 | 3.2 + SDK | JSON 解析集成模块 |
| 4.4 | [yaml-integration](./04_yaml_integration.md) | ⏳ 待开始 | 2小时 | 3.3 + SDK | YAML 解析集成模块 |
| 4.5 | [network-integration](./05_network_integration.md) | ⏳ 待开始 | 2小时 | 3.9 + SDK | 网络层集成模块 |
| 4.6 | [metrics-integration](./06_metrics_integration.md) | ⏳ 待开始 | 2小时 | 3.15 + SDK | 指标收集集成模块 |
| 4.7 | [basic-libs](./07_basic_libs.md) | ⏳ 待开始 | 3小时 | 多个基础库 + SDK | 基础库统一集成 |
| 4.8 | [SDK 完善](./08_sdk_complete.md) | ⏳ 待开始 | 4小时 | 微内核 | 完整的 SDK 接口封装 |

## 创建集成层示例

集成层的作用是将独立库与微内核架构连接起来。

创建 `modules/integrations/log-integration/src/log_integration.c`：

```c
#include "sdk.h"
#include "idcu/log/log.h"
#include <stdio.h>

static int log_integration_init(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Initializing log integration");
    return IDCU_ERR_SUCCESS;
}

static int log_integration_start(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Starting log integration");
    return IDCU_ERR_SUCCESS;
}

static int log_integration_stop(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Stopping log integration");
    return IDCU_ERR_SUCCESS;
}

static void log_integration_destroy(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Destroying log integration");
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

## 创建 SDK 示例

SDK（软件开发工具包）可以简化模块开发。

创建 `modules/core/sdk/include/sdk.h`：

```c
#ifndef IDCU_SDK_H
#define IDCU_SDK_H

#include "idcu/common/error_code.h"
#include "idcu/log/log.h"

typedef struct idcu_SdkContext idcu_SdkContext;

typedef int (*idcu_SdkInitFunc)(idcu_SdkContext* ctx);
typedef int (*idcu_SdkStartFunc)(idcu_SdkContext* ctx);
typedef int (*idcu_SdkStopFunc)(idcu_SdkContext* ctx);
typedef void (*idcu_SdkDestroyFunc)(idcu_SdkContext* ctx);

void idcu_sdk_log_info(idcu_SdkContext* ctx, const char* fmt, ...);
void idcu_sdk_log_error(idcu_SdkContext* ctx, const char* fmt, ...);
void* idcu_sdk_get_user_data(idcu_SdkContext* ctx);
void idcu_sdk_set_user_data(idcu_SdkContext* ctx, void* data);

#define IDCU_SDK_MODULE_DEFINE(name, ver, desc, init_fn, start_fn, stop_fn, destroy_fn) \
    /* 模块定义宏实现 */

#endif
```

## 阶段验收标准

- [ ] 所有独立库都有对应的集成模块
- [ ] SDK 可以正常使用
- [ ] 模块可以通过 SDK 便捷地开发
- [ ] 集成层与微内核架构无缝对接
