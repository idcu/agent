# 构建指南

本文档详细介绍 IDCU Agent 的构建流程、依赖要求和平台兼容性。

## 目录

- [构建指南](#构建指南)
  - [目录](#目录)
  - [系统要求](#系统要求)
    - [最低依赖版本](#最低依赖版本)
    - [Windows 平台](#windows-平台)
    - [Linux 平台](#linux-平台)
  - [快速构建](#快速构建)
    - [Windows](#windows)
    - [Linux/macOS](#linuxmacos)
  - [构建参数详解](#构建参数详解)
    - [Windows (build.bat)](#windows-buildbat)
    - [Linux/macOS (build.sh)](#linuxmacos-buildsh)
  - [输出目录结构](#输出目录结构)
  - [高级构建选项](#高级构建选项)
    - [直接使用 CMake](#直接使用-cmake)
    - [启用测试](#启用测试)
    - [代码覆盖率](#代码覆盖率)
  - [常见问题](#常见问题)

## 系统要求

### 最低依赖版本

| 工具 | 最低版本 | 说明 |
|------|---------|------|
| CMake | 3.14+ | 构建系统 |
| GCC | 7.0+ | Linux 编译器 |
| Clang | 6.0+ | Linux/macOS 编译器 |
| MSVC | 19.1+ (VS 2017) | Windows 编译器 |
| MinGW-w64 | 7.0+ | Windows 替代编译器 |

### Windows 平台

#### 编译器选项

1. **MSVC (推荐)**
   - Visual Studio 2017 或更高版本
   - 下载地址：https://visualstudio.microsoft.com/
   - 安装时选择"使用 C++ 的桌面开发"工作负载

2. **MinGW-w64**
   - 推荐使用 MSYS2 安装：https://www.msys2.org/
   - 或者使用 TDM-GCC：https://jmeubank.github.io/tdm-gcc/

#### 依赖安装

- **CMake**: 从 https://cmake.org/download/ 下载并安装
- 确保将 CMake 添加到系统 PATH

### Linux 平台

#### Ubuntu/Debian

```bash
# 安装基础构建工具
sudo apt-get update
sudo apt-get install -y build-essential cmake

# 可选：安装 Clang
sudo apt-get install -y clang

# 可选：安装代码覆盖率工具
sudo apt-get install -y lcov
```

#### CentOS/RHEL

```bash
# 安装基础构建工具
sudo yum groupinstall -y "Development Tools"
sudo yum install -y cmake

# 对于 CentOS 7，需要使用 devtoolset 获取较新版本的 GCC
sudo yum install -y centos-release-scl
sudo yum install -y devtoolset-7-gcc devtoolset-7-gcc-c++
scl enable devtoolset-7 bash
```

#### Arch Linux

```bash
sudo pacman -S base-devel cmake clang
```

## 快速构建

### Windows

#### 最简单方式（自动检测）

```cmd
cd scripts
build.bat
```

#### 指定编译器

```cmd
# 使用 MSVC
build.bat -c msvc

# 使用 MinGW
build.bat -c mingw
```

#### Debug 构建

```cmd
build.bat -t Debug -C
```

### Linux/macOS

#### 最简单方式

```bash
cd scripts
chmod +x build.sh
./build.sh
```

#### 指定编译器

```bash
# 使用 GCC
./build.sh -C gcc

# 使用 Clang
./build.sh -C clang
```

#### Debug 构建

```bash
./build.sh -t Debug -c
```

## 构建参数详解

### Windows (build.bat)

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `-h, --help` | 显示帮助信息 | - |
| `-t, --build-type TYPE` | 构建类型：Debug/Release/RelWithDebInfo | Release |
| `-c, --compiler COMPILER` | 编译器：msvc/mingw/auto | auto |
| `-b, --with-benchmark` | 包含基准测试 | ON |
| `--no-benchmark` | 不包含基准测试 | - |
| `-T, --with-tests` | 包含测试 | OFF |
| `-C, --clean` | 清理旧构建后重新构建 | OFF |
| `-v, --verbose` | 显示详细输出 | OFF |

**示例**：

```cmd
# Release 构建，使用 MSVC，包含测试
build.bat -t Release -c msvc -T

# Debug 构建，清理缓存，使用 MinGW
build.bat -t Debug -C -c mingw --no-benchmark
```

### Linux/macOS (build.sh)

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `-h, --help` | 显示帮助信息 | - |
| `-t, --build-type TYPE` | 构建类型：Debug/Release/RelWithDebInfo | Release |
| `-b, --with-benchmark` | 包含基准测试 | ON |
| `--no-benchmark` | 不包含基准测试 | - |
| `-T, --with-tests` | 包含测试 | OFF |
| `-c, --clean` | 清理旧构建后重新构建 | OFF |
| `-C, --compiler COMPILER` | 编译器：gcc/clang | 自动检测 |
| `-v, --verbose` | 显示详细输出 | OFF |

**示例**：

```bash
# Release 构建，使用 Clang，包含测试
./build.sh -t Release -C clang -T

# Debug 构建，清理缓存，不包含基准测试
./build.sh -t Debug -c --no-benchmark
```

## 输出目录结构

构建完成后，所有产物统一输出到 `build/output/{build_type}/` 目录：

```
build/output/
├── debug/
│   ├── bin/
│   │   ├── idcu_agent          # 主程序
│   │   ├── idcu_benchmark      # 基准测试程序（可选）
│   │   └── config/              # 配置文件
│   │       └── agent.cfg
│   └── lib/                      # 库文件（如果有）
│       └── *.a / *.so / *.lib
└── release/
    ├── bin/
    └── lib/
```

## 高级构建选项

### 直接使用 CMake

如果需要更精细的控制，可以直接使用 CMake：

```bash
# 创建构建目录
mkdir -p build && cd build

# 配置
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=OFF \
    -DBUILD_BENCHMARKS=ON

# 编译
make -j$(nproc)
```

Windows MSVC:

```cmd
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### 启用测试

```bash
./build.sh -t Debug -T

# 运行测试
cd build
ctest --output-on-failure
```

### 代码覆盖率

仅在 Linux/macOS 上支持，需要 GCC 或 Clang：

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_CODE_COVERAGE=ON -DBUILD_TESTS=ON
make -j$(nproc)
make coverage
```

覆盖率报告将生成在 `build/coverage_report/` 目录。

## 常见问题

### Q: CMake 版本过低怎么办？

A: 请从 https://cmake.org/download/ 下载并安装最新版本的 CMake。

### Q: Windows 上找不到 MSVC 编译器？

A: 
1. 确保已安装 Visual Studio 2017 或更高版本
2. 打开 "Developer Command Prompt for VS" 再运行构建脚本
3. 或者使用 `-c mingw` 参数使用 MinGW 编译器

### Q: Linux 上编译提示 pthread 相关错误？

A: 确保已安装 `libpthread`，通常在 `build-essential` 包中。

### Q: 如何完全清理构建？

A: 
```bash
# Linux/macOS
rm -rf build

# Windows
rd /s /q build
```

### Q: 构建产物在哪里？

A: 根据构建类型，产物位于：
- Release: `build/output/release/bin/`
- Debug: `build/output/debug/bin/`

### Q: 如何交叉编译？

A: 直接使用 CMake 指定工具链文件：
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/toolchain.cmake
```
