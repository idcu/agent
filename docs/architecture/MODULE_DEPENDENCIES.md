# 模块依赖关系

本文档详细描述了 IDCU Agent 各模块之间的依赖关系。

## 核心模块

### idcu-common
- **依赖**: 无（基础库）
- **被依赖**: 所有其他模块
- **功能**: 提供基础数据结构、错误码、线程安全原语等

### idcu-log
- **依赖**: idcu-common
- **被依赖**: idcu-http-server, idcu-http-client, idcu-config, idcu-healthcheck 等
- **功能**: 日志记录系统

### idcu-memory
- **依赖**: idcu-common
- **被依赖**: 各模块可选使用
- **功能**: 内存池管理

### idcu-config
- **依赖**: idcu-common, idcu-yaml, idcu-json, idcu-log
- **被依赖**: 应用层模块
- **功能**: 配置管理

## 中间件模块

### idcu-network
- **依赖**: idcu-common
- **被依赖**: idcu-http-server, idcu-http-client, idcu-healthcheck
- **功能**: 网络套接字抽象

### idcu-msgbus
- **依赖**: idcu-common
- **被依赖**: idcu-microkernel
- **功能**: 消息总线

### idcu-module-system
- **依赖**: idcu-common
- **被依赖**: idcu-microkernel
- **功能**: 模块管理

### idcu-coroutine
- **依赖**: idcu-common
- **被依赖**: idcu-microkernel
- **功能**: 协程调度

## 微内核模块

### idcu-microkernel
- **依赖**: idcu-common, idcu-module-system, idcu-msgbus, idcu-coroutine, idcu-log
- **被依赖**: 应用层
- **功能**: 微内核核心

## 应用层模块

### idcu-http-server
- **依赖**: idcu-common, idcu-network, idcu-log
- **被依赖**: 应用程序
- **功能**: HTTP 服务器

### idcu-http-client
- **依赖**: idcu-common, idcu-network, idcu-log
- **被依赖**: 应用程序
- **功能**: HTTP 客户端

### idcu-metrics
- **依赖**: idcu-common
- **被依赖**: 应用程序
- **功能**: 指标收集

### idcu-healthcheck
- **依赖**: idcu-common, idcu-network, idcu-log
- **被依赖**: 应用程序
- **功能**: 健康检查

### idcu-storage
- **依赖**: idcu-common
- **被依赖**: 应用程序
- **功能**: 存储抽象

## 依赖层级

```
第1层 (基础):
  idcu-common

第2层 (核心库):
  idcu-log
  idcu-memory
  idcu-yaml
  idcu-json
  idcu-network

第3层 (中间件):
  idcu-config
  idcu-msgbus
  idcu-module-system
  idcu-coroutine

第4层 (微内核):
  idcu-microkernel

第5层 (应用服务):
  idcu-http-server
  idcu-http-client
  idcu-metrics
  idcu-healthcheck
  idcu-storage
```

## 循环依赖检查

当前架构设计中无循环依赖。所有依赖都是单向的，从高层到底层。
