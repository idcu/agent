# 任务：创建 SDK 基础

> **阶段**: 2 - 核心基础设施构建
> **优先级**: ⭐⭐⭐⭐⭐
> **依赖**: idcu-common, 微内核, idcu-module-build
> **预计工作量**: 1-2 天

---

## 🎯 目标

创建软件开发工具包（SDK），简化模块开发，支持：
- 模块生命周期管理封装
- 日志接口封装
- 配置接口封装
- 消息发送/接收接口封装
- 模块定义宏

---

## 📝 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/core/sdk/include
mkdir -p modules/core/sdk/src
mkdir -p modules/core/sdk/tests
mkdir -p modules/core/sdk/examples
```

### 2. 先写测试（TDD）

创建测试文件 `modules/core/sdk/tests/test_sdk.c`：

```c
#include "idcu/testframework/testframework.h"
#include "sdk.h"
#include "idcu/common/error_code.h"
#include <stdio.h>
#include <string.h>

static int g_init_count = 0;
static int g_start_count = 0;
static char g_last_log[256] = {0};

static idcu_ErrorCode test_module_init(idcu_SdkContext* ctx) {
    (void)ctx;
    g_init_count++;
    return IDCU_ERR_SUCCESS;
}

static idcu_ErrorCode test_module_start(idcu_SdkContext* ctx) {
    (void)ctx;
    g_start_count++;
    idcu_sdk_log_info(ctx, "Module started");
    return IDCU_ERR_SUCCESS;
}

static void test_module_stop(idcu_SdkContext* ctx) {
    (void)ctx;
}

static void test_module_destroy(idcu_SdkContext* ctx) {
    (void)ctx;
}

IDCU_SDK_MODULE_DEFINE(
    test_module,
    "1.0.0",
    "Test module for SDK",
    test_module_init,
    test_module_start,
    test_module_stop,
    test_module_destroy
);

IDCU_TEST_CASE(Sdk, CreateContext) {
    idcu_SdkContext* ctx = idcu_sdk_context_create();
    IDCU_TEST_ASSERT(ctx != NULL);
    idcu_sdk_context_destroy(ctx);
}

IDCU_TEST_CASE(Sdk, ModuleLifecycle) {
    idcu_SdkContext* ctx = idcu_sdk_context_create();
    IDCU_TEST_ASSERT(ctx != NULL);

    g_init_count = 0;
    g_start_count = 0;
    memset(g_last_log, 0, sizeof(g_last_log));

    idcu_ErrorCode err = idcu_sdk_init_module(ctx, &test_module_def);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_SUCCESS, err);
    IDCU_TEST_ASSERT_EQUAL(1, g_init_count);

    err = idcu_sdk_start_module(ctx, &test_module_def);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_SUCCESS, err);
    IDCU_TEST_ASSERT_EQUAL(1, g_start_count);

    idcu_sdk_stop_module(ctx, &test_module_def);
    idcu_sdk_destroy_module(ctx, &test_module_def);
    idcu_sdk_context_destroy(ctx);
}

IDCU_TEST_CASE(Sdk, Logging) {
    idcu_SdkContext* ctx = idcu_sdk_context_create();
    IDCU_TEST_ASSERT(ctx != NULL);

    idcu_sdk_log_debug(ctx, "Debug message");
    idcu_sdk_log_info(ctx, "Info message");
    idcu_sdk_log_warn(ctx, "Warn message");
    idcu_sdk_log_error(ctx, "Error message");

    idcu_sdk_context_destroy(ctx);
}

int main(void) {
    return idcu_test_run_all();
}
```

### 3. 实现头文件

创建 `modules/core/sdk/include/sdk.h`：

```c
#ifndef IDCU_SDK_H
#define IDCU_SDK_H

#include "idcu/common/error_code.h"
#include "micro_kernel.h"
#include <stdarg.h>
#include <stddef.h>

typedef struct idcu_SdkContext idcu_SdkContext;

// SDK 上下文创建/销毁
idcu_SdkContext* idcu_sdk_context_create(void);
void idcu_sdk_context_destroy(idcu_SdkContext* ctx);

// 获取微内核
idcu_MicroKernel* idcu_sdk_get_kernel(idcu_SdkContext* ctx);

// 用户数据
void* idcu_sdk_get_user_data(idcu_SdkContext* ctx);
void idcu_sdk_set_user_data(idcu_SdkContext* ctx, void* data);

// 日志接口
void idcu_sdk_log_debug(idcu_SdkContext* ctx, const char* fmt, ...);
void idcu_sdk_log_info(idcu_SdkContext* ctx, const char* fmt, ...);
void idcu_sdk_log_warn(idcu_SdkContext* ctx, const char* fmt, ...);
void idcu_sdk_log_error(idcu_SdkContext* ctx, const char* fmt, ...);

// 配置接口
idcu_ErrorCode idcu_sdk_get_config_string(
    idcu_SdkContext* ctx,
    const char* key,
    const char** out_value
);
idcu_ErrorCode idcu_sdk_get_config_int(
    idcu_SdkContext* ctx,
    const char* key,
    int* out_value
);
idcu_ErrorCode idcu_sdk_get_config_bool(
    idcu_SdkContext* ctx,
    const char* key,
    bool* out_value
);

// 消息接口
idcu_ErrorCode idcu_sdk_send_message(
    idcu_SdkContext* ctx,
    const char* topic,
    void* data,
    size_t data_size
);
idcu_ErrorCode idcu_sdk_subscribe_message(
    idcu_SdkContext* ctx,
    const char* topic,
    idcu_MessageHandler handler,
    void* user_data
);

// 模块生命周期（内部使用）
idcu_ErrorCode idcu_sdk_init_module(idcu_SdkContext* ctx, const idcu_ModuleDef* def);
idcu_ErrorCode idcu_sdk_start_module(idcu_SdkContext* ctx, const idcu_ModuleDef* def);
void idcu_sdk_stop_module(idcu_SdkContext* ctx, const idcu_ModuleDef* def);
void idcu_sdk_destroy_module(idcu_SdkContext* ctx, const idcu_ModuleDef* def);

// 模块定义宏
#define IDCU_SDK_MODULE_DEFINE(name, ver, desc, init_fn, start_fn, stop_fn, destroy_fn) \
    static idcu_ErrorCode _sdk_##name##_init(idcu_Module* module) { \
        idcu_SdkContext* ctx = idcu_module_get_sdk_context(module); \
        return init_fn(ctx); \
    } \
    static idcu_ErrorCode _sdk_##name##_start(idcu_Module* module) { \
        idcu_SdkContext* ctx = idcu_module_get_sdk_context(module); \
        return start_fn(ctx); \
    } \
    static void _sdk_##name##_stop(idcu_Module* module) { \
        idcu_SdkContext* ctx = idcu_module_get_sdk_context(module); \
        stop_fn(ctx); \
    } \
    static void _sdk_##name##_destroy(idcu_Module* module) { \
        idcu_SdkContext* ctx = idcu_module_get_sdk_context(module); \
        destroy_fn(ctx); \
    } \
    const idcu_ModuleDef name##_def = { \
        .name = #name, \
        .version = ver, \
        .description = desc, \
        .init = _sdk_##name##_init, \
        .start = _sdk_##name##_start, \
        .stop = _sdk_##name##_stop, \
        .destroy = _sdk_##name##_destroy, \
        .dependencies = NULL, \
        .dependency_count = 0 \
    }

#endif
```

### 4. 实现源文件

创建 `modules/core/sdk/src/sdk.c`（参考现有代码实现）：

```c
#include "sdk.h"
#include "idcu/common/error_code.h"
#include "idcu/log/log.h"
#include "idcu/config/config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// 结构定义
struct idcu_SdkContext {
    idcu_MicroKernel* kernel;
    void* user_data;
    // ...
};

// 实现各个函数
// ...
```

### 5. 创建 CMakeLists.txt

```cmake
include(idcu_module)

idcu_add_library(idcu_sdk
    VERSION 1.0.0
    SOURCES
        src/sdk.c
    HEADERS
        include/sdk.h
    DEPENDS
        idcu::common
        idcu::log
        idcu::config
        idcu::micro_kernel
)

# 测试
if(BUILD_TESTS)
    add_executable(test_sdk
        tests/test_sdk.c
    )
    target_link_libraries(test_sdk PRIVATE
        idcu::sdk
        idcu::testframework
    )
    add_test(NAME test_sdk COMMAND test_sdk)
endif()

# 示例
if(BUILD_EXAMPLES)
    add_executable(example_sdk examples/example_sdk.c)
    target_link_libraries(example_sdk PRIVATE idcu::sdk)
endif()
```

### 6. 创建 README.md

```markdown
# idcu-sdk

IDCU Agent 软件开发工具包（SDK）。

## 快速开始

```c
#include "sdk.h"

static idcu_ErrorCode my_module_init(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Initializing my module");
    return IDCU_ERR_SUCCESS;
}

static idcu_ErrorCode my_module_start(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Starting my module");
    return IDCU_ERR_SUCCESS;
}

static void my_module_stop(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Stopping my module");
}

static void my_module_destroy(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Destroying my module");
}

IDCU_SDK_MODULE_DEFINE(
    my_module,
    "1.0.0",
    "My first module",
    my_module_init,
    my_module_start,
    my_module_stop,
    my_module_destroy
);
```

## API 参考

详见 [include/sdk.h](include/sdk.h)
```

### 7. YAML 配置示例（默认格式）

创建配置示例 `config/default/sdk.yaml`：

```yaml
# SDK 配置
sdk:
  # 日志配置
  logging:
    level: "info"
    format: "[%level%] %message%"

  # 配置加载
  config:
    auto_reload: true
    reload_interval_ms: 5000

  # 消息配置
  messaging:
    default_topic_prefix: "idcu"

  # 模块配置
  modules:
    # 可以在这里配置各个模块的参数
    my_module:
      enabled: true
      param1: "value1"
      param2: 42
```

---

## ✅ 验证检查清单

- [ ] 测试框架已通过（TDD）
- [ ] SDK 上下文创建/销毁功能正常
- [ ] 模块生命周期封装正常
- [ ] 日志接口功能正常
- [ ] 配置接口功能正常
- [ ] 消息接口功能正常
- [ ] 模块定义宏工作正常
- [ ] 代码已格式化（clang-format）
- [ ] 静态分析通过（clang-tidy）
- [ ] 所有单元测试通过
- [ ] YAML 配置示例已创建
- [ ] README.md 已创建
- [ ] 已使用 idcu-module-build

---

## 📦 Git 提交

```bash
git add modules/core/sdk/
git add config/default/sdk.yaml
git commit -m "feat(core): add SDK

- Add SDK header with module lifecycle macros
- Add SDK implementation
- Add unit tests (TDD)
- Add example code
- Add CMakeLists.txt using idcu-module-build
- Add YAML config example (default format)
- Add README"
```

---

## 🔍 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 模块定义不工作 | 宏使用错误 | 检查宏参数格式 |
| 日志不输出 | SDK 上下文未初始化 | 确保先创建 SDK 上下文 |
| 配置读取失败 | 配置文件不存在 | 检查配置文件路径 |

---

## 📖 参考资料

- 参考现有项目 `modules/core/sdk/` 的实现

---

**最后更新**: 2026-04-08
