# IDCU Agent 官方模块仓库

本仓库包含 IDCU Agent 的官方模块集合、开发模板和模块索引管理。

## 概述

module-repo 是 IDCU Agent 的官方模块仓库，负责：
- 存储和管理官方提供的标准模块
- 提供模块开发模板和最佳实践
- 维护模块索引和元数据
- 支持模块的分类管理和版本控制

## 目录结构详解

```
module-repo/
├── README.md                    # 本文件 - 仓库说明文档
├── index.json                   # 模块索引文件 - 包含所有模块元数据
├── CMakeLists.txt              # CMake 构建配置 - 统一构建所有官方模块
├── official/                    # 官方模块目录 - 官方维护的标准模块
│   ├── core/                   # 核心基础模块
│   │   ├── core_module.c       # 核心基础功能模块
│   │   └── log_module.c        # 基础日志模块
│   ├── monitoring/             # 监控相关模块
│   │   ├── health_check_module.c    # 健康状态检测模块
│   │   └── metrics_module.c         # 性能指标收集和管理模块
│   ├── networking/             # 网络相关模块
│   │   ├── distributed_node_module.c   # 多节点分布式通信模块
│   │   ├── http_management_module.c    # HTTP 管理接口模块
│   │   ├── network_layer_module.c      # TCP/UDP Socket 网络层封装
│   │   └── node_discovery_module.c     # 节点自动发现模块
│   └── security/               # 安全相关模块
│       ├── sandbox_module.c            # 基础沙箱安全机制
│       └── sandbox_enhanced_module.c   # 增强的安全沙箱模块
└── templates/                  # 模块开发模板
    ├── README.md               # 模板使用说明
    └── module_template.c       # 模块代码模板
```

## 模块分类说明

### 核心模块 (core/)
提供系统最基础的功能模块：

| 模块 | 文件名 | 功能说明 |
|------|--------|---------|
| 核心模块 | core_module.c | 提供基础运行环境、模块协调、系统初始化等核心功能 |
| 日志模块 | log_module.c | 基础日志记录功能，是其他模块的依赖 |

### 监控模块 (monitoring/)
系统监控和健康管理相关模块：

| 模块 | 文件名 | 功能说明 |
|------|--------|---------|
| 健康检查模块 | health_check_module.c | 定期检测各模块和系统健康状态，支持告警 |
| 性能指标模块 | metrics_module.c | 收集、管理和导出系统性能指标 |

### 网络模块 (networking/)
网络通信和分布式功能模块：

| 模块 | 文件名 | 功能说明 |
|------|--------|---------|
| 网络层模块 | network_layer_module.c | 封装 TCP/UDP Socket，提供统一的网络接口 |
| 节点发现模块 | node_discovery_module.c | 自动发现网络中的其他 IDCU Agent 节点 |
| 分布式通信模块 | distributed_node_module.c | 实现多节点之间的通信和协调 |
| HTTP 管理模块 | http_management_module.c | 提供 HTTP REST API 用于管理和监控 |

### 安全模块 (security/)
安全和隔离相关模块：

| 模块 | 文件名 | 功能说明 |
|------|--------|---------|
| 沙箱模块 | sandbox_module.c | 基础沙箱机制，提供模块间安全隔离 |
| 增强沙箱模块 | sandbox_enhanced_module.c | 增强的沙箱安全，包含更多安全限制 |

## 模块索引 (index.json)

`index.json` 文件包含所有模块的元数据信息：

```json
{
  "version": "1.0.0",
  "modules": [
    {
      "name": "health_check_module",
      "category": "monitoring",
      "version": "1.0.0",
      "description": "健康状态检测模块",
      "dependencies": ["core_module", "log_module"],
      "config_section": "module.healthcheck_module"
    }
  ]
}
```

## 使用模块

### 方式 1：内置编译集成

将官方模块编译到主程序中：

```bash
# 1. 在主 CMakeLists.txt 中添加模块
# 2. 构建项目
mkdir -p build && cd build
cmake ..
make
```

### 方式 2：动态加载模块

将模块编译为动态库（.dll/.so），在运行时动态加载：

```bash
# 编译模块为动态库
cd module-repo/official/monitoring
mkdir -p build && cd build
cmake .. -DBUILD_SHARED_LIBS=ON
make

# 运行时加载
idcu_agent --load-module=health_check_module.so
```

### 方式 3：配置文件启用

在配置文件中启用需要的模块：

```ini
[modules]
core = base_log, core_module
monitoring = health_check_module, metrics_module
networking = network_layer_module, http_management_module
```

## 开发新模块

### 使用模板

参考 `templates/` 目录下的模块模板进行开发：

```bash
# 复制模板
cp module-repo/templates/module_template.c \
   modules/business/my_new_module/src/my_new_module.c

# 或者使用 idcu-module-build 工具
cd libs/idcu-module-build/scripts
python generate_template.py --name my_new_module \
   --output ../../modules/business/my_new_module
```

### 提交流程

如果开发的模块适合成为官方模块：

1. 在 `module-repo/official/{category}/` 下创建模块
2. 更新 `module-repo/index.json` 添加模块元数据
3. 编写模块文档和测试
4. 提交 Pull Request

## 配置模块

每个官方模块都有对应的配置段：

```ini
[module.healthcheck_module]
enabled = true
check_interval_ms = 10000
alert_threshold = 3
checks.cpu = true
checks.memory = true
```

详细配置说明请参考各模块的 README 文档。

## 最佳实践

### 模块开发

1. **遵循命名规范**：使用小写和下划线，`idcu_` 前缀
2. **提供完整文档**：每个模块应有 README 说明功能和配置
3. **编写单元测试**：使用 idcu-testframework 编写测试
4. **处理配置变更**：支持配置热更新，注册变更回调

### 模块使用

1. **按需加载**：只启用需要的模块，减少资源占用
2. **配置分离**：不同环境使用不同的配置文件
3. **监控健康**：启用健康检查模块，及时发现问题
4. **版本管理**：关注模块版本，及时更新

## 与 modules/ 目录的关系

- `module-repo/official/`：官方标准模块，经过充分测试和验证
- `modules/business/`：业务模块，项目特定的业务功能
- `modules/integrations/`：集成模块，连接 libs 和模块系统

一般情况下：
- 通用功能优先使用 `module-repo/official/` 中的模块
- 项目特定功能放在 `modules/business/`
- 需要集成 libs 的放在 `modules/integrations/`

## 更多资源

- [模块开发教程](../docs/tutorial_02_module_development.md)
- [架构设计文档](../docs/architecture.md)
- [模板使用说明](templates/README.md)
- [配置管理指南](../config/README.md)

