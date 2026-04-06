# Examples Directory

这个目录包含 IDCU Agent 的完整使用示例，涵盖从基础模块开发到高级分布式功能的各个方面。

## 目录说明

### 基础示例

| 文件 | 描述 | 文档 |
|------|------|------|
| `simple_monitor_example.c` | 简单的监控服务使用示例 | - |
| `network_layer_example.c` | 网络层使用示例 | - |
| `sandbox_example.c` | 沙箱安全机制示例 | - |
| `productivity_example.c` | 生产力示例 | - |

### 新增进阶示例

| 文件 | 描述 | 文档 |
|------|------|------|
| `custom_collector_example.c` | 自定义数据采集模块（基础模块开发） | [custom_collector_example_README.md](./custom_collector_example_README.md) |
| `msgbus_communication_example.c` | 消息总线跨模块通信 | [msgbus_communication_example_README.md](./msgbus_communication_example_README.md) |
| `http_network_example.c` | HTTP客户端/服务端网络交互 | [http_network_example_README.md](./http_network_example_README.md) |
| `hotload_module_example.c` | 热加载模块 | [hotload_module_example_README.md](./hotload_module_example_README.md) |
| `distributed_node_example.c` | 分布式节点通信 | [distributed_node_example_README.md](./distributed_node_example_README.md) |

## 示例分层

### 第一层：基础入门
- `simple_monitor_example.c` - 了解如何使用基础服务
- `custom_collector_example.c` - 学习如何开发自定义模块

### 第二层：模块间通信
- `msgbus_communication_example.c` - 掌握消息总线的使用
- `sandbox_example.c` - 理解沙箱安全机制

### 第三层：网络编程
- `network_layer_example.c` - 底层网络操作
- `http_network_example.c` - HTTP服务端和客户端开发

### 第四层：高级功能
- `hotload_module_example.c` - 动态模块加载和热更新
- `distributed_node_example.c` - 分布式节点和集群通信

## 编译示例

### 使用 CMake 编译（推荐）

示例程序已集成到主项目的 CMake 构建系统中：

```bash
# 在项目根目录
mkdir -p build && cd build
cmake ..
make

# 编译好的示例会在 build/examples/ 目录下
```

### 单独编译示例

如果需要单独编译某个示例：

```bash
gcc -o example_name example_name.c \
    -I./modules/core/sdk/include \
    -I./modules/core/module-system/include \
    -I./libs/idcu-log/include \
    -I./libs/idcu-common/include \
    -L./build/libs -L./build/modules \
    -lidcu-log -lidcu-common [其他依赖库...]
```

## 运行步骤

### 1. 基础示例

直接运行编译好的可执行文件：

```bash
# Windows
build/examples/simple_monitor_example.exe

# Linux
./build/examples/simple_monitor_example
```

### 2. HTTP网络示例

需要先启动服务端，再运行客户端：

```bash
# 终端1：启动服务端
./build/examples/http_network_example server

# 终端2：运行客户端
./build/examples/http_network_example client
```

### 3. 分布式节点示例

在多个终端分别运行不同节点：

```bash
# 终端1：协调者节点
./build/examples/distributed_node_example coordinator 9000

# 终端2：工作节点1
./build/examples/distributed_node_example worker-1 9001

# 终端3：工作节点2
./build/examples/distributed_node_example worker-2 9002
```

### 4. 作为模块集成到 IDCU Agent

将示例代码作为业务模块集成到 Agent 中：

1. 将示例代码复制到 `modules/business/<module_name>/src/`
2. 创建对应的 `CMakeLists.txt`
3. 在根目录 `CMakeLists.txt` 中添加子目录
4. 在 `config/agent.cfg` 中启用模块
5. 重新编译并运行 Agent

详细步骤请参考各示例的 README 文档。

## 学习路径建议

### 新手入门

1. 阅读 [docs/tutorial_01_quick_start.md](../docs/tutorial_01_quick_start.md)
2. 运行 `simple_monitor_example.c` 了解基本功能
3. 学习 `custom_collector_example.c` 开发第一个模块

### 进阶学习

1. 研究 `msgbus_communication_example.c` 实现模块间通信
2. 掌握 `http_network_example.c` 进行网络编程
3. 探索 `sandbox_example.c` 了解安全机制

### 高级主题

1. 使用 `hotload_module_example.c` 实现热更新
2. 通过 `distributed_node_example.c` 构建分布式系统
3. 查看 `modules/business/examples/` 下的完整业务模块示例

## 核心代码解读

每个示例都包含详细的代码注释和独立的 README 文档，涵盖：

- 功能说明和主要特性
- 编译方式（独立/集成）
- 运行步骤和配置
- 核心代码逐段解读
- 扩展建议和最佳实践
- 依赖关系说明

## 依赖关系

所有示例都可能依赖以下库：

- `idcu-core-sdk` - 模块开发SDK
- `idcu-module-system` - 模块系统核心
- `idcu-log` - 日志系统
- `idcu-common` - 通用工具和错误码
- `idcu-msgbus` - 消息总线（通信示例）
- `idcu-http-server` / `idcu-http-client` - HTTP库（网络示例）
- `idcu-json` - JSON处理（网络示例）
- `idcu-distributed` / `idcu-discovery` - 分布式库（分布式示例）

## 更多资源

- [新手教程](../docs/tutorial_01_quick_start.md) - 从零开始入门
- [模块开发教程](../docs/tutorial_02_module_development.md) - 深入模块开发
- [架构文档](../docs/architecture.md) - 了解系统架构
- [开发者指南](../docs/developer_guide.md) - 高级开发指南
- [业务模块示例](../modules/business/examples/) - 真实业务模块参考

## 贡献示例

欢迎贡献新的示例！请遵循以下步骤：

1. 在本目录创建新的示例代码
2. 添加详细的中文注释
3. 编写对应的 README 文档
4. 在本文件的表格中添加条目
5. 提交 Pull Request

## 使用说明

每个示例都演示了如何使用 IDCU Agent 的不同服务模块功能。建议按学习路径逐步探索，从简单到复杂，逐步掌握 IDCU Agent 的各项功能。
