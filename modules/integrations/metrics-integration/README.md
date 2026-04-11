# metrics-integration

指标系统集成模块。

## 配置

使用 YAML 配置（默认）：

```yaml
metrics:
  enabled: true
  collection_interval: 1000
  prometheus:
    enabled: true
    port: 9090
```

## 功能

- 统一的指标接口
- 指标收集、导出、聚合
- 指标告警和持久化
- Prometheus 格式支持
