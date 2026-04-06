# IDCU Agent

一个基于微内核架构的实时代理程序，使用纯 C 语言编写。

---

## 这是什么？

IDCU Agent 就像一个"智能小管家"，可以在你的电脑后台安静地运行，帮你完成各种任务。它的核心设计理念是：**核心很小，功能通过添加模块来实现**。

打个比方：
- **微内核** = 一个万能插座板
- **模块** = 各种电器（台灯、电视、空调等）

你想用什么功能，就把对应的模块"插"上去就行，不用改动核心代码！

---

## 主要特点

🚀 **微内核架构** - 核心代码只有几百行，其他功能都通过模块实现
⚡ **实时响应快** - 采用协程调度，任务切换效率高
🔌 **模块化设计** - 想加什么功能，写个模块就行
💻 **跨平台** - Windows 和 Linux 都能用
📦 **轻量级** - 不需要安装复杂的依赖库
🛡️ **沙箱安全** - 模块之间安全隔离，互不影响
🔄 **协程调度** - 高效的多任务管理
📨 **消息总线** - 模块之间可以方便地通信
🔧 **动态加载** - 运行时可以热加载 DLL/SO 模块
📊 **监控支持** - 内置健康检查和指标收集
🌐 **分布式能力** - 支持多节点通信
⚠️ **告警系统** - 异常情况可以及时通知

---

## 项目结构

```
idcu-agent/
├── app/                          # 应用程序入口（就像房子的大门）
│   ├── main.c                    # 主程序入口
│   └── benchmark.c               # 性能测试程序
├── config/                       # 配置文件目录
│   └── agent.cfg                 # 主配置文件（可以在这里调整各种设置）
├── libs/                         # 独立库模块（可复用的基础组件）
│   ├── idcu-module-build/        # 模块构建工具
│   ├── idcu-common/              # 通用基础组件（原子操作、锁、错误码）
│   ├── idcu-log/                 # 日志系统
│   ├── idcu-config/              # 配置管理
│   ├── idcu-json/                # JSON 解析
│   ├── idcu-network/             # 网络层（TCP/UDP Socket）
│   ├── idcu-http-server/         # HTTP 服务器
│   ├── idcu-http-client/         # HTTP 客户端
│   ├── idcu-conn-pool/           # 连接池
│   ├── idcu-distributed/         # 分布式节点
│   ├── idcu-discovery/           # 节点发现
│   ├── idcu-metrics/             # 指标收集
│   ├── idcu-healthcheck/         # 健康检查
│   ├── idcu-alert/               # 告警系统
│   ├── idcu-memory/              # 内存池
│   ├── idcu-permission/          # 权限管理
│   └── idcu-sandbox/             # 沙箱安全
├── modules/                      # 模块化结构（这里是所有功能模块的家）
│   ├── core/                     # 核心基础设施（微内核的基础）
│   │   ├── micro-kernel/         # 微内核核心（最重要的部分）
│   │   ├── module-system/        # 模块管理系统
│   │   ├── scheduler/            # 调度器模块（协程、消息总线）
│   │   ├── sdk/                  # 软件开发工具包（帮你更容易写模块）
│   │   └── test-framework/       # 测试框架
│   ├── services/                 # 服务模块组（提供基础服务）
│   │   ├── storage/              # 存储服务
│   │   ├── cache/                # 缓存服务
│   │   ├── security/             # 安全系统
│   │   └── plugin/               # 插件系统
│   ├── integrations/             # 集成模块（连接库和模块系统）
│   │   ├── config-integration/
│   │   ├── healthcheck-integration/
│   │   ├── http-client-integration/
│   │   ├── http-server-integration/
│   │   ├── json-integration/
│   │   ├── log-integration/
│   │   ├── metrics-integration/
│   │   ├── network-integration/
│   │   └── network-monitor-integration/
│   └── business/                 # 业务模块组（具体的功能模块）
│       ├── core-module/          # 核心基础模块
│       ├── log/                  # 日志模块
│       ├── alert/                # 告警模块
│       ├── collect/              # 采集模块
│       ├── config/               # 配置模块
│       ├── data-collector/       # 数据采集模块
│       ├── healthcheck/          # 健康检查模块
│       ├── heartbeat/            # 心跳模块
│       ├── http-client/          # HTTP客户端模块
│       ├── metrics/              # 指标模块
│       ├── storage/              # 存储模块
│       ├── cache/                # 缓存模块
│       ├── security/             # 安全模块
│       ├── examples/             # 示例模块（学习参考）
│       │   ├── simple-example/   # 简单示例
│       │   └── messaging-example/# 消息通信示例
│       └── dynamic/              # 动态加载模块示例
├── module-repo/                  # 模块仓库（存放官方模块）
│   ├── official/                 # 官方模块
│   │   ├── core/                 # 核心模块
│   │   ├── monitoring/           # 监控模块
│   │   ├── networking/           # 网络模块
│   │   └── security/             # 安全模块
│   └── templates/                # 模块模板（照着写模块）
├── examples/                     # 示例程序
├── tests/                        # 测试用例
│   ├── unit/                     # 单元测试
│   └── integration/              # 集成测试
├── docs/                         # 文档目录（这里有详细的教程）
│   ├── api/                     # API 文档
│   │   ├── idcu-common.md
│   │   └── idcu-log.md
│   ├── user_guide.md             # 用户指南
│   ├── developer_guide.md        # 开发者指南
│   ├── architecture.md           # 架构设计文档
│   ├── tutorial_01_quick_start.md    # 教程一：快速入门
│   ├── tutorial_02_module_development.md  # 教程二：模块开发
│   ├── tutorial_03_message_bus.md      # 教程三：消息总线
│   ├── tutorial_04_coroutine.md         # 教程四：协程
│   ├── tutorial_05_sandbox.md           # 教程五：沙箱
│   ├── sdk_guide.md             # SDK 指南
│   ├── management_api.md        # 管理 API
│   ├── refactoring_plan.md      # 重构计划
│   ├── roadmap.md               # 路线图
│   └── community_guide.md       # 社区指南
├── scripts/                      # 构建脚本
│   ├── build.bat                 # Windows 编译脚本（双击就能编译）
│   ├── build.sh                  # Linux 编译脚本
│   └── generate_coverage.sh      # 覆盖率生成脚本
├── out/                          # 编译输出目录（程序会生成在这里）
├── CMakeLists.txt                # CMake 构建配置
├── CMakeLists_benchmark.txt      # 性能测试配置
├── Doxyfile                      # Doxygen 配置
├── CODE_OF_CONDUCT.md            # 行为准则
├── CONTRIBUTING.md               # 贡献指南
└── README.md                     # 你正在看的这个文件
```

---

## 前缀命名空间说明

为了避免命名冲突（就像两个人同名会混淆一样），本项目所有公共符号都使用 `idcu_` 前缀：

- **类型**: `idcu_TypeName` （例如：`idcu_Mutex`, `idcu_ErrorCode`）
- **函数**: `idcu_function_name()` （例如：`idcu_mutex_init()`）
- **宏**: `IDCU_MACRO_NAME` （例如：`IDCU_ERR_OK`, `IDCU_CONFIG_MAX_MODULES`）
- **枚举值**: `IDCU_ENUM_VALUE` （例如：`IDCU_MOD_STATE_INITED`）

---

## 快速开始

### 第一步：准备环境

在开始之前，你需要安装 C 语言编译器。

#### Windows 用户

推荐使用 **TDM-GCC**（对新手更友好）：
1. 访问 https://jmeubank.github.io/tdm-gcc/
2. 下载最新版本的 TDM-GCC 安装包
3. 运行安装程序，一路点击"下一步"
4. 安装完成后，打开命令提示符（按 Win+R，输入 cmd）
5. 输入 `gcc --version`，如果有版本信息输出，说明安装成功！

#### Linux 用户

大多数 Linux 发行版已经预装了编译器。打开终端，输入：
```bash
gcc --version
```
如果有输出说明可以用了。如果没有，用下面的命令安装：

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y gcc make cmake
```

**CentOS/Fedora:**
```bash
sudo yum install -y gcc make cmake
```

### 第二步：编译程序

#### 最简单的方法（推荐新手）

**Windows 用户**：直接双击项目文件夹里的 `scripts\build.bat` 文件，等它运行完就可以了！

**Linux 用户**：打开终端，进入项目文件夹，运行：
```bash
chmod +x scripts/build.sh
./scripts/build.sh
```

编译成功后，你会看到 `out` 文件夹里有 `idcu_agent.exe`（Windows）或 `idcu_agent`（Linux）。

### 第三步：运行程序

#### Windows 用户

打开命令提示符，进入项目文件夹：
```bash
cd out
idcu_agent.exe
```

#### Linux 用户

```bash
cd out
./idcu_agent
```

你会看到类似这样的输出：
```
idcu/agent hard real-time microkernel start...
[core_module] init
[base_log] init
...
idcu/agent hard real-time microkernel stopped.
```

### 第四步：停止程序

想停止程序时，按 `Ctrl + C` 就可以了。程序会优雅地停止所有模块后退出。

---

## 配置说明

程序的配置文件在 `config/agent.cfg`，你可以用记事本（Windows）或文本编辑器（Linux）打开它来修改。

配置文件使用 INI 格式，就像这样：

```ini
[general]
version = 3.0.0
log_level = info          # 日志级别：debug（详细）、info（普通）、warn（警告）、error（错误）
log_file = agent.log      # 日志文件路径
max_modules = 32          # 最多支持多少个模块
max_messages = 128        # 消息队列大小

[modules]
# 在这里配置要启用哪些模块
core = base_log, core_module
business = biz_collect, heartbeat_module, healthcheck_module

# 启用/禁用具体的模块
enable_base_log = true
enable_core_module = true
enable_biz_collect = true
enable_heartbeat_module = true
```

**提示**：修改配置后，需要重启程序才能生效。

---

## 怎么学习？

### 如果你只是想使用这个程序

看 [用户指南](./docs/user_guide.md)，里面有详细的使用说明。

### 如果你想自己开发模块

看 [教程一：快速入门](./docs/tutorial_01_quick_start.md)，从零开始教你。

然后看 [教程二：模块开发](./docs/tutorial_02_module_development.md)，学习如何开发更复杂的模块。

### 如果你想深入了解架构

看 [架构设计](./docs/architecture.md)，了解系统是怎么设计的。

### 如果你想贡献代码

看 [开发者指南](./docs/developer_guide.md) 和 [贡献指南](./CONTRIBUTING.md)。

---

## 学习资源

📚 **教程系列**（按顺序学习）：
1. [教程一：快速入门](./docs/tutorial_01_quick_start.md) - 带你从零开始
2. [教程二：模块开发](./docs/tutorial_02_module_development.md) - 学习写模块
3. [教程三：消息总线](./docs/tutorial_03_message_bus.md) - 模块间通信
4. [教程四：协程调度](./docs/tutorial_04_coroutine.md) - 多任务管理
5. [教程五：沙箱安全](./docs/tutorial_05_sandbox.md) - 安全机制

📖 **其他文档**：
- [用户指南](./docs/user_guide.md) - 如何使用 IDCU Agent
- [开发者指南](./docs/developer_guide.md) - 深入开发
- [架构设计](./docs/architecture.md) - 系统架构详解

---

## 常见问题

### Q: 双击 build.bat 没反应？

A: 可能是没有安装编译器。先按照"快速开始"里的说明安装 TDM-GCC。

### Q: 编译时提示找不到 gcc？

A: Windows 用户需要把 TDM-GCC 的 bin 目录加到系统环境变量 PATH 里。或者直接用 "MinGW Terminal" 来运行命令。

### Q: 运行程序时一闪就没了？

A: 不要直接双击 exe 文件！先打开命令提示符，进入 out 文件夹，然后输入 `idcu_agent.exe` 来运行，这样就能看到错误信息了。

### Q: 怎么清理编译出来的文件？

A: 直接删除 `out` 和 `build` 文件夹就可以了。

### Q: 我可以添加自己的功能吗？

A: 当然可以！看 [教程一：快速入门](./docs/tutorial_01_quick_start.md)，里面有详细的步骤。

### Q: 如何获得帮助？

A: 你可以：
1. 先看文档和教程，很多问题里面都有答案
2. 在 GitHub Issues 中提问
3. 看看代码，代码是最好的文档！

---

## 许可证

本项目仅供学习和研究使用。

---

## 祝你好运！

如果这是你第一次接触这个项目，别担心，跟着教程一步步来，你很快就能上手！有问题多试几次，编程就是这样，多练习就会了。加油！💪

有任何问题，欢迎在社区提问！🎉
