# IDCU Agent 核心库 API 文档

本目录包含 IDCU Agent 项目核心库的 API 文档。

## 库概览

### 核心基础设施库

- [idcu-common](common.md) - 通用工具和数据结构
- [idcu-log](log.md) - 日志系统
- [idcu-module-system](module-system.md) - 模块系统
- [idcu-coroutine](coroutine.md) - 协程调度器
- [idcu-msgbus](msgbus.md) - 消息总线
- [idcu-microkernel](microkernel.md) - 微内核

### 数据处理库

- [idcu-json](json.md) - JSON 解析器和序列化器
- [idcu-yaml](yaml.md) - YAML 解析器和序列化器
- [idcu-config](config.md) - 配置管理
- [idcu-storage](storage.md) - 键值存储

### 网络库

- [idcu-network](network.md) - 网络编程（TCP/UDP）
- [idcu-http-server](http-server.md) - HTTP 服务器框架
- [idcu-http-client](http-client.md) - HTTP 客户端框架

### 监控库

- [idcu-metrics](metrics.md) - 指标收集
- [idcu-healthcheck](healthcheck.md) - 健康检查

## 使用模式

### 错误处理

所有函数返回 `idcu_ErrorCode` 来指示成功或失败：

```c
int ret = some_function();
if (ret != IDCU_ERR_OK) {
    // 处理错误
    fprintf(stderr, "错误: %s\n", idcu_error_message(ret));
    return ret;
}
```

### 资源管理

大多数组件遵循初始化/销毁模式：

```c
SomeType* obj = NULL;
ret = some_type_init(&obj, config);
if (ret != IDCU_ERR_OK) {
    // 处理错误
    return ret;
}

// 使用对象
some_type_operation(obj, ...);

// 清理
some_type_destroy(obj);
```

## 许可证

本项目采用 MIT 许可证。
