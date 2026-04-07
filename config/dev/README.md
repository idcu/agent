# 开发环境配置

本目录包含 IDCU Agent 的开发环境配置文件。

## 环境说明

开发环境配置适用于：
- 本地开发和调试
- 单元测试和集成测试
- 快速迭代和原型验证

## 配置特点

- **详细日志**：`log_level = debug`，输出所有级别的日志
- **配置热重载**：`config_hot_reload = true`，支持运行时重新加载配置
- **最小告警**：禁用邮件、Webhook 等告警通知，避免打扰
- **资源宽松**：较高的内存和 CPU 限制，便于调试
- **功能完整**：启用所有模块和功能

## 使用方式

### 方式 1：指定配置文件启动

```bash
# Windows
idcu_agent.exe --config config/dev/agent.cfg

# Linux
./idcu_agent --config config/dev/agent.cfg
```

### 方式 2：设置环境变量

```bash
# Windows
set IDCU_CONFIG_PATH=config/dev/agent.cfg
idcu_agent.exe

# Linux
export IDCU_CONFIG_PATH=config/dev/agent.cfg
./idcu_agent
```

### 方式 3：复制到默认位置

```bash
# Windows
copy config\dev\agent.cfg config\agent.cfg

# Linux
cp config/dev/agent.cfg config/agent.cfg
```

## 开发建议

1. **启用调试模式**：利用详细的 debug 日志进行问题排查
2. **配置热更新**：修改配置后无需重启，直接生效
3. **快速迭代**：可以频繁修改配置和代码进行测试
4. **本地服务**：使用本地 Mock 服务替代外部依赖
5. **性能分析**：可以启用性能分析工具进行调优

## 相关配置

- [测试环境配置](../test/README.md)
- [生产环境配置](../prod/README.md)
