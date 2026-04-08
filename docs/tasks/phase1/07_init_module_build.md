# 任务 1.7: 初始化 idcu-module-build

## 目标

创建模块构建工具，这是最重要的一步，后续所有模块都将使用它。

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-module-build/cmake
mkdir -p libs/idcu-module-build/include
mkdir -p libs/idcu-module-build/src
mkdir -p libs/idcu-module-build/tests
mkdir -p libs/idcu-module-build/examples
```

### 2. 创建核心 CMake 模块文件

```cmake
# libs/idcu-module-build/cmake/idcu_module.cmake

# IDCU 模块构建辅助函数
include_guard()

# 创建一个 IDCU 库
function(idcu_add_library name)
    cmake_parse_arguments(IDCU_LIB
        ""
        "VERSION"
        "SOURCES;HEADERS;DEPENDS"
        ${ARGN}
    )
    
    # 创建库
    add_library(${name} STATIC ${IDCU_LIB_SOURCES})
    
    # 设置包含目录
    target_include_directories(${name} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    
    # 添加依赖
    if(IDCU_LIB_DEPENDS)
        target_link_libraries(${name} PUBLIC ${IDCU_LIB_DEPENDS})
    endif()
    
    # 创建别名
    add_library(idcu::${name} ALIAS ${name})
    
    message(STATUS "Added library: idcu::${name}")
endfunction()

# 创建一个 IDCU 模块
function(idcu_add_module name)
    cmake_parse_arguments(IDCU_MODULE
        ""
        "VERSION"
        "SOURCES;DEPENDS"
        ${ARGN}
    )
    
    # 创建模块库
    add_library(${name} STATIC ${IDCU_MODULE_SOURCES})
    
    # 设置包含目录
    target_include_directories(${name} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
    
    # 添加依赖
    if(IDCU_MODULE_DEPENDS)
        target_link_libraries(${name} PUBLIC ${IDCU_MODULE_DEPENDS})
    endif()
    
    message(STATUS "Added module: ${name}")
endfunction()
```

### 3. 创建完整的 idcu-module-build

**重要提示**：由于这个模块的复杂性，**强烈建议直接参考现有项目的完整实现**。现有项目的 `libs/idcu-module-build/` 目录包含：
- 完整的 CMake 模块
- 配置文件支持
- 跨平台支持
- 详细的文档

### 4. 验证 idcu-module-build

```bash
# 确保可以引用这个模块
# 在根 CMakeLists.txt 中已经添加了：
# list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/libs/idcu-module-build/cmake")
```

## 验收标准

- [ ] idcu-module-build 目录结构已创建
- [ ] 核心 CMake 模块已创建
- [ ] 可以被其他 CMakeLists.txt 引用
- [ ] 有详细的 README 文档

## Git 提交

```bash
git add libs/idcu-module-build/
git commit -m "feat(libs): initialize idcu-module-build

- Create directory structure
- Add core CMake module functions
- Add README documentation"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| CMake 找不到模块 | CMAKE_MODULE_PATH 未正确设置 | 检查根 CMakeLists.txt 中的配置 |
| 函数调用失败 | 参数格式不正确 | 检查函数调用的参数 |

## 经验提示

- 这是最重要的一步，多花时间设计
- 后续所有模块都将使用它
- 确保 API 稳定，避免后续重构
- 参考现有项目的完整实现
- 优先完成这个任务！
