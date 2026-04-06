# IDCU Agent 模块示例

这是一个完整的 IDCU Agent 模块示例，展示如何使用 idcu-module-build 框架开发 IDCU Agent 模块。

## 功能特性

- 完整的 IDCU Agent 模块接口实现
- 自动适配 IDCU Agent SDK
- 配置驱动的构建方式

## 目录结构

```
demo-idcu-agent/
├── include/
│   └── demo_idcu_agent/
│       └── demo_idcu_agent.h
├── src/
│   └── demo_idcu_agent.c
├── CMakeLists.txt
├── module.json
└── README.md
```

## 构建

```bash
cd ../../
python scripts/build.py release --module examples/demo-idcu-agent
```

## 说明

本示例展示了：
1. 如何定义 module.json 配置
2. 如何实现 IDCU 模块接口
3. 如何使用自动适配功能
