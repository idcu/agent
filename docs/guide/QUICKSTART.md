# 快速入门指南

本指南将帮助你快速开始使用 IDCU Agent。

## 前置条件

### 适用于所有平台

- CMake 3.15 或更高版本
- 一个 C 编译器：
  - GCC 9+ 或 Clang 11+（Linux/macOS）
  - MSVC 2019+（Windows）
- Git

### 额外要求（可选）

- Docker（用于容器化构建）
- Python 3.7+（用于某些构建脚本）

## 构建项目

### 克隆仓库

```bash
git clone <repository-url>
cd idcu-agent
```

### Linux/macOS 构建

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### Windows 构建（PowerShell）

```powershell
mkdir build ; cd build
cmake ..
cmake --build . --config Release
```

### 构建选项

```bash
# 构建测试
cmake .. -DBUILD_TESTS=ON

# 构建示例
cmake .. -DBUILD_EXAMPLES=ON

# 发布版本构建
cmake .. -DCMAKE_BUILD_TYPE=Release
```

## 运行你的第一个程序

### Hello World 示例

创建一个文件 `hello.c`：

```c
#include <idcu/log/log.h>
#include <idcu/common/error_code.h>

int main(void) {
    // 初始化日志
    idcu_log_init(NULL, IDCU_LOG_INFO);

    IDCU_LOG_INFO("你好，IDCU Agent！");

    idcu_log_shutdown();
    return 0;
}
```

### 编译和运行

```bash
# 链接 idcu-common 和 idcu-log
gcc hello.c -o hello -lidcu-common -lidcu-log
./hello
```

## 测试

### 运行所有测试

```bash
cd build
ctest -V
```

### 运行特定测试

```bash
cd build
./bin/test_common
./bin/test_yaml
```

## 下一步

- 阅读 [API 文档](../api/README.md) 了解详细的库用法
- 探索 [示例](../examples/) 目录
- 查看 [架构指南](ARCHITECTURE.md) 了解系统设计

## 故障排除

### 构建错误

- 确保 CMake 在你的 PATH 中
- 检查编译器版本要求
- 清理构建目录并重试：

```bash
rm -rf build
mkdir build && cd build
cmake ..
```

### 链接错误

- 确保库搜索路径包含构建输出
- 按正确顺序指定库

## 获取帮助

- 检查 [README](../../README.md)
- 查看 [开发计划](../DEVELOPMENT_PLAN.md)
- 在仓库中打开一个 issue
