# IDCU Agent 官方模块仓库

本仓库包含 IDCU Agent 的官方模块、模板和示例。

## 目录结构

```
module-repo/
├── README.md                    # 本文件
├── index.json                   # 模块索引文件
├── CMakeLists.txt              # CMake 构建配置
├── official/                    # 官方模块目录
│   ├── monitoring/             # 监控相关模块
│   │   ├── health_check_module.c
│   │   ├── metrics_module.c
│   │   └── prometheus_exporter_module.c
│   ├── networking/             # 网络相关模块
│   │   ├── distributed_node_module.c
│   │   ├── http_management_module.c
│   │   ├── network_layer_module.c
│   │   └── node_discovery_module.c
│   └── security/               # 安全相关模块
│       ├── sandbox_module.c
│       └── sandbox_enhanced_module.c
└── templates/                  # 模块开发模板
    ├── README.md
    └── module_template.c
```

## 模块分类

### 监控模块 (monitoring/)
- **健康检查模块** (`health_check_module.c`) - 模块健康状态检测
- **性能指标模块** (`metrics_module.c`) - 性能指标收集和管理
- **Prometheus 导出模块** (`prometheus_exporter_module.c`) - Prometheus 格式指标导出

### 网络模块 (networking/)
- **网络层模块** (`network_layer_module.c`) - TCP/UDP Socket 封装
- **节点发现模块** (`node_discovery_module.c`) - 节点自动发现
- **分布式通信模块** (`distributed_node_module.c`) - 多节点通信
- **HTTP 管理模块** (`http_management_module.c`) - HTTP 管理接口

### 安全模块 (security/)
- **沙箱模块** (`sandbox_module.c`) - 基础沙箱安全机制
- **增强沙箱模块** (`sandbox_enhanced_module.c`) - 增强的安全沙箱

## 使用模块

### 内置模块
官方模块可以通过项目的构建系统直接编译集成使用。

### 动态加载模块
模块可以编译为 DLL/SO 动态库，使用动态模块加载器加载。

## 开发新模块

参考 `templates/` 目录下的模块模板进行开发。

## 模块索引

详见 `index.json` 文件获取完整的模块列表和信息。
