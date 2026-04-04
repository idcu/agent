# 教程一：快速入门

欢迎来到 IDCU Agent 的世界！这篇教程将带你从零开始，快速上手这个基于微内核架构的实时代理程序。

---

## 学习目标

完成本教程后，你将学会：
- 如何获取和编译 IDCU Agent
- 如何运行程序并查看输出
- 如何修改配置文件
- 如何添加一个简单的自定义模块

---

## 第一步：环境准备

### 1.1 系统要求

- **Windows 10/11** 或 **Linux (Ubuntu/Debian/CentOS)**
- 至少 512MB 内存
- 100MB 空闲磁盘空间

### 1.2 安装编译器

#### Windows 用户

推荐使用 **TDM-GCC**（对新手更友好）：
1. 访问 https://jmeubank.github.io/tdm-gcc/
2. 下载最新版本的 TDM-GCC 安装包
3. 运行安装程序，按照向导完成安装
4. 安装完成后，打开命令提示符（cmd），输入：
   ```bash
   gcc --version
   ```
   如果看到版本信息，说明安装成功！

#### Linux 用户

大多数 Linux 发行版已经预装了编译器，如果没有，可以使用以下命令安装：

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y gcc make cmake git
```

**CentOS/Fedora:**
```bash
sudo yum install -y gcc make cmake git
```

验证安装：
```bash
gcc --version
cmake --version
```

---

## 第二步：获取代码

### 2.1 使用 Git 克隆（推荐）

如果你熟悉 Git，这是最好的方式：

```bash
git clone <仓库地址>
cd idcu-agent
```

### 2.2 直接下载

如果你没有 Git，可以：
1. 访问项目主页
2. 点击 "Code" → "Download ZIP"
3. 解压下载的文件
4. 进入解压后的目录

---

## 第三步：编译程序

### 3.1 最简单的方法（推荐新手）

#### Windows 用户

直接双击项目根目录下的 `build.bat` 文件，等待编译完成。

#### Linux 用户

打开终端，进入项目目录，运行：

```bash
chmod +x build.sh
./build.sh
```

### 3.2 使用 CMake 手动编译

如果你想更灵活地控制编译过程：

#### Windows 用户

```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

#### Linux 用户

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)  # -j 使用多核编译，更快
```

### 3.3 验证编译

编译成功后，你会在项目根目录下看到 `out` 文件夹，里面应该有：
- `idcu_agent` (Linux) 或 `idcu_agent.exe` (Windows)

---

## 第四步：运行程序

### 4.1 基本运行

#### Windows 用户

```bash
cd out
idcu_agent.exe
```

#### Linux 用户

```bash
cd out
./idcu_agent
```

### 4.2 查看输出

程序启动后，你应该会看到类似这样的输出：

```
[IDCU] Starting agent...
[IDCU] Initializing micro kernel...
[IDCU] Loading modules...
[base_log] init
[core_module] init
[biz_collect] init
[IDCU] All modules loaded
[IDCU] Agent running... (Press Ctrl+C to stop)
```

### 4.3 停止程序

要停止程序，按 `Ctrl + C` 即可。程序会优雅地停止所有模块后退出。

---

## 第五步：修改配置

### 5.1 配置文件位置

配置文件位于 `config/agent.cfg`，使用 INI 格式。

### 5.2 主要配置项

```ini
[general]
version = 1.0.0
log_level = info          # 日志级别: debug, info, warn, error
log_file = agent.log      # 日志文件路径
max_modules = 32          # 最大模块数量
max_messages = 128        # 消息队列大小

[modules]
enable_base_log = true    # 是否启用基础日志模块
enable_core_module = true # 是否启用核心模块
enable_biz_collect = true # 是否启用数据收集模块
```

### 5.3 修改日志级别

让我们把日志级别改成 `debug`，看看更多细节：

1. 用文本编辑器打开 `config/agent.cfg`
2. 找到 `log_level = info` 这一行
3. 改成 `log_level = debug`
4. 保存文件
5. 重新运行程序

现在你会看到更详细的调试信息！

---

## 第六步：创建你的第一个模块

让我们创建一个简单的 "Hello World" 模块。

### 6.1 创建模块文件

在 `modules/business/` 目录下创建新的模块目录和文件：

```c
#include "core/module-system/include/module_def.h"
#include <stdio.h>

static int hello_init(void)
{
    printf("[hello] Hello, World! Module initialized.\n");
    return 0;
}

static int hello_run(void)
{
    // 这个函数会被内核反复调用
    // 我们在这里打印一条消息，每秒一次（仅作示例）
    static int count = 0;
    if (count % 100 == 0) {
        printf("[hello] I'm running! Count: %d\n", count / 100);
    }
    count++;
    return 0;
}

static int hello_stop(void)
{
    printf("[hello] Goodbye! Module stopped.\n");
    return 0;
}

IDCU_REGISTER_MODULE(hello, hello_init, hello_run, hello_stop);
```

### 6.2 创建模块的 CMakeLists.txt

在新模块目录下创建 `CMakeLists.txt`，参考其他业务模块的配置。CMake 会自动发现和构建所有模块。

### 6.3 重新编译并运行

```bash
# Windows: 重新运行 build.bat
# Linux: 重新运行 build.sh
```

运行程序，你会看到你的 hello 模块在工作！

---

## 常见问题解答

### Q: 编译时提示找不到 gcc？

A: 确保编译器已经安装，并且添加到了系统环境变量 PATH 中。Windows 用户可以尝试使用 "MinGW Terminal" 来运行命令。

### Q: 运行程序时一闪就没了？

A: 不要直接双击 exe 文件，先打开命令提示符，进入 out 文件夹，然后运行程序，这样就能看到错误信息了。

### Q: 如何清理编译文件？

A: 直接删除 `build` 和 `out` 文件夹即可。

### Q: 我的模块没有被加载？

A: 检查：
1. 模块文件是否正确添加到了 CMakeLists.txt
2. 模块是否使用了 `IDCU_REGISTER_MODULE` 宏
3. 重新编译程序

---

## 下一步

恭喜你完成了快速入门教程！接下来你可以：

- 阅读 [教程二：模块开发入门](./tutorial_02_module_development.md)，学习如何开发更复杂的模块
- 查看 [开发者指引](./developer_guide.md)，深入了解项目架构
- 探索现有的模块代码，学习别人是怎么写的

---

祝你学习愉快！如有问题，欢迎在社区提问。🎉
