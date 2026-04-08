# 任务：创建 metrics-module 业务模块

> **阶段**: 5 - 业务模块开发
> **优先级**: ⭐⭐⭐
> **依赖**: SDK, idcu-metrics
> **预计工作量**: 0.5-1 天

---

## 🎯 目标

创建指标收集业务模块，支持：
- 收集系统和模块指标
- 通过消息总线暴露指标
- HTTP API 查询指标
- 与健康检查集成

---

## 📝 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/business/metrics-module/src
mkdir -p modules/business/metrics-module/include
mkdir -p modules/business/metrics-module/tests
```

### 2. 实现业务模块

创建 `modules/business/metrics-module/src/metrics_module.c`：

```c
#include "sdk.h"
#include "idcu/metrics/metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    idcu_MetricsRegistry* registry;
    idcu_Counter* requests_total;
    idcu_Gauge* active_modules;
    idcu_Histogram* request_duration;
} MetricsModuleData;

static idcu_ErrorCode metrics_module_init(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Initializing metrics module");

    MetricsModuleData* data = malloc(sizeof(MetricsModuleData));
    if (!data) {
        return IDCU_ERR_NO_MEMORY;
    }
    memset(data, 0, sizeof(MetricsModuleData));

    // 创建指标注册表
    data->registry = idcu_metrics_registry_create();
    if (!data->registry) {
        free(data);
        return IDCU_ERR_INTERNAL;
    }

    // 创建标准指标
    data->requests_total = idcu_metrics_counter_create(
        data->registry,
        "requests_total",
        "Total number of requests"
    );
    data->active_modules = idcu_metrics_gauge_create(
        data->registry,
        "active_modules",
        "Number of active modules"
    );
    data->request_duration = idcu_metrics_histogram_create(
        data->registry,
        "request_duration_seconds",
        "Request duration in seconds"
    );

    idcu_sdk_set_user_data(ctx, data);
    idcu_sdk_log_info(ctx, "Metrics module initialized");
    return IDCU_ERR_SUCCESS;
}

static idcu_ErrorCode metrics_module_start(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Starting metrics module");

    MetricsModuleData* data = idcu_sdk_get_user_data(ctx);

    // 设置初始指标
    idcu_metrics_gauge_set(data->active_modules, 1);

    return IDCU_ERR_SUCCESS;
}

static void metrics_module_stop(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Stopping metrics module");
}

static void metrics_module_destroy(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Destroying metrics module");

    MetricsModuleData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        if (data->registry) {
            idcu_metrics_registry_destroy(data->registry);
        }
        free(data);
    }
}

// 指标查询处理
static void metrics_query_handler(const idcu_Message* msg, void* user_data) {
    idcu_SdkContext* ctx = (idcu_SdkContext*)user_data;
    MetricsModuleData* data = idcu_sdk_get_user_data(ctx);

    if (data && data->registry) {
        // 导出指标
        char* output = idcu_metrics_registry_export(data->registry);
        if (output) {
            idcu_sdk_log_info(ctx, "Metrics: %s", output);
            free(output);
        }
    }
}

IDCU_SDK_MODULE_DEFINE(
    metrics_module,
    "1.0.0",
    "Metrics collection module",
    metrics_module_init,
    metrics_module_start,
    metrics_module_stop,
    metrics_module_destroy
);
```

### 3. 创建 CMakeLists.txt

```cmake
include(idcu_module)

idcu_add_library(metrics_module
    VERSION 1.0.0
    SOURCES
        src/metrics_module.c
    HEADERS
        # 暂无公开头文件
    DEPENDS
        idcu::sdk
        idcu::metrics
)
```

### 4. YAML 配置示例（默认格式）

创建配置示例 `config/default/metrics_module.yaml`：

```yaml
# 指标模块配置
metrics:
  # 启用指标收集
  enabled: true

  # 收集间隔（毫秒）
  collection_interval_ms: 15000

  # 导出格式
  export:
    format: "prometheus"
    include_timestamp: true

  # HTTP 标准指标
  standard_metrics:
    process_cpu_seconds: true
    process_resident_memory_bytes: true
    process_virtual_memory_bytes: true
    go_goroutines: false  # C 版本不适用

  # 自定义指标
  custom_metrics:
    - name: "custom_counter"
      type: "counter"
      help: "Custom counter metric"
    - name: "custom_gauge"
      type: "gauge"
      help: "Custom gauge metric"
```

### 5. 创建 README.md

```markdown
# metrics-module

指标收集业务模块。

## 配置

使用 YAML 配置（默认）：

```yaml
metrics:
  enabled: true
  collection_interval_ms: 15000
```

## 使用

```c
// 递增计数器
idcu_metrics_counter_inc(data->requests_total);

// 设置 Gauge
idcu_metrics_gauge_set(data->active_modules, 5);
```
```

---

## ✅ 验证检查清单

- [ ] 模块可以正常初始化
- [ ] 指标收集功能正常
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
git add modules/business/metrics-module/
git add config/default/metrics_module.yaml
git commit -m "feat(business): add metrics module

- Add metrics business module
- Add standard metrics
- Add YAML config example (default format)
- Add CMakeLists.txt using idcu-module-build
- Add README"
```

---

**最后更新**: 2026-04-08
