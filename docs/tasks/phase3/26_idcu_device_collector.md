# 任务 3.26: idcu-device-collector - 设备数据采集

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-09  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的设备数据采集库，支持多种设备类型（服务器、网络设备、IoT设备）、多种采集协议（SNMP、HTTP、Modbus、自定义）、数据收集和聚合、数据过滤和转换、定时采集、数据缓存、数据导出，满足采集延迟 ≤ 1s、支持 100+ 并发设备、数据缓存容量 ≥ 10000 条的性能要求。

### 1.2 不做什么
- 不实现设备控制功能（仅采集）
- 不实现复杂的协议转换
- 不实现数据持久化到数据库
- 不实现实时数据可视化

### 1.3 输入
- 设备配置（名称、类型、地址、协议、采集间隔）
- 指标定义（名称、类型、单位、采集方法）
- 采集规则（过滤条件、转换规则）
- 调度配置（采集间隔）

### 1.4 输出
- 采集的指标数据（值、时间戳）
- 设备状态（在线/离线、连接状态）
- 采集统计（成功/失败次数）
- 返回码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 库已实现
- idcu-scheduler 库已实现
- idcu-storage 库已实现
- idcu-log 库已实现
- phase3 前 25 个任务已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **设备类型**: 枚举类型（SERVER、NETWORK、IOT、CUSTOM）
- **协议支持**: SNMP、HTTP、Modbus、TCP、UDP、自定义
- **数据类型**: Gauge、Counter、Histogram、String、Boolean
- **定时调度**: 使用 idcu-scheduler 库
- **数据缓存**: 内存缓存 + 可选持久化

### 2.2 核心逻辑
```
采集器工作流程：
1. 初始化采集器，加载配置
2. 注册设备，配置采集指标
3. 启动调度器，定时触发采集
4. 连接设备，执行采集
5. 过滤和转换数据
6. 缓存采集结果
7. 提供查询和导出接口

设备采集流程：
1. 检查设备连接状态
2. 建立连接（如需要）
3. 按指标定义采集数据
4. 解析和转换数据
5. 更新设备状态
6. 断开连接（如需要）
```

### 2.3 数据结构/接口
```c
typedef uint64_t idcu_DeviceId;
typedef uint64_t idcu_MetricId;

typedef enum {
    IDCU_DEVICE_TYPE_SERVER = 0,
    IDCU_DEVICE_TYPE_NETWORK,
    IDCU_DEVICE_TYPE_IOT,
    IDCU_DEVICE_TYPE_CUSTOM
} idcu_DeviceType;

typedef enum {
    IDCU_PROTOCOL_SNMP = 0,
    IDCU_PROTOCOL_HTTP,
    IDCU_PROTOCOL_MODBUS,
    IDCU_PROTOCOL_TCP,
    IDCU_PROTOCOL_UDP,
    IDCU_PROTOCOL_CUSTOM
} idcu_ProtocolType;

typedef struct {
    idcu_DeviceId id;
    char name[128];
    idcu_DeviceType type;
    idcu_ProtocolType protocol;
    char address[256];
    uint16_t port;
    uint64_t collect_interval_ms;
    int enabled;
    int connected;
} idcu_Device;

int  idcu_device_collector_init(idcu_DeviceCollector* collector, const idcu_DeviceCollectorConfig* config);
void idcu_device_collector_destroy(idcu_DeviceCollector* collector);
int  idcu_device_collector_start(idcu_DeviceCollector* collector);
int  idcu_device_collector_stop(idcu_DeviceCollector* collector);
int  idcu_device_collector_register_device(idcu_DeviceCollector* collector, const idcu_Device* device);
int  idcu_device_collector_collect_now(idcu_DeviceCollector* collector, idcu_DeviceId device_id);
int  idcu_device_collector_get_metrics(idcu_DeviceCollector* collector, idcu_DeviceId device_id, idcu_Vector* values);
```

### 2.4 跨平台适配
- **网络通信**: 使用 idcu-network 库封装的跨平台 Socket API
- **时间**: 使用 idcu-common 库提供的跨平台时间函数
- **文件操作**: 使用标准 C 库，跨平台兼容

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以注册和管理多种类型设备
- [ ] 支持 SNMP、HTTP、Modbus 等协议
- [ ] 定时采集功能正常工作
- [ ] 数据过滤和转换正常工作
- [ ] 数据缓存功能正常
- [ ] 可以查询和导出采集数据

### 3.2 性能验收
- 单次采集延迟 ≤ 1s
- 支持 100+ 并发设备采集
- 数据缓存容量 ≥ 10000 条
- 数据导出速度 ≥ 1000 条/秒
- 内存占用 ≤ 50MB（100 设备）

### 3.3 异常验收
- [ ] 设备离线后自动重连
- [ ] 采集失败后记录错误并重试
- [ ] 无效设备配置返回明确错误
- [ ] 多线程并发操作无数据竞争

---

## 4. 执行计划

### 4.1 工期
2 天/人

### 4.2 里程碑
- D1：完成接口定义、头文件、CMakeLists.txt、module.yaml、README.md
- D2：完成核心采集逻辑、协议支持、单元测试

### 4.3 人力
1 人（技能要求：C 语言 + 网络编程 + 设备通信）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名：idcu_device_collector_* 小写加下划线

### 5.2 测试要求
- 单元测试覆盖率 ≥ 70%
- 测试覆盖：设备注册、数据采集、缓存、导出

### 5.3 部署指引
- 编译命令：`cmake -B build &amp;&amp; cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::device-collector)`

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险 1
描述：设备协议复杂，实现难度大  
应对：先实现基础协议（HTTP、TCP），其他协议通过自定义接口扩展

### 6.2 风险 2
描述：大量设备并发采集导致性能问题  
应对：使用工作线程池，限制并发数量

---

## 7. 详细实现步骤

### 7.1 创建目录结构

```bash
mkdir -p libs/idcu-device-collector/include/idcu/device_collector
mkdir -p libs/idcu-device-collector/src/idcu/device_collector
mkdir -p libs/idcu-device-collector/tests
mkdir -p libs/idcu-device-collector/examples
```

### 7.2 创建设备采集头文件 (device_collector.h)

（保留原有的详细代码实现）

---

## 8. 验证检查清单

- [ ] 设备采集头文件已创建
- [ ] 设备采集实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以注册和管理设备
- [ ] 数据采集功能正常工作
- [ ] 数据导出功能正常工作

---

## 9. Git 提交

```bash
git add libs/idcu-device-collector/
git commit -m "feat: add idcu-device-collector library

- Add multiple device types support
- Add multiple collection protocols (SNMP, HTTP, Modbus)
- Add data collection and aggregation
- Add data filtering and transformation
- Add scheduled collection
- Add data caching
- Add data export
- Add CMake build configuration
- Add module.yaml metadata"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 设备连接失败 | 网络问题或认证错误 | 检查网络连接和认证信息 |
| 采集超时 | 设备响应慢或间隔太短 | 增加超时时间或采集间隔 |
| 数据不准确 | 协议实现问题 | 检查协议处理代码 |
