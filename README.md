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
├── main.c              # 程序入口
├── include/            # 头文件
│   ├── atomic.h
│   ├── config.h
│   ├── config_manager.h
│   ├── distributed_node.h
│   ├── dynamic_module.h
│   ├── error_code.h
│   ├── health_check.h
│   ├── json_parser.h
│   ├── lock.h
│   ├── log.h
│   ├── memory_pool.h
│   ├── metrics.h
│   ├── module_def.h
│   ├── module_registry.h
│   ├── network_layer.h
│   ├── node_discovery.h
│   ├── plugin_ecosystem.h
│   ├── sandbox_enhanced.h
│   └── test_framework.h
├── kernel/             # 微内核核心代码
│   ├── atomic.c
│   ├── config_manager.c
│   ├── context.c
│   ├── coroutine.c
│   ├── distributed_node.c
│   ├── dynamic_loader.c
│   ├── health_check.c
│   ├── json_parser.c
│   ├── lock.c
│   ├── log.c
│   ├── memory_pool.c
│   ├── metrics.c
│   ├── micro_kernel.c
│   ├── module_registry.c
│   ├── msg_bus.c
│   ├── network_layer.c
│   ├── node_discovery.c
│   ├── plugin_ecosystem.c
│   ├── sandbox.c
│   ├── sandbox_enhanced.c
│   └── test_framework.c
├── modules/            # 功能模块
│   ├── base/           # 基础模块
│   │   └── log.c
│   └── biz/            # 业务模块
│       └── collect.c
├── config/             # 配置文件
│   └── agent.cfg
├── tests/              # 测试用例
│   ├── unit/           # 单元测试
│   └── integration/    # 集成测试
├── CMakeLists.txt      # CMake 构建脚本
├── build.bat           # Windows 编译脚本
├── build.sh            # Linux 编译脚本
└── benchmark.c         # 性能基准测试
```

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
