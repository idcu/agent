# 任务 2.2: 完善 idcu-module-build

## 目标

完善模块构建系统，使其能够支持：
- 自动发现和构建模块
- 统一的模块元数据配置（使用 YAML）
- 跨平台构建支持
- 测试和示例的集成
- 与现有项目的实现对齐

## 详细步骤

### 1. 完善目录结构

```bash
mkdir -p libs/idcu-module-build/config
mkdir -p libs/idcu-module-build/scripts
mkdir -p libs/idcu-module-build/include
mkdir -p libs/idcu-module-build/src
mkdir -p libs/idcu-module-build/tests
mkdir -p libs/idcu-module-build/examples
```

### 2. 创建默认配置文件 (default.yaml)

创建 `libs/idcu-module-build/config/default.yaml`：

```yaml
build:
  cmake_minimum_required: "3.15"
  c_standard: "11"
  cxx_standard: "17"

defaults:
  library_type: STATIC
  build_testing: true
  build_examples: false
  install_prefix: "/usr/local"

paths:
  include_dir: "include"
  source_dir: "src"
  test_dir: "tests"
  example_dir: "examples"
  config_dir: "config"

modules:
  metadata_file: "module.yaml"
  json_metadata_file: "module.json"
```

### 3. 创建自动适配配置 (auto-adapter.yaml)

创建 `libs/idcu-module-build/config/auto-adapter.yaml`：

```yaml
adapters:
  - name: cmake
    enabled: true
    priority: 100
    patterns:
      - "CMakeLists.txt"

  - name: make
    enabled: true
    priority: 90
    patterns:
      - "Makefile"
      - "makefile"

  - name: meson
    enabled: true
    priority: 80
    patterns:
      - "meson.build"

auto_discovery:
  enabled: true
  scan_depth: 3
  exclude_dirs:
    - "build"
    - "out"
    - ".git"
    - "node_modules"

validation:
  required_fields:
    - name
    - version
    - description

  optional_fields:
    - author
    - license
    - dependencies
    - build
    - features
    - testing
```

### 4. 创建 CMakeLists.txt

创建 `libs/idcu-module-build/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-module-build VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" OFF)

set(IDCU_MODULE_BUILD_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/config" CACHE PATH "Module build config directory")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/idcu/module_build/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/idcu/module_build/config.h"
    @ONLY
)

add_library(idcu-module-build INTERFACE)

target_include_directories(idcu-module-build INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_library(idcu::module-build ALIAS idcu-module-build)

function(idcu_add_module module_name)
    set(options STATIC SHARED MODULE)
    set(oneValueArgs VERSION DESCRIPTION)
    set(multiValueArgs SOURCES INCLUDE_DIRS LINK_LIBRARIES DEPENDS)
    cmake_parse_arguments(IDCU_MODULE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(IDCU_MODULE_STATIC)
        set(lib_type STATIC)
    elseif(IDCU_MODULE_SHARED)
        set(lib_type SHARED)
    elseif(IDCU_MODULE_MODULE)
        set(lib_type MODULE)
    else()
        set(lib_type STATIC)
    endif()

    add_library(${module_name} ${lib_type} ${IDCU_MODULE_SOURCES})

    if(IDCU_MODULE_VERSION)
        set_target_properties(${module_name} PROPERTIES VERSION ${IDCU_MODULE_VERSION})
    endif()

    if(IDCU_MODULE_DESCRIPTION)
        set_target_properties(${module_name} PROPERTIES DESCRIPTION ${IDCU_MODULE_DESCRIPTION})
    endif()

    if(IDCU_MODULE_INCLUDE_DIRS)
        target_include_directories(${module_name} PUBLIC ${IDCU_MODULE_INCLUDE_DIRS})
    endif()

    if(IDCU_MODULE_LINK_LIBRARIES)
        target_link_libraries(${module_name} PRIVATE ${IDCU_MODULE_LINK_LIBRARIES})
    endif()

    if(IDCU_MODULE_DEPENDS)
        add_dependencies(${module_name} ${IDCU_MODULE_DEPENDS})
    endif()
endfunction()

function(idcu_add_module_test test_name)
    if(NOT BUILD_TESTING)
        return()
    endif()

    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES LINK_LIBRARIES)
    cmake_parse_arguments(IDCU_TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${test_name} ${IDCU_TEST_SOURCES})
    target_link_libraries(${test_name} PRIVATE ${IDCU_TEST_LINK_LIBRARIES})
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 5. 创建模块构建配置头文件模板

创建 `libs/idcu-module-build/include/idcu/module_build/config.h.in`：

```c
#ifndef IDCU_MODULE_BUILD_CONFIG_H
#define IDCU_MODULE_BUILD_CONFIG_H

#define IDCU_MODULE_BUILD_VERSION "@PROJECT_VERSION@"
#define IDCU_MODULE_BUILD_CONFIG_DIR "@IDCU_MODULE_BUILD_CONFIG_DIR@"

#endif
```

### 6. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-module-build/module.yaml`：

```yaml
name: idcu-module-build
version: 1.0.0
description: Module build system for IDCU Agent
author: IDCU Team
license: MIT

dependencies: []

build:
  type: cmake
  targets:
    - idcu-module-build

config_files:
  - config/default.yaml
  - config/auto-adapter.yaml

scripts:
  - scripts/build.py
  - scripts/build.sh
  - scripts/build.bat

features:
  - cmake_helpers: CMake helper functions
  - auto_discovery: Automatic module discovery
  - yaml_config: YAML-based module configuration
  - cross_platform: Cross-platform build support

testing:
  enabled: true
  framework: internal
```

### 7. 创建 Python 构建脚本

创建 `libs/idcu-module-build/scripts/build.py`：

```python
#!/usr/bin/env python3
import os
import sys
import argparse
import subprocess
import yaml
from pathlib import Path

def load_config(config_path):
    with open(config_path, 'r', encoding='utf-8') as f:
        return yaml.safe_load(f)

def find_modules(root_dir):
    modules = []
    for path in Path(root_dir).rglob('module.yaml'):
        modules.append(path.parent)
    return sorted(modules)

def build_module(module_dir, build_type='Release'):
    build_dir = module_dir / 'build'
    build_dir.mkdir(exist_ok=True)

    cmake_args = [
        'cmake',
        '-B', str(build_dir),
        '-S', str(module_dir),
        f'-DCMAKE_BUILD_TYPE={build_type}',
    ]

    print(f"Configuring {module_dir.name}...")
    result = subprocess.run(cmake_args)
    if result.returncode != 0:
        return False

    print(f"Building {module_dir.name}...")
    build_args = ['cmake', '--build', str(build_dir), '--config', build_type]
    result = subprocess.run(build_args)
    return result.returncode == 0

def main():
    parser = argparse.ArgumentParser(description='IDCU Module Builder')
    parser.add_argument('--root', default='.', help='Root directory to search for modules')
    parser.add_argument('--config', help='Configuration file path')
    parser.add_argument('--build-type', default='Release', choices=['Debug', 'Release', 'RelWithDebInfo'])
    parser.add_argument('--list', action='store_true', help='List discovered modules')

    args = parser.parse_args()

    root_dir = Path(args.root).resolve()

    if args.config:
        config = load_config(args.config)
        print(f"Loaded config: {config}")

    modules = find_modules(root_dir)

    if args.list:
        print("Discovered modules:")
        for module in modules:
            print(f"  - {module.relative_to(root_dir)}")
        return 0

    print(f"Found {len(modules)} modules")

    success_count = 0
    for module in modules:
        if build_module(module, args.build_type):
            success_count += 1
        else:
            print(f"Failed to build {module.name}", file=sys.stderr)

    print(f"\nSuccessfully built {success_count}/{len(modules)} modules")
    return 0 if success_count == len(modules) else 1

if __name__ == '__main__':
    sys.exit(main())
```

### 8. 创建 README.md

创建 `libs/idcu-module-build/README.md`：

```markdown
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
```

### 9. 创建根目录的模块构建辅助 CMake

创建 `libs/idcu-module-build/cmake/IdcuModuleBuild.cmake`（如果需要的话）：

```cmake
include_guard()

if(NOT COMMAND idcu_add_module)
    function(idcu_add_module module_name)
        # 简化版本，与主 CMakeLists.txt 中的函数保持一致
    endfunction()
endif()
```

## 验证检查清单

- [ ] 默认配置文件已创建
- [ ] 自动适配配置已创建
- [ ] CMakeLists.txt 已创建，包含辅助函数
- [ ] module.yaml 配置文件已创建
- [ ] Python 构建脚本已创建
- [ ] README.md 已创建
- [ ] 可以使用 idcu_add_module 函数构建简单模块
- [ ] 可以使用 Python 脚本发现和构建模块

## Git 提交

```bash
git add libs/idcu-module-build/
git commit -m "feat: improve idcu-module-build

- Add default.yaml and auto-adapter.yaml configs
- Add CMake helper functions
- Add Python build script
- Add module.yaml metadata
- Add comprehensive README"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| CMake 函数未找到 | 未正确包含模块构建 | 确保 find_package(idcu-module-build) 被调用 |
| 模块未被发现 | module.yaml 位置不对 | 确保 module.yaml 在模块根目录 |
| YAML 解析错误 | YAML 格式不对 | 检查 module.yaml 的 YAML 语法 |
