# 任务 3.27: idcu-server-monitor - 服务器监控

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-09  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的服务器监控库，支持 CPU 使用率监控、内存使用率监控、磁盘使用率监控、网络流量监控、进程监控、系统负载监控、告警规则、数据记录和历史查询，满足监控数据采集延迟 ≤ 500ms、告警响应时间 ≤ 1s、历史数据保留 ≥ 7 天的性能要求。

### 1.2 不做什么
- 不实现日志收集（已有 idcu-log）
- 不实现分布式监控
- 不实现可视化 UI
- 不实现告警通知发送（邮件、短信等）

### 1.3 输入
- 监控配置（采集间隔、历史保留时间）
- 告警规则（指标、阈值、持续时间、级别）
- 查询参数（时间范围、指标名称）

### 1.4 输出
- 实时监控数据（CPU、内存、磁盘、网络、进程、负载）
- 告警记录（级别、时间、消息）
- 历史数据查询结果
- 返回码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 库已实现
- idcu-metrics 库已实现
- idcu-alert 库已实现
- idcu-storage 库已实现
- phase3 前 26 个任务已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **系统信息**: Windows 使用 WMI/Performance Counters，Linux 使用 /proc 文件系统
- **告警规则**: 基于阈值 + 持续时间
- **数据存储**: 内存缓存 + idcu-storage 持久化
- **数据导出**: JSON、Prometheus 格式

### 2.2 核心逻辑
```
监控器工作流程：
1. 初始化监控器，加载配置
2. 注册告警规则
3. 启动定时采集
4. 采集系统指标（CPU、内存、磁盘、网络、进程、负载）
5. 更新 metrics 数据
6. 检查告警规则
7. 触发告警（如需要）
8. 存储历史数据
9. 提供查询接口

告警检查流程：
1. 获取当前指标值
2. 与阈值比较
3. 检查持续时间
4. 连续违规次数达标则触发告警
5. 更新告警状态
```

### 2.3 数据结构/接口
```c
typedef struct {
    double user;
    double system;
    double idle;
    uint64_t timestamp;
} idcu_CpuStats;

typedef struct {
    uint64_t total;
    uint64_t used;
    uint64_t free;
    uint64_t timestamp;
} idcu_MemoryStats;

typedef struct {
    char mount_point[256];
    uint64_t total;
    uint64_t used;
    double usage_percent;
    uint64_t timestamp;
} idcu_DiskStats;

typedef enum {
    IDCU_ALERT_LEVEL_INFO = 0,
    IDCU_ALERT_LEVEL_WARNING,
    IDCU_ALERT_LEVEL_CRITICAL,
    IDCU_ALERT_LEVEL_FATAL
} idcu_AlertLevel;

int  idcu_server_monitor_init(idcu_ServerMonitor* monitor, const idcu_ServerMonitorConfig* config);
void idcu_server_monitor_destroy(idcu_ServerMonitor* monitor);
int  idcu_server_monitor_start(idcu_ServerMonitor* monitor);
int  idcu_server_monitor_stop(idcu_ServerMonitor* monitor);
int  idcu_server_monitor_get_cpu(idcu_ServerMonitor* monitor, idcu_CpuStats* stats);
int  idcu_server_monitor_get_memory(idcu_ServerMonitor* monitor, idcu_MemoryStats* stats);
int  idcu_server_monitor_add_alert_rule(idcu_ServerMonitor* monitor, const idcu_AlertRule* rule);
int  idcu_server_monitor_get_active_alerts(idcu_ServerMonitor* monitor, idcu_Vector* alerts);
```

### 2.4 跨平台适配
- **Windows**: 使用 WMI 或 Performance Counters 获取系统信息
- **Linux**: 使用 /proc、/sys 文件系统获取系统信息
- **统一接口**: 提供统一的 API 屏蔽平台差异

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以获取 CPU 使用率
- [ ] 可以获取内存使用率
- [ ] 可以获取磁盘使用率
- [ ] 可以获取网络流量
- [ ] 可以获取进程信息
- [ ] 可以获取系统负载
- [ ] 告警规则正常工作
- [ ] 历史数据查询正常

### 3.2 性能验收
- 监控数据采集延迟 ≤ 500ms
- 告警响应时间 ≤ 1s
- 历史数据保留 ≥ 7 天
- 支持 100+ 告警规则
- 内存占用 ≤ 20MB

### 3.3 异常验收
- [ ] 权限不足时返回明确错误
- [ ] 系统信息不可用时降级处理
- [ ] 告警规则配置错误返回明确错误
- [ ] 多线程并发查询无数据竞争

---

## 4. 执行计划

### 4.1 工期
2 天/人

### 4.2 里程碑
- D1：完成接口定义、头文件、CMakeLists.txt、module.yaml、README.md
- D2：完成核心监控逻辑、告警规则、单元测试

### 4.3 人力
1 人（技能要求：C 语言 + 系统编程）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名：idcu_server_monitor_* 小写加下划线

### 5.2 测试要求
- 单元测试覆盖率 ≥ 70%
- 测试覆盖：各指标采集、告警规则、历史查询

### 5.3 部署指引
- 编译命令：`cmake -B build &amp;&amp; cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::server-monitor)`

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险 1
描述：跨平台系统信息获取差异大  
应对：封装平台特定实现，提供统一接口

### 6.2 风险 2
描述：历史数据占用过多磁盘空间  
应对：配置保留时间，自动过期旧数据

---

## 7. 详细实现步骤

### 7.1 创建目录结构

```bash
mkdir -p libs/idcu-server-monitor/include/idcu/server_monitor
mkdir -p libs/idcu-server-monitor/src/idcu/server_monitor
mkdir -p libs/idcu-server-monitor/tests
mkdir -p libs/idcu-server-monitor/examples
```

### 7.2 创建服务器监控头文件 (server_monitor.h)

（保留原有的详细代码实现）

---

## 8. 验证检查清单

- [ ] 服务器监控头文件已创建
- [ ] 服务器监控实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以获取系统监控数据
- [ ] 告警规则正常工作
- [ ] 数据导出功能正常

---

## 9. Git 提交

```bash
git add libs/idcu-server-monitor/
git commit -m "feat: add idcu-server-monitor library

- Add CPU usage monitoring
- Add memory usage monitoring
- Add disk usage monitoring
- Add network traffic monitoring
- Add process monitoring
- Add system load monitoring
- Add alert rules
- Add data recording and historical queries
- Add CMake build configuration
- Add module.yaml metadata"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 无法获取系统信息 | 权限不足 | 确保程序有足够权限 |
| 告警不触发 | 规则配置错误 | 检查告警规则配置 |
| 历史数据丢失 | 存储配置错误 | 检查存储路径配置 |
