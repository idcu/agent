# idcu-module-build

IDCU 模块构建辅助工具，提供 CMake 函数简化库和模块的创建。

## 快速开始

### 在项目中使用

在根 CMakeLists.txt 中添加：

```cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/libs/idcu-module-build/cmake")
include(idcu_module)
```

### 创建一个库

```cmake
idcu_add_library(my_lib
    SOURCES src/my_lib.c
    HEADERS include/idcu/my_lib/my_lib.h
    DEPENDS idcu::common
)
```

### 创建一个模块

```cmake
idcu_add_module(my_module
    SOURCES src/my_module.c
    DEPENDS idcu::my_lib
)
```

## API 参考

### idcu_add_library

创建一个 IDCU 库。

**参数：**
- `NAME`：库名称
- `SOURCES`：源文件列表
- `HEADERS`：头文件列表（可选）
- `DEPENDS`：依赖库列表（可选）
- `VERSION`：版本号（可选）

**示例：**
```cmake
idcu_add_library(idcu_log
    SOURCES src/idcu/log/log.c
    HEADERS include/idcu/log/log.h
    DEPENDS idcu::common
)
```

### idcu_add_module

创建一个 IDCU 模块。

**参数：**
- `NAME`：模块名称
- `SOURCES`：源文件列表
- `DEPENDS`：依赖库列表（可选）
- `VERSION`：版本号（可选）

**示例：**
```cmake
idcu_add_module(log_module
    SOURCES src/log_module.c
    DEPENDS idcu::log
)
```

## 目录结构

```
libs/idcu-module-build/
├── cmake/
│   └── idcu_module.cmake    # CMake 模块文件
├── include/                  # 头文件目录（预留）
├── src/                      # 源文件目录（预留）
├── tests/                    # 测试目录（预留）
├── examples/                 # 示例目录（预留）
├── CMakeLists.txt            # 构建配置
└── README.md                 # 本文档
```
