# basic-libs

基础库统一集成模块，提供所有 phase3 基础库的统一访问接口。

## 配置

使用 YAML 配置（默认）：

```yaml
basic_libs:
  enabled_libs:
    - log
    - config
    - json
    - yaml
    - network
    - metrics
```

## 功能

- 所有基础库的统一初始化和清理
- 统一的配置管理
- 统一的日志管理
- 集中式的库访问接口

## 许可证

本库采用 [Apache License 2.0](../../../LICENSE) 许可证。
