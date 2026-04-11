# idcu-management

IDCU Agent 的管理库。

## 功能特性
- 命令行界面 (CLI)
- HTTP 管理 API
- 模块管理接口
- 配置管理接口
- 系统监控接口
- 线程安全实现
- CMake 构建配置

## 使用方法

```c
#include <idcu/management/management.h>

idcu_Management* ctx;
int ret = idcu_mgmt_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // 执行管理命令
    idcu_MgmtResponse response;
    ret = idcu_mgmt_execute(ctx, IDCU_MGMT_CMD_STATUS, NULL, &response);
    if (ret == IDCU_ERR_OK) {
        // 处理响应
        idcu_mgmt_response_destroy(&response);
    }
    
    // 关闭
    idcu_mgmt_destroy(ctx);
}
```

## API 参考
- `idcu_mgmt_init()` - 初始化管理库
- `idcu_mgmt_destroy()` - 关闭管理库
- `idcu_mgmt_execute()` - 执行管理命令
- `idcu_mgmt_response_destroy()` - 销毁响应
- `idcu_mgmt_cli_start()` - 启动 CLI 界面
- `idcu_mgmt_cli_stop()` - 停止 CLI 界面
- `idcu_mgmt_http_start()` - 启动 HTTP API
- `idcu_mgmt_http_stop()` - 停止 HTTP API

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
