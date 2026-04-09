# 任务 1.7: 初始化 idcu-module-build

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建模块构建工具 idcu-module-build，这是后续所有模块开发的基础。提供 CMake 辅助函数来简化库和模块的创建过程。

### 1.2 不做什么
- 不实现完整的包管理功能
- 不实现模块依赖自动解析
- 不实现模块版本管理

### 1.3 输入
- 无（从零创建）

### 1.4 输出
- idcu-module-build 目录结构
- CMake 模块文件：`libs/idcu-module-build/cmake/idcu_module.cmake`
- CMakeLists.txt 配置
- README 文档

### 1.5 前置依赖
- 任务 1.6 已完成（测试框架已搭建）
- CMake 构建系统已配置

---

## 2. 技术实现方案

### 2.1 核心选型
- 构建工具：CMake 3.14+
- 模块类型：静态库（STATIC）
- 命名空间：idcu::

### 2.2 核心逻辑
```
1. 创建目录结构
2. 创建 CMake 模块文件
   ├─ idcu_add_library()：创建库
   └─ idcu_add_module()：创建模块
3. 创建 CMakeLists.txt
4. 创建 README 文档
```

### 2.3 数据结构/接口
```cmake
# 创建库
idcu_add_library(name
    [VERSION version]
    SOURCES source1 source2 ...
    [HEADERS header1 header2 ...]
    [DEPENDS depend1 depend2 ...]
)

# 创建模块
idcu_add_module(name
    [VERSION version]
    SOURCES source1 source2 ...
    [DEPENDS depend1 depend2 ...]
)
```

### 2.4 跨平台适配
- 使用 CMake 的标准命令，确保跨平台兼容
- 静态库在 Windows 上生成 .lib，在 Linux 上生成 .a

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] idcu-module-build 目录结构已创建
- [ ] 核心 CMake 模块已创建
- [ ] CMakeLists.txt 已创建
- [ ] README 文档已创建
- [ ] idcu_add_library() 函数可以正常使用
- [ ] idcu_add_module() 函数可以正常使用
- [ ] 可以被其他 CMakeLists.txt 引用

### 3.2 性能验收
- CMake 配置时间增加 ≤ 1 秒
- 库/模块创建时间 ≤ 5 秒

### 3.3 异常验收
- [ ] 参数缺失时给出明确错误
- [ ] 依赖未找到时给出明确错误

---

## 4. 执行计划

### 4.1 工期
1.5 小时/人

### 4.2 里程碑
- D7-00: 创建目录结构
- D7-20: 创建 CMake 模块文件
- D7-40: 创建 CMakeLists.txt
- D7-60: 创建 README 文档
- D7-90: 测试验证和提交

### 4.3 人力
1 人（技能要求：CMake 函数编写）

---

## 5. 工程化要求

### 5.1 编码规范
- CMake 代码使用 4 空格缩进
- 函数名使用小写+下划线
- 提供清晰的注释和使用示例

### 5.2 测试要求
- 测试 idcu_add_library() 函数
- 测试 idcu_add_module() 函数
- 验证跨平台编译

### 5.3 部署指引
- 在根 CMakeLists.txt 中添加：`list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/libs/idcu-module-build/cmake")`

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：CMake 函数设计不够灵活，后续需要重构  
应对：参考现有项目的完整实现，确保 API 设计合理

### 6.2 风险2
描述：跨平台兼容性问题  
应对：使用 CMake 标准命令，充分测试

### 6.3 风险3
描述：模块依赖关系管理复杂  
应对：提供清晰的依赖关系示例和文档

### 6.4 风险4
描述：构建配置错误导致模块无法正确编译  
应对：提供详细的错误提示和调试信息

---

## 7. 详细实现步骤

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

### 3. 创建 CMakeLists.txt

```cmake
# libs/idcu-module-build/CMakeLists.txt

# 这个模块主要提供 CMake 函数，不需要编译代码
# 但我们可以创建一个空的库来满足构建系统
add_library(idcu_module_build INTERFACE)

target_include_directories(idcu_module_build INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_library(idcu::module_build ALIAS idcu_module_build)

message(STATUS "idcu-module-build initialized")
```

### 4. 创建 README

```markdown
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

## 重要提示

由于这个模块的复杂性，**强烈建议直接参考现有项目的完整实现**。现有项目的 `libs/idcu-module-build/` 目录包含：
- 完整的 CMake 模块
- 配置文件支持
- 跨平台支持
- 详细的文档
```

### 5. 参考现有项目的完整实现

**重要提示**：由于这个模块的复杂性，**强烈建议直接参考现有项目的完整实现**。现有项目的 `libs/idcu-module-build/` 目录包含：
- 完整的 CMake 模块
- 配置文件支持
- 跨平台支持
- 详细的文档

---

## 8. 验证检查清单

- [ ] idcu-module-build 目录结构已创建
- [ ] 核心 CMake 模块已创建
- [ ] CMakeLists.txt 已创建
- [ ] README 文档已创建
- [ ] 可以被其他 CMakeLists.txt 引用
- [ ] idcu_add_library() 函数正常工作
- [ ] idcu_add_module() 函数正常工作
- [ ] 符合工程化标准中的构建要求
- [ ] 跨平台兼容性已验证
- [ ] 已通过代码质量工具检查
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-module-build/
git commit -m "feat(libs): initialize idcu-module-build

- Create directory structure
- Add core CMake module functions
- Add README documentation"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| CMake 找不到模块 | CMAKE_MODULE_PATH 未正确设置 | 检查根 CMakeLists.txt 中的配置 |
| 函数调用失败 | 参数格式不正确 | 检查函数调用的参数 |
