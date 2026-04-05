# IDCU Agent 用户指南

欢迎使用 IDCU Agent！这篇文档会用最简单的方式，手把手教你怎么用好这个程序。

---

## 目录

1. [准备工作](#准备工作)
2. [编译程序](#编译程序)
3. [运行程序](#运行程序)
4. [配置程序](#配置程序)
5. [模块说明](#模块说明)
6. [常见问题](#常见问题)
7. [下一步](#下一步)

---

## 准备工作

在开始之前，你需要先安装一些必要的工具。别担心，这很简单！

### 什么是编译器？

简单说，编译器就是把 C 语言代码翻译成电脑能直接运行的程序的工具。就像翻译官把中文翻译成英文一样。

### Windows 用户

你需要安装 **TDM-GCC**（一个 Windows 上的 C 语言编译器），它对新手特别友好。

**安装步骤：**
1. 打开浏览器，访问 https://jmeubank.github.io/tdm-gcc/
2. 点击 "Download" 下载最新版本的 TDM-GCC 安装包
3. 双击下载的安装包，开始安装
4. 一路点击 "Next"（下一步），使用默认设置就好
5. 安装完成后，我们来验证一下是否安装成功

**验证安装：**
1. 按键盘上的 `Win + R` 键（Win键就是那个有 Windows 图标的键）
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
别慌，这说明编译器的路径没有加到系统环境变量里。你可以：
1. 在开始菜单里找 "MinGW Terminal" 或 "TDM-GCC Terminal"
2. 用那个终端来运行命令，就没问题了

### Linux 用户

Linux 系统通常已经自带了编译器，我们先检查一下。

**检查是否已安装：**
打开终端，输入：
```bash
gcc --version
```
如果有版本信息输出，说明已经安装好了，可以直接跳到[编译程序](#编译程序)部分。

**如果没有安装，按下面的步骤操作：**

**Ubuntu/Debian 用户：**
```bash
sudo apt-get update          # 更新软件列表
sudo apt-get install -y gcc make cmake  # 安装编译器和构建工具
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

## 编译程序

现在我们来把源代码编译成可运行的程序。

### 方法一：最简单的方法（强烈推荐新手用这个！）

#### Windows 用户

1. 打开项目文件夹
2. 找到 `scripts` 文件夹
3. 双击里面的 `build.bat` 文件
4. 等待它运行完成（你会看到一个黑色窗口在闪，别担心，这是正常的）
5. 当窗口消失或提示"编译完成"时，就好了！

#### Linux 用户

1. 打开终端，进入项目文件夹（比如你的项目在 `~/idcu-agent`，就输入 `cd ~/idcu-agent`）
2. 给构建脚本添加执行权限：
   ```bash
   chmod +x scripts/build.sh
   ```
3. 运行构建脚本：
   ```bash
   ./scripts/build.sh
   ```
4. 等待编译完成

### 方法二：用 CMake 手动编译（如果你想更灵活地控制）

如果你熟悉 CMake，或者想了解更多细节，可以用这个方法。

#### Windows 用户

```bash
mkdir build          # 创建 build 文件夹
cd build             # 进入 build 文件夹
cmake -G "MinGW Makefiles" ..  # 生成 Makefile
mingw32-make         # 开始编译
```

#### Linux 用户

```bash
mkdir build          # 创建 build 文件夹
cd build             # 进入 build 文件夹
cmake ..             # 生成 Makefile
make -j$(nproc)      # 开始编译（-j 使用多核编译，更快）
```

### 查看编译结果

编译成功后，你会在项目根目录下看到一个 `out` 文件夹，里面应该有：
- Windows：`idcu_agent.exe`
- Linux：`idcu_agent`

如果看到这些文件，恭喜你，编译成功了！🎉

---

## 运行程序

现在我们来运行程序，看看它是怎么工作的。

### 重要提示：不要直接双击 exe 文件！

**Windows 用户请注意：** 不要直接双击 `idcu_agent.exe`！那样程序运行后窗口会一闪就消失，你看不到任何输出。

我们要用命令行来运行，这样才能看到程序的输出信息。

### Windows 用户

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

### Linux 用户

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

## 配置程序

IDCU Agent 的所有配置都在 `config/agent.cfg` 文件里。你可以用任何文本编辑器打开它来修改。

### 配置文件格式

配置文件使用 **INI 格式**，这是一种很简单的格式：
- `[section]` 表示一个配置组
- `key = value` 表示一个配置项
- `#` 开头的行是注释，不会被程序读取

### 快速看懂配置文件

让我们来看一下配置文件的主要部分：

#### 1. 通用配置 [general]

```ini
[general]
version = 3.0.0              # 配置文件版本（不用改）
log_level = info              # 日志级别
log_file = agent.log          # 日志文件路径
max_modules = 32              # 最多支持多少个模块
max_messages = 128            # 消息队列大小
```

**日志级别说明：**
- `debug` - 最详细，会打印所有调试信息（开发时用）
- `info` - 普通信息，日常使用推荐（默认）
- `warn` - 只显示警告和错误
- `error` - 只显示错误

#### 2. 模块分类配置 [modules]

这里配置要加载哪些模块，按优先级分组：

```ini
[modules]
# Level 0: 核心基础模块（最先加载）
core = base_log, core_module

# Level 1: 基础服务模块
foundation = 

# Level 2: 服务层模块
monitoring = 
networking = 
security = 

# Level 3: 业务功能模块
business = biz_collect, heartbeat_module, healthcheck_module, metrics_module, alert_module, config_module, http_client_module, storage_module, cache_module, security_module

# Level 4: 扩展功能模块（最后加载）
extension = 
```

#### 3. 模块启用/禁用配置

这里可以单独控制每个模块是否启用：

```ini
# 核心模块（通常保持启用）
enable_base_log = true
enable_core_module = true

# 业务模块（可根据需要启用/禁用）
enable_biz_collect = true
enable_heartbeat_module = true
enable_healthcheck_module = true
enable_metrics_module = true
enable_alert_module = true
enable_config_module = true
enable_http_client_module = true
enable_storage_module = true
enable_cache_module = true
enable_security_module = true
```

**怎么禁用一个模块？**
把 `true` 改成 `false` 就行。比如你不想用心跳模块，就改成：
```ini
enable_heartbeat_module = false
```

#### 4. 模块优先级配置

这里可以调整每个模块的优先级：

```ini
base_log.priority = high
core_module.priority = realtime
biz_collect.priority = normal
heartbeat_module.priority = high
```

**优先级说明：**
- `realtime` - 实时（最高优先级，关键模块用）
- `high` - 高
- `normal` - 普通（默认）
- `low` - 低

#### 5. 模块权限配置

这里配置每个模块有哪些权限：

```ini
[permissions]
base_log = log,run
core_module = module_mgr,config,run
biz_collect = send,recv,run,hw
heartbeat_module = send,recv,run
```

**常用权限：**
- `log` - 写日志
- `run` - 运行
- `send` - 发送消息
- `recv` - 接收消息
- `config` - 读写配置
- `module_mgr` - 管理其他模块
- `hw` - 访问硬件

#### 6. 模块特定配置

每个模块还可以有自己的特定配置，比如心跳模块：

```ini
[module.heartbeat_module]
enabled = true                    # 主开关
interval_ms = 5000                # 心跳间隔（毫秒）
timeout_ms = 15000                # 超时时间（毫秒）
targets = 127.0.0.1:8080,192.168.1.1:8080  # 心跳目标列表
retry_count = 3                   # 重试次数
check_response = true             # 是否检查心跳响应
```

### 修改配置的步骤

1. 用文本编辑器打开 `config/agent.cfg`
2. 找到你想修改的配置项
3. 修改值
4. 保存文件
5. **重要：重启程序才能让配置生效！**

### 配置示例：修改日志级别

假设你想看到更详细的日志信息：

1. 打开 `config/agent.cfg`
2. 找到 `log_level = info`
3. 改成 `log_level = debug`
4. 保存文件
5. 重启程序

现在你会看到更多的调试信息！

---

## 模块说明

IDCU Agent 自带了很多实用的模块，让我们来了解一下它们都是做什么的。

### 核心模块

#### base_log（基础日志模块）
- **作用**：提供日志功能，让其他模块可以输出日志
- **配置项**：`enable_base_log`
- **优先级**：high
- **通常保持启用**

#### core_module（核心模块）
- **作用**：系统的核心基础模块
- **配置项**：`enable_core_module`
- **优先级**：realtime
- **必须启用**

### 业务模块

#### biz_collect（采集模块）
- **作用**：采集各种数据
- **配置项**：`enable_biz_collect`
- **优先级**：normal

#### heartbeat_module（心跳模块）
- **作用**：定期发送心跳，检测目标是否在线
- **配置项**：
  - `enable_heartbeat_module` - 是否启用
  - `heartbeat_module.interval_ms` - 心跳间隔
  - `heartbeat_module.targets` - 心跳目标列表
- **优先级**：high

#### healthcheck_module（健康检查模块）
- **作用**：检查系统和模块的健康状态
- **配置项**：
  - `enable_healthcheck_module` - 是否启用
  - `healthcheck_module.check_interval_ms` - 检查间隔
  - `healthcheck_module.alert_threshold` - 告警阈值
- **优先级**：normal

#### metrics_module（指标模块）
- **作用**：收集和导出系统指标
- **配置项**：
  - `enable_metrics_module` - 是否启用
  - `metrics_module.collection_interval_ms` - 采集间隔
  - `metrics_module.export_enabled` - 是否导出
- **优先级**：normal

#### alert_module（告警模块）
- **作用**：根据规则发送告警通知
- **配置项**：
  - `enable_alert_module` - 是否启用
  - `alert_module.notification_url` - 通知 URL
  - `alert_module.channels.*` - 通知渠道配置
- **优先级**：low

#### config_module（配置模块）
- **作用**：管理配置文件
- **配置项**：`enable_config_module`
- **优先级**：high

#### http_client_module（HTTP 客户端模块）
- **作用**：发送 HTTP 请求
- **配置项**：`enable_http_client_module`
- **优先级**：normal

#### storage_module（存储模块）
- **作用**：提供数据存储功能
- **配置项**：`enable_storage_module`
- **优先级**：normal

#### cache_module（缓存模块）
- **作用**：提供数据缓存功能
- **配置项**：`enable_cache_module`
- **优先级**：normal

#### security_module（安全模块）
- **作用**：提供安全相关功能
- **配置项**：`enable_security_module`
- **优先级**：high

### 示例模块

项目还包含一些示例模块，供你学习参考：
- `simple-example` - 简单示例
- `messaging-example` - 消息通信示例
- `advanced-example` - 高级示例

你可以在 `modules/business/examples/` 目录下找到它们。

---

## 常见问题

这里整理了一些用户经常遇到的问题，希望能帮到你。

### Q: 双击 build.bat 没反应？

**A:** 可能的原因和解决方法：
1. 没有安装编译器 - 按照[准备工作](#准备工作)里的说明安装 TDM-GCC
2. 安装了编译器但路径没配置对 - 用开始菜单里的 "MinGW Terminal" 来运行
3. 杀毒软件拦截了 - 暂时关闭杀毒软件试试

### Q: 编译时提示找不到命令？

**A:** 
- Windows 用户：把 TDM-GCC 的 bin 目录加到系统环境变量 PATH 里，或者用 "MinGW Terminal"
- Linux 用户：确认已经安装了 gcc 和 make

### Q: 运行程序时一闪就没了？

**A:** 不要直接双击 exe 文件！先打开命令提示符，进入 out 文件夹，然后运行程序，这样就能看到错误信息了。

### Q: 怎么清理编译出来的文件？

**A:** 直接删除项目根目录下的 `out` 和 `build` 文件夹就可以了。

### Q: 程序启动后某个模块报错怎么办？

**A:** 
1. 检查配置文件，确认该模块已启用
2. 把日志级别改成 `debug`，看看详细的错误信息
3. 检查该模块的特定配置是否正确

### Q: 我可以只启用部分模块吗？

**A:** 当然可以！在配置文件里把不想用的模块的 `enable_xxx` 改成 `false` 就行。

### Q: 修改配置后不生效？

**A:** 修改配置后需要重启程序才能生效！

### Q: 程序占用 CPU 很高怎么办？

**A:** 
1. 检查是否有模块在死循环
2. 调整日志级别，减少日志输出
3. 调整模块的优先级

### Q: 我可以把程序放到其他电脑上运行吗？

**A:** 可以！但是要注意：
- Windows 上编译的只能在 Windows 上运行
- Linux 上编译的只能在 Linux 上运行
- 如果用了动态链接库，需要一起带上

### Q: 如何查看程序的运行状态？

**A:** 
1. 看控制台输出的日志
2. 如果启用了 metrics 模块，可以看指标数据
3. 如果启用了 healthcheck 模块，可以看健康状态

---

## 下一步

恭喜你！现在你已经会基本使用 IDCU Agent 了！

如果你想：
- **了解这个程序是怎么工作的** → 看 [架构设计](./architecture.md)
- **自己开发模块** → 看 [教程一：快速入门](./tutorial_01_quick_start.md)
- **深入开发** → 看 [开发者指南](./developer_guide.md)

---

## 祝你使用愉快！

如果遇到问题，别担心：
1. 先看这篇文档，很多问题这里都有答案
2. 再看看日志输出，通常能找到线索
3. 最后可以在社区提问

多试几次，你会越来越熟练的！加油！💪
