# 任务 1.3: 配置 CMake 构建系统

## 目标

创建灵活、可扩展的 CMake 构建系统，为 idcu-module-build 预留接口。

## 详细步骤

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

## 验证检查清单

- [ ] CMakeLists.txt 语法正确
- [ ] 可以运行 `cmake -B build` 不报错
- [ ] 可以看到配置摘要输出
- [ ] 支持 Debug 和 Release 构建
- [ ] 有 BUILD_TESTS 和 BUILD_EXAMPLES 选项

## Git 提交

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

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| CMake 找不到编译器 | 编译器未安装或不在 PATH 中 | 安装编译器并检查 PATH |
| CMake 版本过低 | 使用的 CMake 版本低于 3.14 | 升级 CMake |
| 配置摘要不显示 | IDCU_AGENT_VERSION_STRING 未定义 | 检查是否正确设置了版本变量 |

## 测试构建

### Windows (MinGW)

```bash
# 创建构建目录
mkdir -p build
cd build

# 配置 CMake
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug ..

# 编译
mingw32-make -j4
```

### Windows (MSVC)

```bash
# 创建构建目录
mkdir -p build
cd build

# 配置 CMake（Visual Studio 2019）
cmake -G "Visual Studio 16 2019" -A x64 ..

# 编译 Debug 版本
cmake --build . --config Debug
```

### Linux

```bash
# 创建构建目录
mkdir -p build
cd build

# 配置 CMake
cmake -DCMAKE_BUILD_TYPE=Debug ..

# 编译
make -j4
```

## 经验提示

- 为 idcu-module-build 预留接口
- 支持多个平台的编译选项
- 使用选项控制是否构建测试和示例
- 配置摘要有助于快速验证构建配置
