# IDCU Agent

一个基于微内核架构的实时代理程序，使用纯 C 语言编写。

## 这是什么？

IDCU Agent 是一个可以在后台运行的程序，它像一个"小管家"一样，可以通过加载不同的功能模块来完成各种任务。

## 主要特点

- 🚀 **微内核架构** - 核心很小，功能通过模块添加
- ⚡ **实时性好** - 响应速度快
- 🔌 **模块化设计** - 可以自由添加新功能
- 💻 **跨平台** - Windows 和 Linux 都能用
- 📦 **轻量级** - 不需要安装复杂的依赖
- 🛡️ **沙箱机制** - 模块安全隔离
- 🔄 **协程调度** - 高效的任务调度
- 📨 **消息总线** - 模块间通信
- 🔧 **动态模块加载** - 运行时加载 DLL/SO 模块
- 📊 **Prometheus 指标导出** - 支持 Prometheus 监控
- 🌐 **分布式节点支持** - 多节点通信
- ⚠️ **告警管理** - 健康检查与告警通知

## 项目结构

```
idcu-agent/
├── app/                          # 应用程序入口
│   ├── main.c                    # 主程序入口
│   └── benchmark.c               # 性能基准测试
├── modules/                      # 模块化结构
│   ├── services/                 # 服务模块组
│   │   ├── monitor/              # 监控系统
│   │   │   ├── include/          # 头文件 (健康检查、指标、告警、通知、Prometheus导出)
│   │   │   ├── src/              # 源代码
│   │   │   ├── tests/            # 测试
│   │   │   ├── CMakeLists.txt
│   │   │   ├── README.md
│   │   │   └── module.json
│   │   ├── network/              # 网络相关
│   │   │   ├── include/          # 头文件 (网络层、连接池、HTTP服务器、分布式节点、节点发现、管理API)
│   │   │   ├── src/              # 源代码
│   │   │   ├── tests/            # 测试
│   │   │   ├── CMakeLists.txt
│   │   │   ├── README.md
│   │   │   └── module.json
│   │   ├── security/             # 安全相关
│   │   │   ├── include/          # 头文件 (沙箱基础版、增强版)
│   │   │   ├── src/              # 源代码
│   │   │   ├── tests/            # 测试
│   │   │   ├── CMakeLists.txt
│   │   │   ├── README.md
│   │   │   └── module.json
│   │   └── plugin/               # 插件系统
│   │       ├── include/          # 头文件
│   │       ├── src/              # 源代码
│   │       ├── tests/            # 测试
│   │       ├── CMakeLists.txt
│   │       ├── README.md
│   │       └── module.json
│   └── business/                 # 业务模块组
│       ├── core-module/          # 核心基础模块
│       ├── alert/                # 告警模块
│       ├── collect/              # 采集模块
│       ├── config/               # 配置模块
│       ├── data-collector/       # 数据采集模块
│       ├── healthcheck/          # 健康检查模块
│       ├── heartbeat/            # 心跳模块
│       ├── http-management/      # HTTP管理模块
│       ├── log/                  # 日志模块
│       ├── metrics/              # 指标模块
│       ├── examples/             # 示例模块
│       │   ├── simple-example/   # 简单示例
│       │   ├── messaging-example/ # 消息示例
│       │   └── advanced-example/ # 高级示例
│       └── dynamic/              # 动态加载模块示例
│           ├── sample/           # 示例模块
│           ├── test-good/        # 测试模块(正常)
│           ├── test-init-fail/   # 测试模块(初始化失败)
│           └── test-no-interface/ # 测试模块(无接口)
├── module-repo/                  # 模块仓库
│   ├── official/                 # 官方模块
│   │   ├── monitoring/
│   │   │   ├── health_check_module.c
│   │   │   ├── metrics_module.c
│   │   │   └── prometheus_exporter_module.c
│   │   ├── networking/
│   │   │   ├── distributed_node_module.c
│   │   │   ├── http_management_module.c
│   │   │   ├── network_layer_module.c
│   │   │   └── node_discovery_module.c
│   │   └── security/
│   │       ├── sandbox_module.c
│   │       └── sandbox_enhanced_module.c
│   ├── templates/                # 模块模板
│   │   ├── README.md
│   │   └── module_template.c
│   ├── CMakeLists.txt
│   └── index.json
├── config/                       # 配置文件
│   └── agent.cfg
├── tests/                        # 测试用例
│   ├── unit/                     # 单元测试
│   └── integration/              # 集成测试
├── examples/                     # 示例程序
│   ├── dynamic_module_example.c
│   ├── distributed_app.c
│   └── prometheus_exporter_example.c
├── docs/                         # 文档目录
│   ├── architecture.md
│   ├── community_guide.md
│   ├── issue_pr_workflow.md
│   ├── management_api.md
│   ├── modular_architecture_guide.md
│   ├── module_loader_design.md
│   ├── roadmap.md
│   ├── sdk_guide.md
│   ├── tutorial_01_quick_start.md
│   ├── tutorial_02_module_development.md
│   ├── tutorial_03_message_bus.md
│   ├── tutorial_04_coroutine.md
│   └── tutorial_05_sandbox.md
├── scripts/                      # 构建和辅助脚本
│   ├── build.bat                 # Windows 编译脚本
│   ├── build.sh                  # Linux 编译脚本
│   └── generate_coverage.sh      # 覆盖率生成脚本
├── templates/                    # 模块模板
│   └── module/
├── CMakeLists.txt                # CMake 构建脚本
├── CMakeLists_benchmark.txt      # 基准测试 CMake 配置
├── Doxyfile                      # Doxygen 文档配置
├── .clang-format                 # 代码格式化配置
├── .clang-tidy                   # 代码静态分析配置
├── .editorconfig                 # 编辑器配置
├── .gitignore                    # Git 忽略文件
├── README.md                     # 项目说明文档
├── CONTRIBUTING.md               # 贡献指南
├── CODE_OF_CONDUCT.md            # 行为准则
└── .github/                      # GitHub 相关配置
    ├── workflows/                # CI/CD 工作流
    ├── ISSUE_TEMPLATE/           # Issue 模板
    └── PULL_REQUEST_TEMPLATE.md  # PR 模板
```

## 前缀命名空间说明

为了避免命名冲突，本项目所有公共符号都使用 `idcu_` 前缀：

- **类型**: `idcu_TypeName` (例如: `idcu_Mutex`, `idcu_ErrorCode`)
- **函数**: `idcu_function_name()` (例如: `idcu_mutex_init()`)
- **宏**: `IDCU_MACRO_NAME` (例如: `IDCU_ERR_OK`, `IDCU_CONFIG_MAX_MODULES`)
- **枚举值**: `IDCU_ENUM_VALUE` (例如: `IDCU_MOD_STATE_INITED`)

同时保留了兼容性宏，旧代码无需修改即可继续编译。

## 快速开始

### Windows 用户

如果你用的是 Windows，最简单的方法是直接双击 `scripts\build.bat` 来编译，或者在命令行中运行：

```
scripts\build.bat
```

### Linux 用户

如果你用的是 Linux，运行 `scripts/build.sh` 来编译：

```
chmod +x scripts/build.sh
./scripts/build.sh
```

## 怎么用？

详细的使用说明请看 [用户指引](./docs/user_guide.md)

## 想自己开发？

如果你想修改代码或添加新功能，请看 [开发者指引](./docs/developer_guide.md)

## 社区

我们欢迎你加入 IDCU Agent 社区！

### 沟通渠道

- 💬 **Discord/Slack**: (待建立) - 实时聊天、讨论问题、分享想法
- 📋 **GitHub Issues**: 报告 Bug、提出功能建议
- 📝 **GitHub Discussions**: 更深入的技术讨论
- 🔄 **Pull Requests**: 参与代码贡献

### 如何参与

1. **提问和讨论**: 在社区中提问、分享你的想法
2. **报告 Bug**: 发现问题时及时报告
3. **贡献代码**: 提交 PR 修复 Bug 或添加新功能
4. **改进文档**: 帮助完善文档和教程
5. **帮助他人**: 回答其他社区成员的问题

详细的参与指南请查看 [社区指南](./docs/community_guide.md) 和 [贡献指南](./CONTRIBUTING.md)。

## 学习资源

- 📚 **教程系列**:
  - [教程一：快速入门](./docs/tutorial_01_quick_start.md)
  - [教程二：模块开发入门](./docs/tutorial_02_module_development.md)
  - [教程三：消息总线使用](./docs/tutorial_03_message_bus.md)
  - [教程四：协程调度详解](./docs/tutorial_04_coroutine.md)
  - [教程五：沙箱安全机制](./docs/tutorial_05_sandbox.md)

- 📖 **其他文档**:
  - [用户指引](./docs/user_guide.md) - 如何使用 IDCU Agent
  - [开发者指引](./docs/developer_guide.md) - 如何开发模块
  - [架构设计](./docs/architecture.md) - 深入了解系统架构

## 常见问题

**Q: 编译失败怎么办？**

A: 先确保你安装了编译器（Windows 需要 MinGW，Linux 需要 gcc）。

**Q: 程序怎么停止？**

A: 按 `Ctrl + C` 就可以停止程序。

**Q: 我可以添加自己的功能吗？**

A: 当然可以！看 [开发者指引](./docs/developer_guide.md) 了解怎么做。

**Q: 如何获得帮助？**

A: 你可以：
1. 查看文档和教程
2. 在 GitHub Issues 中提问
3. 加入我们的社区聊天（待建立）

## 许可证

本项目仅供学习和研究使用。
