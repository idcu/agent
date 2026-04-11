# idcu-module-build

IDCU Agent 的模块构建系统。

## 功能特性

- **CMake 辅助函数**: 简化模块构建的 CMake 函数
- **自动发现**: 自动发现和构建模块
- **YAML 配置**: 基于 YAML 的模块元数据配置
- **跨平台**: 支持 Windows、Linux、macOS

## 快速开始

### 使用 CMake 辅助函数

```cmake
find_package(idcu-module-build REQUIRED)

idcu_add_module(my-module
    STATIC
    VERSION 1.0.0
    DESCRIPTION "My module"
    SOURCES src/my_module.c
    INCLUDE_DIRS include
    LINK_LIBRARIES idcu::common
)
```

### 使用 module.yaml

在模块根目录创建 `module.yaml`：

```yaml
name: my-module
version: 1.0.0
description: My module description
author: My Name
license: MIT

dependencies:
  - idcu-common

build:
  type: cmake
  targets:
    - my-module
```

## 构建脚本

使用 Python 构建脚本：

```bash
python3 scripts/build.py --root libs --build-type Release
```

## API 文档

详见 [CMakeLists.txt](CMakeLists.txt) 中的辅助函数。

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
