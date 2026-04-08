# 任务 4.6: metrics-integration - 指标集成模块

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-08  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建 metrics-integration 集成模块，将 idcu-metrics 集成到微内核架构中，支持：
- 统一的指标接口
- 指标收集、导出、聚合
- 指标告警和持久化
- Prometheus 格式支持
- 指标更新延迟 ≤ 10ms，支持 10000 指标/秒

### 1.2 不做什么
- 不修改 idcu-metrics 独立库的核心代码
- 不实现 Grafana 集成
- 不实现指标可视化

### 1.3 输入
- idcu-metrics 独立库
- idcu-alert、idcu-storage 独立库
- SDK 基础
- YAML 配置文件

### 1.4 输出
- metrics-integration 集成模块
- 可以注册、更新、查询指标
- 支持 Prometheus 格式导出

### 1.5 前置依赖
- phase3 15_idcu_metrics.md 任务已完成
- phase2 SDK 基础已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- 指标库：idcu-metrics
- 告警：idcu-alert
- 存储：idcu-storage
- 导出格式：Prometheus
- 构建系统：idcu-module-build

### 2.2 核心逻辑
1. 创建 metrics-integration 目录结构
2. 封装 idcu-metrics 接口
3. 实现指标注册和更新
4. 实现指标告警规则
5. 实现指标持久化
6. 实现 Prometheus 格式导出

### 2.3 数据结构/接口
```c
typedef struct {
    char name[128];
    char metric_name[128];
    double threshold;
    int operator_type;
    char alert_topic[256];
    int enabled;
} idcu_MetricsAlertRule;

typedef struct {
    idcu_MetricsRegistry registry;
    idcu_Vector alert_rules;
    idcu_Mutex lock;
    idcu_AlertManager* alert_manager;
    idcu_KVStore* storage;
} idcu_MetricsIntegration;
```

### 2.4 跨平台适配
- 无特殊跨平台差异

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以创建和更新指标
- [ ] Prometheus 格式导出正常工作
- [ ]