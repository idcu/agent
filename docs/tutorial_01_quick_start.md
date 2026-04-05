# 教程一：快速入门

欢迎来到 IDCU Agent 的世界！这篇教程将带你从零开始，一步步上手这个基于微内核架构的实时代理程序。别担心，即使你是编程新手，也能跟着完成！

---

## 学习目标

完成本教程后，你将学会：
- ✅ 如何安装必要的工具
- ✅ 如何获取和编译 IDCU Agent
- ✅ 如何运行程序并查看输出
- ✅ 如何修改配置文件
- ✅ 如何添加一个简单的自定义模块

---

## 第一步：环境准备

在开始之前，我们需要先安装一些工具。别担心，这很简单！

### 1.1 系统要求

- **Windows 10/11** 或 **Linux (Ubuntu/Debian/CentOS)**
- 至少 512MB 内存
- 100MB 空闲磁盘空间

### 1.2 安装编译器

什么是编译器？简单说，编译器就是把 C 语言代码翻译成电脑能直接运行的程序的工具，就像翻译官把中文翻译成英文一样。

#### Windows 用户

推荐使用 **TDM-GCC**（对新手特别友好）：

1. 打开浏览器，访问 https://jmeubank.github.io/tdm-gcc/
2. 点击 "Download" 下载最新版本的 TDM-GCC 安装包
3. 双击下载的安装包，开始安装
4. 一路点击 "Next"（下一步），使用默认设置就好
5. 安装完成后，我们来验证一下

**验证安装：**
1. 按键盘上的 `Win + R` 键
2. 在弹出的框里输入 `cmd`，然后按回车
3. 这时候会打开一个黑色的窗口（命令提示符）
4. 在里面输入：`gcc --version`，然后按回车
5. 如果看到类似这样的输出，说明安装成功了！
   ```
   gcc (tdm64-1) 10.3.0
   Copyright (C) 2020 Free Software Foundation, Inc.
   ...
   ```

**如果提示"不是内部或外部命令"怎么办？**
别慌！你可以在开始菜单里找 "MinGW Terminal" 或 "TDM-GCC Terminal"，用那个终端来运行命令就没问题了。

#### Linux 用户

大多数 Linux 发行版已经预装了编译器，我们先检查一下：

打开终端，输入：
```bash
gcc --version
```
如果有版本信息输出，说明已经安装好了！

**如果没有安装，按下面的步骤操作：**

**Ubuntu/Debian 用户：**
```bash
sudo apt-get update
sudo apt-get install -y gcc make cmake
```

**CentOS/Fedora 用户：**
```bash
sudo yum install -y gcc make cmake
```

**验证安装：**
安装完成后，再次输入：
```bash
gcc --version
cmake --version
```
都有版本信息输出，就说明安装成功了！

---

## 第二步：获取代码

现在我们来获取项目代码。

### 2.1 使用 Git 克隆（如果你有 Git）

如果你熟悉 Git，这是最好的方式：

```bash
git clone <仓库地址>
cd idcu-agent
```

### 2.2 直接下载（最简单）

如果你没有 Git，可以：

1. 访问项目主页
2. 点击 "Code" → "Download ZIP"
3. 下载完成后，解压文件
4. 进入解压后的目录

---

## 第三步：编译程序

现在我们来把源代码编译成可运行的程序。

### 3.1 最简单的方法（强烈推荐新手用这个！）

#### Windows 用户

1. 打开项目文件夹
2. 找到 `scripts` 文件夹
3. 双击里面的 `build.bat` 文件
4. 等待它运行完成（你会看到一个黑色窗口在闪，别担心，这是正常的）
5. 当窗口消失或提示"编译完成"时，就好了！

#### Linux 新手

1. 打开终端，进入项目文件夹
2. 给构建脚本添加执行权限：
   ```bash
   chmod +x scripts/build.sh
   ```
3. 运行构建脚本：
   ```bash
   ./scripts/build.sh
   ```
4. 等待编译完成

### 3.2 查看编译结果

编译成功后，你会在项目根目录下看到一个 `out` 文件夹，里面应该有：
- Windows：`idcu_agent.exe`
- Linux：`idcu_agent`

如果看到这些文件，恭喜你，编译成功了！🎉

---

## 第四步：运行程序

现在我们来运行程序，看看它是怎么工作的。

### 重要提示：不要直接双击 exe 文件！

**Windows 新手请注意：不要直接双击 idcu_agent.exe！那样程序运行后窗口会一闪就消失，你看不到任何输出。我们要用命令行来运行，这样才能看到程序的输出信息。**

### Windows 新手

1. 按 `Win + R`，输入 `cmd`，打开命令提示符
2. 进入项目的 `out` 文件夹。比如你的项目在 `D:\idcu-agent`，就输入：
   ```bash
   D:
   cd idcu-agent\out
   ```
3. 运行程序：
   ```bash
   idcu_agent.exe
   ```

### Linux 新手

1. 打开终端
2. 进入项目的 `out` 文件夹：
   ```bash
   cd ~/idcu-agent/out  # 假设项目在你的用户目录下
   ```
3. 运行程序：
   ```bash
   ./idcu_agent
   ```

### 你会看到什么？

程序启动后，你会看到类似这样的输出：

```
idcu/agent hard real-time microkernel start...
[core_module] init
[base_log] init
[biz_collect] init
[heartbeat_module] init
[healthcheck_module] init
[metrics_module] init
[IDCU] All modules loaded successfully
[IDCU] Agent is running... (Press Ctrl+C to stop)
[core_module] run (counter: 0)
[heartbeat_module] sending heartbeat...
...
```

这说明程序正常运行了！各个模块都在工作。

### 停止程序

想停止程序时，按键盘上的 `Ctrl + C` 就可以了。程序会：
1. 先停止所有模块
2. 清理资源
3. 然后优雅地退出

你会看到类似这样的输出：
```
^C[IDCU] Received signal, shutting down...
[core_module] stop
[base_log] stop
...
idcu/agent hard real-time microkernel stopped.
```

---

## 第五步：修改配置

现在我们来学习如何修改配置文件，让程序按照我们的想法运行。

### 5.1 配置文件在哪里？

配置文件位于 `config/agent.cfg`，用记事本（Windows）或任何文本编辑器（Linux）都可以打开它。

### 5.2 配置文件长什么样？

配置文件使用 **INI 格式**，这是一种很简单的格式：
- `[section]` 表示一个配置组
- `key = value` 表示一个配置项
- `#` 开头的行是注释，不会被程序读取

让我们来看一下配置文件的主要部分：

```ini
[general]
version = 3.0.0
log_level = info          # 日志级别
log_file = agent.log      # 日志文件路径
max_modules = 32          # 最多支持多少个模块
max_messages = 128        # 消息队列大小
```

**日志级别说明：**
- `debug` - 最详细，会打印所有调试信息（开发时用）
- `info` - 普通信息，日常使用推荐（默认）
- `warn` - 只显示警告和错误
- `error` - 只显示错误

### 5.3 动手修改：把日志级别改成 debug

让我们把日志级别改成 `debug`，看看更多细节：

1. 用记事本打开 `config/agent.cfg`
2. 找到 `log_level = info` 这一行
3. 改成 `log_level = debug`
4. 保存文件
5. 重新运行程序

现在你会看到更详细的调试信息！

---

## 第六步：创建你的第一个模块（进阶）

现在让我们来创建一个简单的 "Hello World" 模块！别担心，跟着步骤来就行。

### 6.1 先看看现有的示例模块

在开始写自己的模块之前，让我们先看看现有的示例模块是怎么写的。

打开 `modules/business/examples/simple-example/src/simple_example.c`，你会看到一个完整的示例模块。

### 6.2 创建你的第一个模块

假设你要创建一个叫 `hello` 的模块：

#### 第一步：创建模块文件

在 `modules/business/` 目录下创建 `hello/` 目录，然后在里面创建 `src/hello_module.c`：

```c
#include "module_def.h"
#include "log.h"
#include <stdio.h>

static int g_hello_counter = 0;

// 初始化函数 - 模块加载时调用一次
static int hello_init(void)
{
    printf("[hello] Hello, World! Module initialized!\n");
    g_hello_counter = 0;
    return 0;
}

// 运行函数 - 会被内核反复调用
static int hello_run(void)
{
    // 每隔一段时间打印一次（避免刷屏）
    if (g_hello_counter % 1000000 == 0) {
        printf("[hello] I'm running! Count: %d\n", 
               g_hello_counter / 1000000);
    }
    g_hello_counter++;
    return 0;
}

// 停止函数 - 模块卸载时调用
static int hello_stop(void)
{
    printf("[hello] Goodbye! Module stopped!\n");
    return 0;
}

// 注册模块！这一行很重要！
IDCU_REGISTER_MODULE(
    hello,
    IDCU_MODULE_VERSION(1, 0, 0),
    hello_init,
    hello_run,
    hello_stop
);
```

#### 第二步：创建 CMakeLists.txt

在 `modules/business/hello/` 目录下创建 `CMakeLists.txt`：

```cmake
add_library(idcu_business_hello STATIC
    src/hello_module.c
)

target_include_directories(idcu_business_hello PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/modules/core/common/include
    ${CMAKE_SOURCE_DIR}/modules/core/utils/include
    ${CMAKE_SOURCE_DIR}/modules/core/module-system/include
)

target_link_libraries(idcu_business_hello PRIVATE
    idcu_core_common
    idcu_core_utils
    idcu_core_module_system
)
```

#### 第三步：更新主 CMakeLists.txt

在项目根目录的 `CMakeLists.txt` 中，找到 "业务模块" 部分，添加：

```cmake
add_subdirectory(modules/business/hello)
```

然后在链接选项中添加 `idcu_business_hello`（参考其他业务模块的写法）。

#### 第四步：更新配置文件

在 `config/agent.cfg` 中，添加你的模块：

```ini
[modules]
business = ... , hello   # 把你的模块加在这里

# 启用模块
enable_hello = true

# 设置优先级
hello.priority = normal
```

#### 第五步：重新编译并运行

```bash
# Windows: 重新运行 build.bat
# Linux: 重新运行 build.sh
```

运行程序，你会看到你的 hello 模块在工作！

---

## 常见问题解答

这里整理了一些新手经常遇到的问题，希望能帮到你。

### Q: 编译时提示找不到 gcc？

**A:** 
- Windows 用户：确保已经安装了 TDM-GCC，并且用 "MinGW Terminal" 来运行命令
- Linux 用户：确认已经安装了 gcc

### Q: 运行程序时一闪就没了？

**A:** 不要直接双击 exe 文件！先打开命令提示符，进入 out 文件夹，然后运行程序，这样就能看到错误信息了。

### Q: 怎么清理编译出来的文件？

**A:** 直接删除项目根目录下的 `out` 和 `build` 文件夹就可以了。

### Q: 我的模块没有被加载？

**A:** 检查：
1. 模块文件是否正确添加到了 CMakeLists.txt
2. 模块是否使用了 `IDCU_REGISTER_MODULE` 宏
3. 配置文件中是否启用了该模块
4. 重新编译程序

---

## 下一步

恭喜你完成了快速入门教程！🎉

接下来你可以：

- 📚 阅读 [教程二：模块开发入门](./tutorial_02_module_development.md)，学习如何开发更复杂的模块
- 📖 查看 [用户指南](./user_guide.md)，深入了解如何使用 IDCU Agent
- 🔧 查看 [开发者指南](./developer_guide.md)，深入了解系统架构和开发技巧
- 💻 探索现有的模块代码，学习别人是怎么写的
- ✏️ 试试修改现有的示例模块，看看效果

---

## 祝你学习愉快！

记住：
- 多看代码，代码是最好的文档！
- 多动手，多练习，编程就是这样，多试几次就会了
- 有问题先看文档和教程，很多问题里面都有答案
- 实在不懂就问，社区欢迎新手提问

继续加油！你做得很好！💪
