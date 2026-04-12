# IDCU Agent

基于微内核架构的实时代理程序，提供模块化、可扩展的服务平台。

## 功能特性

- **微内核架构**：轻量级内核，支持动态加载模块
- **模块化设计**：独立的功能库，按需使用
- **消息总线**：模块间高效通信
- **协程调度**：轻量级并发处理
- **丰富的组件库**：
  - 日志系统
  - JSON/YAML 解析
  - 配置管理
  - 网络通信
  - HTTP 服务器/客户端
  - 指标监控
  - 健康检查
  - 存储系统
  - 等等...

## 快速开始

详见 [快速开始指南](docs/guide/QUICKSTART.md)

## 项目结构

```
idcu-agent/
├── .github/             # GitHub 配置
│   └── workflows/       # CI/CD 工作流
├── app/                 # 应用程序入口
├── build_yaml/          # YAML 构建系统输出目录（已加入 .gitignore）
├── config/              # 配置文件
│   └── default/         # 默认配置
├── docs/                # 文档
│   ├── api/             # API 文档
│   ├── examples/        # 示例代码
│   ├── guide/           # 指南文档
│   └── tasks/           # 开发任务
├── libs/                # 独立库（20+ 个功能库）
│   └── */               # 每个库包含 module.yaml 配置文件
├── modules/             # 业务模块
│   ├── business/        # 业务模块
│   └── integrations/    # 集成模块
├── scripts/             # 构建脚本
├── tests/               # 测试
│   ├── benchmark/       # 基准测试
│   └── integration/     # 集成测试
├── tools/               # 工具
├── .clang-format        # 代码格式配置
├── .clang-tidy          # 代码检查配置
├── .gitignore           # Git 忽略文件
├── build.py             # 主构建脚本（YAML 构建系统）
├── CMakeLists.txt       # CMake 构建配置（传统方式）
└── Dockerfile           # Docker 配置
```

### 模块配置文件 (module.yaml)

每个模块都包含一个 `module.yaml` 配置文件，定义了模块的元数据、依赖关系和构建信息。新的 YAML 构建系统会自动读取这些配置来构建模块。

## 文档

- [快速开始指南](docs/guide/QUICKSTART.md)
- [API 文档](docs/api/README.md)
- [架构设计](docs/guide/ARCHITECTURE.md)
- [模块系统](docs/guide/MODULE_SYSTEM.md)
- [消息总线](docs/guide/MSGBUS.md)
- [开发计划](docs/DEVELOPMENT_PLAN.md)

## 构建

### 前置条件

- Python 3.7+
- GCC/MinGW 编译器
- Git
- PyYAML (可选，用于 module.yaml 解析)

### 使用新的 YAML 构建系统（推荐）

项目现在完全使用基于 module.yaml 的构建系统，无需 CMakeLists.txt。

#### 列出所有模块

```bash
python build.py --list
```

#### 构建所有模块

```bash
python build.py
```

#### 构建特定模块

```bash
python build.py --module idcu-common,idcu-log
```

#### 清理后构建

```bash
python build.py --clean
```

#### Debug 模式构建

```bash
python build.py --build-type Debug
```

### CMake 构建系统（传统方式，仍可用）

#### 前置条件

- CMake 3.15+
- C 编译器（GCC 9+, Clang 11+, MSVC 2019+）

#### Linux/macOS

```bash
git clone <repository-url>
cd idcu-agent
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

#### Windows (PowerShell)

```powershell
git clone <repository-url>
cd idcu-agent
mkdir build ; cd build
cmake ..
cmake --build . --config Release
```

#### 运行测试

```bash
cd build
ctest -V
```

## 开发状态

项目正在积极开发中，详见 [开发计划](docs/DEVELOPMENT_PLAN.md) 了解详细进度。

## 许可证

本项目采用 [Apache License 2.0](LICENSE) 许可证。


