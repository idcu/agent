# 任务 1.3: 配置 CMake 构建系统

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建灵活、可扩展的 CMake 构建系统，支持多个平台（Windows/MSVC、Windows/MinGW、Linux），提供 Debug/Release 配置选项，为 idcu-module-build 预留接口。

### 1.2 不做什么
- 不实现复杂的构建脚本
- 不集成第三方构建工具（如 meson、ninja）
- 不处理跨编译（仅支持本地编译）

### 1.3 输入
- 项目源代码（app/、libs/）
- 编译器（GCC、Clang、MSVC）
- CMake 3.14+

### 1.4 输出
- 可执行文件：build/bin/idcu_agent
- 静态库：build/lib/*.a（或 .lib）
- 构建配置摘要

### 1.5 前置依赖
- 任务 1.1 已完成（项目目录结构已创建）
- 任务 1.2 已完成（主程序入口已创建）
- CMake 3.14 或更高版本已安装
- C 编译器已安装并配置

---

## 2. 技术实现方案

### 2.1 核心选型
- 构建系统：CMake 3.14+
- 编译器支持：MSVC、GCC、Clang
- C 标准：C99

### 2.2 核心逻辑
```
1. 根 CMakeLists.txt 配置
   ├─ 设置项目和 C 标准
   ├─ 配置编译选项（平台相关）
   ├─ 设置输出目录
   ├─ 定义构建选项（BUILD_TESTS、BUILD_EXAMPLES）
   └─ 包含子目录
2. app/CMakeLists.txt 配置
   └─ 构建主程序可执行文件
3. libs/CMakeLists.txt 配置
   ├─ 添加 idcu-module-build
   └─ 预留其他库的添加位置
4. tests/CMakeLists.txt 配置
   └─ 预留测试框架集成
```

### 2.3 数据结构/接口
CMake 函数和变量：
- `idcu_add_library()`：预留用于创建库
- `idcu_add_module()`：预留用于创建模块
- `CMAKE_MODULE_PATH`：用于加载 idcu-module-build 模块
- `BUILD_TESTS`、`BUILD_EXAMPLES`：构建选项

### 2.4 跨平台适配
- Windows/MSVC：使用 `/W4 /WX /utf-8 /MP` 编译选项
- Windows/MinGW：使用 `-Wall -Wextra -Wpedantic -Werror` 编译选项
- Linux/GCC/Clang：使用 `-Wall -Wextra -Wpedantic -Werror` 编译选项
- Debug/Release 构建配置在所有平台都支持

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] `cmake -B build` 配置成功，无错误
- [ ] `cmake --build build` 编译成功
- [ ] Debug 版本可以正常编译
- [ ] Release 版本可以正常编译
- [ ] 支持 BUILD_TESTS=ON/OFF 选项
- [ ] 支持 BUILD_EXAMPLES=ON/OFF 选项
- [ ] 配置摘要正确显示

### 3.2 性能验收
- CMake 配置时间 ≤ 5 秒
- 完整编译时间 ≤ 30 秒（首次）
- 增量编译时间 ≤ 5 秒

### 3.3 异常验收
- [ ] 编译器警告作为错误处理
- [ ] CMake 版本过低时给出明确提示
- [ ] 编译器未找到时给出明确提示

---

## 4. 执行计划

### 4.1 工期
2 小时/人

### 4.2 里程碑
- D3-00: 完成根 CMakeLists.txt
- D3-30: 完成 app/CMakeLists.txt
- D3-60: 完成 libs/CMakeLists.txt
- D3-90: 完成 tests/CMakeLists.txt
- D3-120: 完成测试验证和提交

### 4.3 人力
1 人（技能要求：CMake 基础知识）

---

## 5. 工程化要求

### 5.1 编码规范
- CMake 代码使用 4 空格缩进
- 函数名使用小写+下划线
- 变量名使用大写+下划线
- 注释使用 `#` 开头

### 5.2 测试要求
- 在至少一个平台上验证配置和编译
- 测试 Debug 和 Release 两种构建类型
- 验证 BUILD_TESTS 和 BUILD_EXAMPLES 选项

### 5.3 部署指引
- 配置命令：`cmake -B build -DCMAKE_BUILD_TYPE=Debug`
- 编译命令：`cmake --build build -j4`
- 安装路径（可选）：`cmake --install build --prefix /usr/local`

---

## 6. 风险与应对

### 6.1 风险1
描述：CMake 版本不兼容  
应对：要求最低 CMake 3.14，在 CMakeLists.txt 开头检查版本

### 6.2 风险2
描述：编译器选项差异导致编译失败  
应对：分别为 MSVC 和 GCC/Clang 配置不同的编译选项

---

## 7. 详细实现步骤

### 1. 创建根目录 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.14)
project(idcu_agent C)

# 设置 C 标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 编译选项
if(MSVC)
    # MSVC 特定选项
    add_compile_options(/W4 /WX /utf-8)
    add_compile_options(/MP)  # 多核编译
else()
    # GCC/Clang 通用选项
    add_compile_options(-Wall -Wextra -Wpedantic)
    add_compile_options(-Werror)  # 警告当作错误
    
    # 调试构建选项
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
        add_compile_definitions(DEBUG=1)
    endif()
    
    # 发布构建选项
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3)
        add_compile_definitions(NDEBUG=1)
    endif()
endif()

# 选项：是否构建测试
option(BUILD_TESTS "Build unit tests" ON)

# 选项：是否构建示例
option(BUILD_EXAMPLES "Build example programs" ON)

# 添加 idcu-module-build 模块
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/libs/idcu-module-build/cmake")

# 包含子目录
add_subdirectory(libs)
add_subdirectory(app)

# 测试（如果启用）
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# 打印配置摘要
message(STATUS "")
message(STATUS "========================================")
message(STATUS "IDCU Agent Configuration Summary")
message(STATUS "========================================")
message(STATUS "  Version: ${IDCU_AGENT_VERSION_STRING}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "  Build Examples: ${BUILD_EXAMPLES}")
message(STATUS "========================================")
message(STATUS "")
```

### 2. 创建 app/CMakeLists.txt

```cmake
# app/CMakeLists.txt

# 主程序
add_executable(idcu_agent
    main.c
    src/version.c
)

target_include_directories(idcu_agent PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 安装规则
install(TARGETS idcu_agent
    RUNTIME DESTINATION bin
)
```

### 3. 创建 libs/CMakeLists.txt

```cmake
# libs/CMakeLists.txt

# idcu-module-build 是一个特殊的模块，先添加它
add_subdirectory(idcu-module-build)

# 其他库将在后续阶段添加
# add_subdirectory(idcu-common)
# add_subdirectory(idcu-log)
# ...
```

### 4. 创建 tests/CMakeLists.txt

```cmake
# tests/CMakeLists.txt
# 测试框架将在后续阶段添加
message(STATUS "Tests will be added in later stages")
```

### 5. 创建 idcu-module-build 目录结构（占位）

```bash
mkdir -p libs/idcu-module-build/cmake
```

### 6. 测试构建（Windows MinGW）

```bash
# 创建构建目录
mkdir -p build
cd build

# 配置 CMake
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug ..

# 编译
mingw32-make -j4
```

### 7. 测试构建（Windows MSVC）

```bash
# 创建构建目录
mkdir -p build
cd build

# 配置 CMake（Visual Studio 2019）
cmake -G "Visual Studio 16 2019" -A x64 ..

# 编译 Debug 版本
cmake --build . --config Debug
```

### 8. 测试构建（Linux）

```bash
# 创建构建目录
mkdir -p build
cd build

# 配置 CMake
cmake -DCMAKE_BUILD_TYPE=Debug ..

# 编译
make -j4
```

---

## 8. 验证检查清单

- [ ] 根 CMakeLists.txt 已创建
- [ ] app/CMakeLists.txt 已创建
- [ ] libs/CMakeLists.txt 已创建
- [ ] tests/CMakeLists.txt 已创建
- [ ] CMake 配置成功（无错误）
- [ ] 可以看到配置摘要输出
- [ ] Debug 版本编译成功
- [ ] Release 版本编译成功
- [ ] BUILD_TESTS 选项工作正常
- [ ] BUILD_EXAMPLES 选项工作正常
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add CMakeLists.txt
git add app/CMakeLists.txt
git add libs/CMakeLists.txt
git add tests/CMakeLists.txt
git commit -m "chore(build): add CMake build system

- Add root CMakeLists.txt with configuration options
- Add app CMakeLists.txt
- Add libs CMakeLists.txt
- Add tests CMakeLists.txt
- Add MSVC and GCC/Clang compiler options"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| CMake 找不到编译器 | 编译器未安装或不在 PATH 中 | 安装编译器并检查 PATH |
| CMake 版本过低 | 使用的 CMake 版本低于 3.14 | 升级 CMake |
| 配置摘要不显示 | IDCU_AGENT_VERSION_STRING 未定义 | 检查是否正确设置了版本变量 |
