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

## 项目结构

```
idcu-agent/
├── main.c                    # 程序入口
├── include/                  # 头文件目录
│   ├── common/              # 通用工具
│   │   ├── atomic.h
│   │   ├── config.h
│   │   ├── error_code.h
│   │   └── lock.h
│   ├── kernel/              # 内核核心
│   │   └── micro_kernel.h
│   ├── module/              # 模块系统
│   │   ├── dynamic_module.h
│   │   ├── module_def.h
│   │   ├── module_registry.h
│   │   └── module_manager.h
│   ├── monitor/             # 监控系统
│   │   ├── health_check.h
│   │   └── metrics.h
│   ├── network/             # 网络相关
│   │   ├── distributed_node.h
│   │   ├── network_layer.h
│   │   └── node_discovery.h
│   ├── plugin/              # 插件系统
│   │   └── plugin_ecosystem.h
│   ├── scheduler/           # 调度相关
│   │   ├── context.h
│   │   ├── coroutine.h
│   │   └── msg_bus.h
│   ├── security/            # 安全相关
│   │   ├── sandbox.h
│   │   └── sandbox_enhanced.h
│   ├── test/                # 测试框架
│   │   └── test_framework.h
│   └── utils/               # 工具模块
│       ├── config_manager.h
│       ├── json_parser.h
│       ├── log.h
│       └── memory_pool.h
├── src/                     # 源代码实现
│   └── (与 include 对应)
├── modules/                 # 功能模块
│   ├── base/                # 基础模块
│   │   ├── core_module.c
│   │   └── log.c
│   └── biz/                 # 业务模块
│       └── collect.c
├── config/                  # 配置文件
│   └── agent.cfg
├── tests/                   # 测试用例
│   ├── unit/                # 单元测试
│   └── integration/         # 集成测试
├── docs/                    # 文档目录
│   ├── architecture.md      # 架构设计文档
│   └── module_loader_design.md
├── CMakeLists.txt           # CMake 构建脚本
├── build.bat                # Windows 编译脚本
├── build.sh                 # Linux 编译脚本
└── benchmark.c              # 性能基准测试
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

如果你用的是 Windows，最简单的方法是直接双击 `build.bat` 来编译。

### Linux 用户

如果你用的是 Linux，运行 `./build.sh` 来编译。

## 怎么用？

详细的使用说明请看 [用户指引.md](./用户指引.md)

## 想自己开发？

如果你想修改代码或添加新功能，请看 [开发者指引.md](./开发者指引.md)

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
  - [用户指引](./用户指引.md) - 如何使用 IDCU Agent
  - [开发者指引](./开发者指引.md) - 如何开发模块
  - [架构设计](./docs/architecture.md) - 深入了解系统架构

## 常见问题

**Q: 编译失败怎么办？**

A: 先确保你安装了编译器（Windows 需要 MinGW，Linux 需要 gcc）。

**Q: 程序怎么停止？**

A: 按 `Ctrl + C` 就可以停止程序。

**Q: 我可以添加自己的功能吗？**

A: 当然可以！看 [开发者指引.md](./开发者指引.md) 了解怎么做。

**Q: 如何获得帮助？**

A: 你可以：
1. 查看文档和教程
2. 在 GitHub Issues 中提问
3. 加入我们的社区聊天（待建立）

## 许可证

本项目仅供学习和研究使用。
