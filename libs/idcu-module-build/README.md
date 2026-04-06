# IDCU Module Build

通用跨平台 C 项目模块构建框架 - 配置驱动 + 自动适配，让构建变得简单。

## 核心特性

- 🔧 配置驱动：通过 `module.json` 描述模块，无需复杂 CMake 代码
- 🤖 自动适配：自动识别项目类型，智能配置构建参数
- 🚀 开箱即用：支持跨平台（Windows/Linux/macOS）构建
- 📦 插件系统：可扩展的签名、打包、发布功能
- 🔗 依赖管理：自动处理本地和远程依赖

## 快速开始

### 环境要求

- CMake 3.10+
- C 编译器（GCC/Clang/MSVC）
- Python 3.6+（可选，用于构建脚本）

### 三步构建你的模块

1. **创建 module.json**

```json
{
  "module": {
    "name": "my-library",
    "version": "1.0.0",
    "type": "shared",
    "auto_adapt": true
  },
  "build": {
    "c_standard": "auto",
    "src": "auto",
    "includes": ["include"],
    "defines": [],
    "deps": []
  }
}
```

2. **创建 CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.10)
project(my-library)

find_package(idcu-module-build REQUIRED)
idcu_build_module()
```

3. **一键构建**

```bash
# 使用 Python 构建脚本（推荐）
python scripts/build.py release

# 或使用平台特定脚本
# Windows: scripts\build.bat release
# Linux/macOS: scripts/build.sh release
```

## 项目目录结构

```
idcu-module-build/
├── cmake/              # CMake 核心模块
│   ├── core/          # 通用构建核心
│   ├── adapters/      # 项目适配器
│   └── plugins/       # 可插拔扩展
├── config/            # 自动适配配置
│   ├── auto-adapter.yaml  # 自动识别规则
│   └── default.yaml       # 默认配置
├── examples/          # 示例项目
│   ├── demo-idcu-agent/  # IDCU Agent 模块示例
│   ├── demo-shared/       # 通用 C 动态库
│   └── demo-custom/       # 自定义项目
├── scripts/           # 构建脚本
├── templates/         # 项目模板
└── README.md
```

## 示例项目

我们提供了三种类型的示例项目，帮助你快速上手：

1. **IDCU Agent 模块示例** - `examples/demo-idcu-agent/`
   - 展示如何为 IDCU Agent 开发模块
   - 自动适配 IDCU Agent SDK
   - 完整的模块接口实现

2. **通用 C 动态库示例** - `examples/demo-shared/`
   - 独立的 C 共享库项目
   - 不依赖任何特定框架
   - 展示基础配置和使用方法

3. **自定义项目示例** - `examples/demo-custom/`
   - 高度自定义的构建配置
   - 展示高级特性的使用
   - 适合复杂项目需求

详细说明请参考各示例目录下的 README。

## 一键模板生成

使用模板生成脚本快速创建新项目：

```bash
# 生成 IDCU Agent 模块
python scripts/generate_template.py --type idcu-agent --name my-module

# 生成通用 C 库
python scripts/generate_template.py --type library --name my-library

# 生成自定义项目
python scripts/generate_template.py --type custom --name my-project
```

## 配置参考

### module.json 完整示例

```json
{
  "module": {
    "name": "example-module",
    "version": "1.0.0",
    "type": "shared",
    "description": "Example module description",
    "author": "Your Name",
    "license": "MIT",
    "auto_adapt": true
  },
  "build": {
    "c_standard": "11",
    "src": ["src/*.c", "src/**/*.c"],
    "type": "shared",
    "includes": ["include", "src"],
    "defines": ["EXAMPLE_DEFINE=1"],
    "deps": ["idcu-common"],
    "link_libraries": ["m"],
    "platform_deps": {
      "win32": ["ws2_32"],
      "unix": ["pthread"]
    }
  },
  "plugins": {
    "sign": false,
    "package": true,
    "coverage": false
  },
  "project": {
    "idcu_agent": {
      "sdk_path": "modules/core/sdk"
    },
    "generic": {}
  }
}
```

### 自动适配配置 (auto-adapter.yaml)

```yaml
adapters:
  - name: idcu_agent
    detect:
      - exists: "modules/core/sdk/"
      - exists: "agent.cfg"
    config:
      type: shared_library
      sdk_path: modules/core/sdk
      output_dir: agent-modules

  - name: generic_c_project
    detect:
      - exists: "CMakeLists.txt"
      - not_exists: "agent.cfg"
    config:
      type: auto
      output_dir: build/modules
```

## CI/CD 集成

### GitHub Actions

将以下内容添加到 `.github/workflows/build.yml`：

```yaml
name: Build Module

on: [push, pull_request]

jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]

    steps:
      - uses: actions/checkout@v4
      
      - name: Setup CMake
        uses: lukka/get-cmake@latest
      
      - name: Build
        run: |
          cd libs/idcu-module-build
          python scripts/build.py release
      
      - name: Upload artifacts
        uses: actions/upload-artifact@v4
        with:
          name: build-${{ matrix.os }}
          path: build/output/
```

## 贡献指南

欢迎提交 Issue 和 Pull Request！

## 许可证

MIT License
