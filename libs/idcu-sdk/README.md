# idcu-sdk

SDK library for IDCU Agent module development, providing simplified APIs for module lifecycle management, logging, configuration, and messaging.

## Features

- Simplified module lifecycle management
- Logging interface wrappers
- Configuration interface wrappers
- Messaging interface wrappers
- Module definition macros

## Usage

```c
#include <idcu/sdk/sdk.h>

static int my_module_init(idcu_SdkContext* ctx, void* user_data) {
    idcu_sdk_log_info(ctx, "Initializing my module");
    return IDCU_ERR_OK;
}

static int my_module_start(idcu_SdkContext* ctx, void* user_data) {
    idcu_sdk_log_info(ctx, "Starting my module");
    return IDCU_ERR_OK;
}

static int my_module_stop(idcu_SdkContext* ctx, void* user_data) {
    idcu_sdk_log_info(ctx, "Stopping my module");
    return IDCU_ERR_OK;
}

static void my_module_destroy(idcu_SdkContext* ctx, void* user_data) {
    idcu_sdk_log_info(ctx, "Destroying my module");
}

IDCU_SDK_MODULE_DEFINE(
    my_module,
    "1.0.0",
    "My sample module",
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

## License

MIT
