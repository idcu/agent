# IDCU Module Build

IDCU Agent 的通用模块构建工具，允许开发者通过 `module.json` 配置文件来定义模块，而无需编写复杂的 CMakeLists.txt。

## 功能特点

- 基于 `module.json` 自动配置构建
- 自动处理依赖关系
- 支持静态库和动态库
- 自动处理平台特定的依赖
- 内置测试和示例支持

## 使用方法

### 1. 创建 module.json

在你的模块目录下创建一个 `module.json` 文件：

```json
{
  "name": "idcu-mymodule",
  "category": "library",
  "version": "1.0.0",
  "description": "My awesome module",
  "author": "IDCU Team",
  "license": "MIT",
  "dependencies": ["idcu-common"],
  "priority": "normal",
  "type": "library",
  "build": {
    "language": "C",
    "standard": "99",
    "type": "static",
    "sources": ["src/idcu/mymodule/*.c"],
    "includes": ["include"],
    "link_libraries": ["idcu-common"],
    "platform_deps": {
      "win32": ["ws2_32"],
      "unix": ["pthread"]
    }
  }
}
```

### 2. 创建简单的 CMakeLists.txt

在你的模块目录下创建一个简单的 `CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.10)

find_package(idcu-module-build REQUIRED)
idcu_build_module()
```

### 3. 构建项目

按照常规的 CMake 方式构建项目。

## module.json 字段说明

### 顶级字段

- `name`: 模块名称
- `category`: 模块分类（library, tool, etc.）
- `version`: 模块版本号
- `description`: 模块描述
- `author`: 作者信息
- `license`: 许可证
- `dependencies`: 依赖的模块列表
- `priority`: 优先级 (high, normal, low)
- `type`: 类型 (library, tool, etc.)

### build 字段

- `language`: 编程语言（目前仅支持 C）
- `standard`: 语言标准（如 99, 11 等）
- `type`: 库类型（static, shared）
- `sources`: 源文件路径模式
- `includes`: 头文件目录
- `link_libraries`: 需要链接的库
- `platform_deps`: 平台特定的依赖库

## 示例

查看 libs 目录下的其他模块，了解如何使用这个构建工具。

## 许可证

MIT License
