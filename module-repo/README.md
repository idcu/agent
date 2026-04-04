# IDCU Agent 官方模块仓库

本仓库包含 IDCU Agent 的官方模块、模板和示例。

## 目录结构

```
module-repo/
├── README.md                    # 本文件
├── index.json                   # 模块索引文件
├── official/                    # 官方模块目录
│   ├── core/                   # 核心功能模块
│   ├── monitoring/             # 监控相关模块
│   ├── networking/             # 网络相关模块
│   └── security/               # 安全相关模块
└── templates/                  # 模块开发模板
```

## 模块分类

### 核心模块 (core/)
- 基础功能模块
- 日志管理模块
- 配置管理模块

### 监控模块 (monitoring/)
- 健康检查模块
- 性能指标模块
- Prometheus 导出模块

### 网络模块 (networking/)
- TCP/UDP 通信模块
- 节点发现模块
- 分布式通信模块

### 安全模块 (security/)
- 沙箱模块
- 权限检查模块
- 加密模块

## 使用模块

### 内置模块
将模块文件复制到项目的 `modules/` 目录，并在 CMakeLists.txt 中配置编译。

### 动态加载模块
编译为 DLL/SO 动态库，使用动态模块加载器加载。

## 开发新模块

参考 `templates/` 目录下的模块模板进行开发。

## 模块索引

详见 `index.json` 文件获取完整的模块列表和信息。
