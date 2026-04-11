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

## 许可证

本库采用 [Apache License 2.0](../../../LICENSE) 许可证。
