# 自定义项目示例

这是一个展示高级配置和自定义特性的示例项目。

## 功能特性

- 同时构建静态库和动态库
- 启用多个插件功能
- 平台特定依赖配置
- 自定义输出目录和命名
- 集成测试支持

## 目录结构

```
demo-custom/
├── include/
│   └── demo_custom/
│       └── demo_custom.h
├── src/
│   └── demo_custom.c
├── tests/
│   └── test_demo_custom.c
├── CMakeLists.txt
├── module.json
└── README.md
```

## 构建

```bash
cd ../../
python scripts/build.py release --module examples/demo-custom
```

## 运行测试

```bash
cd build
ctest --output-on-failure
```

## 说明

本示例展示了：
1. 如何配置高级构建选项
2. 如何启用插件功能
3. 如何配置平台特定依赖
4. 如何自定义输出配置
5. 如何集成单元测试
