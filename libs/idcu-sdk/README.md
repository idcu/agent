# idcu-sdk

IDCU Agent 模块开发的 SDK 库，提供简化的 API 用于模块生命周期管理、日志、配置和消息传递。

## 功能特性

- 简化的模块生命周期管理
- 日志接口包装器
- 配置接口包装器
- 消息传递接口包装器
- 模块定义宏

## 使用方法

```c
#include <idcu/sdk/sdk.h>

static int my_module_init(idcu_SdkContext* ctx, void* user_data) {
    idcu_sdk_log_info(ctx, "初始化我的模块");
    return IDCU_ERR_OK;
}

static int my_module_start(idcu_SdkContext* ctx, void* user_data) {
    idcu_sdk_log_info(ctx, "启动我的模块");
    return IDCU_ERR_OK;
}

static int my_module_stop(idcu_SdkContext* ctx, void* user_data) {
    idcu_sdk_log_info(ctx, "停止我的模块");
    return IDCU_ERR_OK;
}

static void my_module_destroy(idcu_SdkContext* ctx, void* user_data) {
    idcu_sdk_log_info(ctx, "销毁我的模块");
}

IDCU_SDK_MODULE_DEFINE(
    my_module,
    "1.0.0",
    "我的示例模块",
    my_module_init,
    my_module_start,
    my_module_stop,
    my_module_destroy,
    NULL
);

int main() {
    idcu_SdkContext* ctx = NULL;
    idcu_sdk_init(&ctx);
    idcu_sdk_register_module(ctx, IDCU_SDK_GET_MODULE(my_module));
    idcu_sdk_destroy(ctx);
    return 0;
}
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
