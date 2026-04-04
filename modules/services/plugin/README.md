# Plugin 服务模块

## 功能说明

插件系统模块，提供：
- **插件生态管理** - 插件的加载、卸载和生命周期管理
- **第三方插件集成** - 支持第三方插件的集成和管理

## 目录结构

```
plugin/
├── include/              # 头文件
│   ├── README.md
│   └── plugin_ecosystem.h
├── src/                  # 源代码
│   └── plugin_ecosystem.c
├── tests/                # 测试
│   ├── CMakeLists.txt
│   └── test_plugin_ecosystem.c
├── CMakeLists.txt
├── README.md
└── module.json
```

## 依赖关系

- 基础公共库
- 工具库
- 模块系统

## 使用说明

业务模块可以通过 include 目录下的头文件使用插件服务的功能。
