# 快速开始指南 (Quick Start Guide)

欢迎使用 IDCU Agent！本指南将帮助你快速上手这个基于微内核架构的实时代理程序。

---

## 前置条件

### 系统要求
- **Windows 10/11** 或 **Linux (Ubuntu/Debian/CentOS)**
- 至少 512MB 内存
- 100MB 可用磁盘空间

### 安装编译器

#### Windows 用户
推荐使用 **TDM-GCC**：
1. 访问 https://jmeubank.github.io/tdm-gcc/ 下载
2. 运行安装程序，使用默认设置
3. 验证安装：打开命令提示符，运行：
   ```cmd
   gcc --version
   ```
4. 如果显示版本信息，说明安装成功！

#### Linux 用户
大多数 Linux 发行版已预装编译器，验证安装：
```bash
gcc --version
cmake --version
```

如果未安装：
- **Ubuntu/Debian：**
  ```bash
  sudo apt-get update
  sudo apt-get install -y gcc make cmake
  ```
- **CentOS/Fedora：**
  ```bash
  sudo yum install -y gcc make cmake
  ```

---

## 编译 IDCU Agent

### 方式一：使用构建脚本（推荐）

#### Windows
双击 `scripts\build.bat` 或在命令提示符中运行：
```cmd
cd scripts
build.bat
```

#### Linux
```bash
chmod +x scripts/build.sh
./scripts/build.sh
```

### 方式二：手动使用 CMake 编译

#### Windows (MinGW)
```cmd
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make -j%NUMBER_OF_PROCESSORS%
```

#### Linux
```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### 编译输出
编译成功后，你会找到：
- 可执行文件：`out/idcu_agent.exe` (Windows) 或 `out/idcu_agent` (Linux)
- 配置文件：`out/agent.cfg`

---

## 运行 IDCU Agent

### 重要提示
**Windows 用户请勿直接双击可执行文件！** 这会导致窗口立即关闭。请始终使用命令行运行。

### Windows
```cmd
cd out
idcu_agent.exe
```

### Linux
```bash
cd out
./idcu_agent
```

### 预期输出
你会看到类似这样的输出：
```
idcu/agent hard real-time microkernel start...
[core_module] init
[base_log] init
[IDCU] All modules loaded successfully
[IDCU] Agent is running... (Press Ctrl+C to stop)
```

### 停止 Agent
按 `Ctrl + C` 优雅停止 Agent。

---

## 配置

主配置文件是 `config/agent.cfg`（编译后也会复制到 `out/agent.cfg`），使用 INI 格式。

### 关键配置项

#### [general] 通用配置
```ini
[general]
version = 3.0.0
log_level = info          # 日志级别：debug（详细）、info（普通）、warn（警告）、error（错误）
log_file = agent.log      # 日志文件路径
max_modules = 32          # 最大支持模块数量
max_messages = 128        # 消息队列大小
```

#### [modules] 模块配置
启用/禁用模块并设置优先级：
```ini
[modules]
core = base_log, core_module
business = biz_collect, heartbeat_module

enable_base_log = true
enable_core_module = true
heartbeat_module.priority = high
```

---

## 创建你的第一个模块

让我们创建一个简单的 "Hello World" 模块！

### 步骤 1：创建模块文件
创建 `modules/business/hello/src/hello_module.c`：
```c
#include "module_def.h"
#include <stdio.h>

static int g_counter = 0;

static int hello_init(void)
{
    printf("[hello] 模块初始化成功！\n");
    return 0;
}

static int hello_run(void)
{
    if (g_counter % 1000000 == 0) {
        printf("[hello] 正在运行！计数: %d\n", g_counter / 1000000);
    }
    g_counter++;
    return 0;
}

static int hello_stop(void)
{
    printf("[hello] 模块已停止！\n");
    return 0;
}

IDCU_REGISTER_MODULE(
    hello,
    IDCU_MODULE_VERSION(1, 0, 0),
    hello_init,
    hello_run,
    hello_stop
);
```

### 步骤 2：创建 CMakeLists.txt
创建 `modules/business/hello/CMakeLists.txt`：
```cmake
add_library(idcu_business_hello STATIC
    src/hello_module.c
)

target_include_directories(idcu_business_hello PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/modules/core/module-system/include
)
```

### 步骤 3：更新主 CMakeLists.txt
在业务模块部分添加：
```cmake
add_subdirectory(modules/business/hello)
```

同时将 `idcu_business_hello` 添加到链接库中。

### 步骤 4：更新配置
在 `config/agent.cfg` 中添加：
```ini
[modules]
business = ..., hello

enable_hello = true
hello.priority = normal
```

### 步骤 5：重新编译并运行
使用构建脚本重新编译，然后运行 Agent！

---

## 下一步
- 阅读 [模块开发手册](./module_development.md) 深入学习模块开发
- 探索 [架构设计文档](./architecture_en.md) 了解系统设计
- 查看 `modules/business/` 中的现有模块作为示例
