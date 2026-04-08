# 第五阶段：业务模块开发

## 阶段里程碑

完成本阶段后，你应该能够：
- ✅ 运行完整的业务功能
- ✅ 模块之间可以协同工作
- ✅ 系统可以稳定运行

## 每个业务模块的标准结构

第五阶段的每个业务模块都应该包含：
- 详细的业务逻辑
- 与其他模块的交互
- 消息总线通信
- 完整的集成测试

## 任务列表

| 序号 | 业务模块 | 状态 | 预计时间 | 依赖 | 功能 |
|-----|---------|------|---------|------|------|
| 5.1 | [core-module](./01_core_module.md) | ⏳ 待开始 | 4小时 | SDK + 基础集成 | 核心基础模块 |
| 5.2 | [log-module](./02_log_module.md) | ⏳ 待开始 | 3小时 | SDK + 3.1 | 日志业务模块 |
| 5.3 | [config-module](./03_config_module.md) | ⏳ 待开始 | 3小时 | SDK + 3.6 | 配置业务模块 |
| 5.4 | [heartbeat](./04_heartbeat.md) | ⏳ 待开始 | 2小时 | SDK | 心跳模块 |
| 5.5 | [metrics-module](./05_metrics_module.md) | ⏳ 待开始 | 3小时 | SDK + 3.15 | 指标业务模块 |
| 5.6 | [healthcheck-module](./06_healthcheck_module.md) | ⏳ 待开始 | 3小时 | SDK + 3.16 | 健康检查业务模块 |
| 5.7 | [alert-module](./07_alert_module.md) | ⏳ 待开始 | 3小时 | SDK + 3.17 | 告警业务模块 |
| 5.8 | [collect-module](./08_collect_module.md) | ⏳ 待开始 | 3小时 | SDK + 3.26 | 数据采集业务模块 |
| 5.9 | [cache-module](./09_cache_module.md) | ⏳ 待开始 | 2小时 | SDK + 3.8 | 缓存业务模块 |
| 5.10 | [storage-module](./10_storage_module.md) | ⏳ 待开始 | 2小时 | SDK + 3.7 | 存储业务模块 |
| 5.11 | [security-module](./11_security_module.md) | ⏳ 待开始 | 3小时 | SDK + 3.20 + 3.21 | 安全业务模块 |
| 5.12 | [http-client-module](./12_http_client_module.md) | ⏳ 待开始 | 3小时 | SDK + 3.12 | HTTP 客户端业务模块 |
| 5.13 | [http-management](./13_http_management.md) | ⏳ 待开始 | 4小时 | SDK + 3.23 | HTTP 管理业务模块 |

## 创建第一个业务模块示例

让我们创建一个简单的心跳模块。

### 创建目录结构

```bash
mkdir -p modules/business/heartbeat/src
```

### 编写模块代码

创建 `modules/business/heartbeat/src/heartbeat_module.c`：

```c
#include "sdk.h"
#include <stdio.h>

typedef struct {
    int counter;
} HeartbeatData;

static int heartbeat_init(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Initializing heartbeat module");
    
    HeartbeatData* data = malloc(sizeof(HeartbeatData));
    if (!data) {
        return IDCU_ERR_NO_MEMORY;
    }
    data->counter = 0;
    idcu_sdk_set_user_data(ctx, data);
    
    return IDCU_ERR_SUCCESS;
}

static int heartbeat_start(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Starting heartbeat module");
    return IDCU_ERR_SUCCESS;
}

static int heartbeat_run(idcu_SdkContext* ctx) {
    HeartbeatData* data = idcu_sdk_get_user_data(ctx);
    
    if (data->counter % 1000000 == 0) {
        idcu_sdk_log_info(ctx, "Heartbeat #%d", data->counter / 1000000);
    }
    data->counter++;
    
    return IDCU_ERR_SUCCESS;
}

static int heartbeat_stop(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Stopping heartbeat module");
    return IDCU_ERR_SUCCESS;
}

static void heartbeat_destroy(idcu_SdkContext* ctx) {
    HeartbeatData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        free(data);
    }
    idcu_sdk_log_info(ctx, "Destroying heartbeat module");
}

IDCU_SDK_MODULE_DEFINE(
    heartbeat,
    "1.0.0",
    "Heartbeat module",
    heartbeat_init,
    heartbeat_start,
    heartbeat_run,
    heartbeat_stop,
    heartbeat_destroy
);
```

### 创建 CMakeLists.txt

```cmake
add_library(idcu_business_heartbeat STATIC
    src/heartbeat_module.c
)

target_include_directories(idcu_business_heartbeat PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/modules/core/sdk/include
)

target_link_libraries(idcu_business_heartbeat PRIVATE
    idcu::common
    idcu::log
    idcu_core_sdk
)
```

## 阶段验收标准

- [ ] 核心业务模块都已实现
- [ ] 模块之间可以通过消息总线通信
- [ ] 系统可以正常运行
- [ ] 所有业务模块都有对应的测试
