# 测试环境配置

本目录包含 IDCU Agent 的测试环境配置文件。

## 环境说明

测试环境配置适用于：
- 自动化测试和 CI/CD 流程
- 集成测试和端到端测试
- 预发布验证和回归测试

## 配置特点

- **适度日志**：`log_level = info`，平衡信息量和性能
- **配置验证**：`config_validate = true`，严格验证配置
- **完整功能**：启用所有模块和功能，模拟生产环境
- **资源适中**：适度的资源限制，接近生产环境
- **告警模拟**：启用测试告警通道，验证告警流程

## 使用方式

### 在 CI/CD 中使用

```yaml
# .github/workflows/ci.yml 示例
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build and test
        run: |
          mkdir -p build && cd build
          cmake ..
          make
          ./idcu_agent --config ../config/test/agent.cfg --test
```

### 自动化测试脚本

```bash
#!/bin/bash
# run-tests.sh

export IDCU_CONFIG_PATH=config/test/agent.cfg

# 运行单元测试
./build/tests/unit_tests

# 运行集成测试
./build/tests/integration_tests

# 启动 Agent 进行端到端测试
./build/idcu_agent --daemon
sleep 5
# 执行测试...
pkill idcu_agent
```

## 测试建议

1. **配置覆盖**：可以通过命令行参数或环境变量覆盖特定配置
2. **Mock 服务**：使用 Mock 服务替代外部依赖，确保测试稳定性
3. **数据隔离**：每个测试使用独立的配置和数据
4. **告警验证**：验证告警触发和发送流程（使用测试通道）
5. **性能基准**：建立性能基准，监控性能变化

## 相关配置

- [开发环境配置](../dev/README.md)
- [生产环境配置](../prod/README.md)
