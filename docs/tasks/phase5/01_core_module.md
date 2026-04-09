# 任务 5.1: core-module - 核心基础模块

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建核心基础模块，支持：
- 模块初始化和清理
- 配置加载和管理
- 日志初始化
- 模块生命周期管理
- 模块依赖管理
- 健康检查集成
- 指标收集集成
- 优雅关闭
- 信号处理

### 1.2 不做什么
- 不实现具体的业务逻辑
- 不实现复杂的分布式协调
- 不实现图形界面

### 1.3 输入
- 配置文件：config/app.yaml
- 依赖库：idcu-common, idcu-log, idcu-config, idcu-healthcheck, idcu-metrics

### 1.4 输出
- 核心模块实例：idcu_CoreModule
- 模块管理器：idcu_ModuleManager
- 配置管理器访问接口
- 日志访问接口

### 1.5 前置依赖
- ✅ phase2 完成：微内核和 SDK 基础
- ✅ phase3 完成：核心库（log、config、healthcheck、metrics）

---

## 2. 技术实现方案

### 2.1 核心选型
- 生命周期状态机：枚举状态转换
- 配置管理：idcu-config
- 日志：idcu-log
- 健康检查：idcu-healthcheck
- 指标：idcu-metrics

### 2.2 核心逻辑
```
1. 初始化核心模块配置
2. 创建模块实例
3. 加载配置文件
4. 初始化日志系统
5. 初始化健康检查
6. 初始化指标收集
7. 注册信号处理器
8. 启动模块
9. 管理模块生命周期
10. 优雅关闭
```

### 2.3 数据结构/接口
```c
typedef struct {
    char name[128];
    char version[64];
    char config_path[1024];
    char log_path[1024];
    idcu_LogLevel log_level;
    int enable_healthcheck;
    int enable_metrics;
    int enable_signals;
    uint64_t graceful_shutdown_timeout_ms;
} idcu_CoreModuleConfig;

typedef struct {
    idcu_CoreModuleId id;
    idcu_CoreModuleState state;
    idcu_ConfigManager config_manager;
    idcu_Logger logger;
    idcu_HealthCheck health_check;
    idcu_MetricsRegistry metrics_registry;
    // ... 其他字段
} idcu_CoreModule;

int idcu_core_module_init(idcu_CoreModule* module, const idcu_CoreModuleConfig* config);
int idcu_core_module_start(idcu_CoreModule* module);
int idcu_core_module_stop(idcu_CoreModule* module);
void idcu_core_module_destroy(idcu_CoreModule* module);
```

### 2.4 跨平台适配
- Windows：使用 SetConsoleCtrlHandler 处理信号
- Linux：使用 sigaction 处理信号
- 统一的模块生命周期接口

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 模块可以正常初始化和启动
- [ ] 配置加载和保存正常工作
- [ ] 日志系统正常工作
- [ ] 健康检查集成正常
- [ ] 指标收集集成正常
- [ ] 信号处理正常工作
- [ ] 优雅关闭功能正常

### 3.2 性能验收
- [ ] 模块初始化时间 ≤ 100ms
- [ ] 配置加载时间 ≤ 50ms
- [ ] 内存占用 ≤ 1MB
- [ ] 信号响应时间 ≤ 10ms

### 3.3 异常验收
- [ ] 配置文件不存在时有明确错误提示
- [ ] 信号处理失败不影响主流程
- [ ] 模块可以从错误状态恢复
- [ ] 优雅关闭超时后可以强制退出

---

## 4. 执行计划

### 4.1 工期
4 小时

### 4.2 里程碑
- D1：完成核心模块头文件和接口定义
- D1：完成核心模块实现
- D1：完成模块管理器实现
- D1：完成测试和验证

### 4.3 人力
1 人（技能要求：C语言 + 系统编程）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐 .clang-format 规范
- 函数名小写+下划线
- 结构体前缀 idcu_

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 测试覆盖所有生命周期状态
- 测试覆盖信号处理场景

### 5.3 部署指引
- 编译命令：cmake --build build
- 模块路径：modules/core-module/
- 配置文件：config/app.yaml

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：跨平台信号处理差异导致兼容性问题  
应对：分别实现 Windows 和 Linux 的信号处理，使用统一的抽象接口

### 6.2 风险2
描述：模块依赖关系复杂导致初始化顺序问题  
应对：明确模块依赖关系，实现依赖排序算法

---

## 7. 详细实现步骤

### 1. 创建目录结构
```bash
mkdir -p modules/core-module/include/idcu/core_module
mkdir -p modules/core-module/src/idcu/core_module
mkdir -p modules/core-module/tests
mkdir -p modules/core-module/examples
```

### 2. 创建核心模块头文件 (core_module.h)
创建 `modules/core-module/include/idcu/core_module/core_module.h`，包含：
- 模块状态枚举定义
- 信号类型枚举定义
- 核心模块配置结构
- 核心模块结构
- 模块管理器结构
- 所有 API 函数声明

### 3. 创建 CMakeLists.txt
创建 `modules/core-module/CMakeLists.txt`，包含：
- 源文件列表
- 依赖库链接
- 测试配置

### 4. 创建模块配置文件 (module.yaml)
创建 `modules/core-module/module.yaml`，包含模块元数据

### 5. 创建 README.md
创建 `modules/core-module/README.md`，包含使用示例

### 6. 实现核心模块功能
实现以下核心功能：
- 模块配置初始化
- 模块初始化和销毁
- 模块启动和停止
- 配置加载和管理
- 日志初始化
- 健康检查集成
- 指标收集集成
- 信号处理
- 优雅关闭
- 模块管理器功能

---

## 8. 验证检查清单

- [ ] 核心模块头文件已创建
- [ ] 核心模块实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 模块可以正常初始化和启动
- [ ] 配置加载正常工作
- [ ] 日志正常工作
- [ ] 单元测试通过

---

## 9. Git 提交

```bash
git add modules/core-module/
git commit -m "feat: add core-module module

- Add module initialization and cleanup
- Add config loading and management
- Add logging initialization
- Add module lifecycle management
- Add module dependency management
- Add health check integration
- Add metrics collection integration
- Add graceful shutdown
- Add signal handling
- Add module manager
- Add CMake build configuration
- Add module.yaml metadata"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 模块启动失败 | 配置错误 | 检查配置文件 |
| 健康检查失败 | 检查函数错误 | 检查健康检查实现 |
| 信号未处理 | 信号处理器未设置 | 确保设置了信号处理器 |
