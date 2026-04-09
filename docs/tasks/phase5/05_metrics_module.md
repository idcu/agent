# 任务 5.5: metrics-module - 指标业务模块

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建 metrics-module 指标业务模块，将 idcu-metrics 集成到业务模块中，支持：
- 指标收集和聚合
- 指标注册和管理
- 指标导出（Prometheus 格式）
- 指标告警规则
- 性能指标：指标更新延迟 ≤ 10ms，支持 10000 指标/秒

### 1.2 不做什么
- 不实现指标持久化（使用 idcu-storage）
- 不实现指标可视化（Grafana 集成）
- 不实现分布式指标收集

### 1.3 输入
- 指标注册请求
- 指标更新数据
- 告警规则配置
- 指标查询请求

### 1.4 输出
- 指标数据
- 告警通知
- Prometheus 格式导出
- 指标统计信息

### 1.5 前置依赖
- ✅ 5.1 完成：core-module
- phase3 15_idcu_metrics.md 任务已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- 指标库：idcu-metrics
- 告警：idcu-alert
- 导出格式：Prometheus
- 消息总线：idcu-msgbus

### 2.2 核心逻辑
```
1. 初始化指标模块
2. 注册系统指标
3. 启动指标收集协程
4. 处理指标更新
5. 检查告警规则
6. 导出指标数据
7. 发送告警通知
```

### 2.3 数据结构/接口
```c
typedef struct {
    idcu_Module base;
    idcu_MetricsRegistry* registry;
    idcu_Vector* alert_rules;
    idcu_Mutex* lock;
} idcu_MetricsModule;

typedef struct {
    char name[128];
    double threshold;
    int operator_type;
    char alert_topic[256];
    int enabled;
} idcu_MetricsAlertRule;

int idcu_metrics_module_init(idcu_MetricsModule* module);
int idcu_metrics_module_register_counter(idcu_MetricsModule* module, const char* name, const char* help);
int idcu_metrics_module_register_gauge(idcu_MetricsModule* module, const char* name, const char* help);
int idcu_metrics_module_inc(idcu_MetricsModule* module, const char* name, double value);
int idcu_metrics_module_set(idcu_MetricsModule* module, const char* name, double value);
int idcu_metrics_module_export_prometheus(idcu_MetricsModule* module, char* buffer, size_t buffer_size);
```

### 2.4 跨平台适配
- 无特殊跨平台差异
- 使用 idcu-metrics 的跨平台支持

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以注册和更新指标
- [ ] 指标告警规则正常工作
- [ ] Prometheus 格式导出正常
- [ ] 指标查询功能正常

### 3.2 性能验收
- [ ] 指标更新延迟 ≤ 10ms
- [ ] 支持 ≥ 10000 指标/秒
- [ ] 内存占用 ≤ 5MB
- [ ] 导出延迟 ≤ 100ms

### 3.3 异常验收
- [ ] 指标不存在时有明确错误提示
- [ ] 告警规则错误时正确处理
- [ ] 导出缓冲区不足时有错误处理

---

## 4. 执行计划

### 4.1 工期
3 小时

### 4.2 里程碑
- D1：完成指标模块接口定义
- D1：完成指标注册和更新
- D1：完成告警规则
- D1：完成测试和验证

### 4.3 人力
1 人（技能要求：C语言 + 指标系统）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐 .clang-format 规范
- 函数名小写+下划线
- 结构体前缀 idcu_

### 5.2 测试要求
- 单元测试覆盖率 ≥ 70%
- 测试覆盖不同指标类型
- 测试覆盖告警场景

### 5.3 部署指引
- 编译命令：cmake --build build
- 模块路径：modules/business/metrics-module/

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：指标收集影响系统性能  
应对：使用异步收集，限制指标收集频率

### 6.2 风险2
描述：指标内存占用过大  
应对：限制指标数量，定期清理过期指标

### 6.3 风险3
描述：性能不满足预期要求  
应对：进行性能基准测试，优化关键路径代码

### 6.4 风险4
描述：跨平台兼容性问题  
应对：使用跨平台 API，充分测试不同平台

---

## 7. 详细实现步骤

### 7.1 创建目录结构
```bash
mkdir -p modules/business/metrics-module/src
mkdir -p modules/business/metrics-module/include
mkdir -p modules/business/metrics-module/tests
```

### 7.2 实现指标模块
创建 `modules/business/metrics-module/src/metrics_module.c：
```c
#include "idcu/metrics_module/metrics_module.h"
#include "idcu/metrics/metrics.h"
#include "idcu/alert/alert.h"
#include <string.h>

typedef struct {
    idcu_Module base;
    idcu_MetricsRegistry* registry;
    idcu_Vector* alert_rules;
    idcu_Mutex* lock;
} idcu_MetricsModule;

int idcu_metrics_module_init(idcu_MetricsModule* module) {
    // 实现初始化
    return IDCU_ERR_OK;
}
```

### 7.3 创建 CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.15)
project(metrics-module VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_library(metrics-module STATIC src/metrics_module.c)
target_include_directories(metrics-module PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(metrics-module PRIVATE idcu::metrics idcu::alert idcu::common)
```

---

## 8. 验证检查清单

- [ ] 模块可以正常初始化
- [ ] 指标注册和更新功能正常
- [ ] 告警规则正常工作
- [ ] Prometheus 格式导出正常
- [ ] 代码已格式化（clang-format）
- [ ] 静态分析通过（clang-tidy）
- [ ] 符合工程化标准要求
- [ ] 性能测试满足指标要求
- [ ] YAML 配置示例已创建
- [ ] README.md 已创建

---

## 9. Git 提交

```bash
git add modules/business/metrics-module/
git add config/default/metrics_module.yaml
git commit -m "feat(business): add metrics module

- Add metrics business module
- Add metrics registration and update
- Add alert rules
- Add Prometheus export
- Add YAML config example
- Add CMakeLists.txt
- Add README"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 指标未更新 | 模块未初始化 | 确保先初始化 metrics-module |
| 告警未触发 | 规则未启用 | 检查告警规则的 enabled 标志 |
| 导出格式错误 | Prometheus 格式不正确 | 检查导出格式是否符合规范 |
