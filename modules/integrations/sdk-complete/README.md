# sdk-complete

SDK 完善模块，提供完整的模块开发接口。

## 配置

使用 YAML 配置（默认）：

```yaml
sdk:
  lifecycle:
    enable_pause_resume: true
  coroutine:
    enabled: true
    max_coroutines: 1024
  service_registry:
    enabled: true
```

## 功能

- 完整的模块生命周期管理（init/start/stop/destroy/pause/resume）
- 完整的消息通信接口
- 完整的配置访问接口
- 完整的日志接口
- 高级功能接口（协程、服务注册等）
