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

## 常见问题

**Q: 编译失败怎么办？**

A: 先确保你安装了编译器（Windows 需要 MinGW，Linux 需要 gcc）。

**Q: 程序怎么停止？**

A: 按 `Ctrl + C` 就可以停止程序。

**Q: 我可以添加自己的功能吗？**

A: 当然可以！看 [开发者指引.md](./开发者指引.md) 了解怎么做。

## 许可证

本项目仅供学习和研究使用。
