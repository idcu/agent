# idcu-metrics

IDCU 项目的独立指标库，提供指标收集和 Prometheus 导出功能。

## 特性

- 跨平台支持（Windows、Linux、macOS）
- 支持多种指标类型：Counter、Gauge、Histogram
- Prometheus 格式导出
- 内置 HTTP 服务器提供 /metrics 端点
- 线程安全的指标操作
- 全局指标收集器支持

## 快速开始

### 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 安装

```bash
cmake --install .
```

### 在其他项目中使用

```cmake
find_package(idcu-common REQUIRED)
find_package(idcu-log REQUIRED)
find_package(idcu-network REQUIRED)
find_package(idcu-metrics REQUIRED)
add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE idcu::metrics)
```

## API 文档

### 初始化和清理

```c
#include <idcu/metrics/metrics.h>

// 初始化指标收集器
int idcu_metrics_init(idcu_MetricsCollector* collector);

// 销毁指标收集器
void idcu_metrics_destroy(idcu_MetricsCollector* collector);
```

### 全局指标收集器

```c
// 初始化全局指标收集器
int idcu_global_metrics_init(void);

// 销毁全局指标收集器
void idcu_global_metrics_destroy(void);

// 获取全局指标收集器
idcu_MetricsCollector* idcu_global_metrics_collector(void);
```

### 指标注册

```c
// 注册指标
int idcu_metrics_register(idcu_MetricsCollector* collector, 
                          const char* name, 
                          const char* desc, 
                          idcu_MetricType type);
```

### 指标操作

```c
// 增加 Counter 类型指标
int idcu_metrics_inc(idcu_MetricsCollector* collector, const char* name, uint64_t value);

// 设置 Gauge 类型指标
int idcu_metrics_set(idcu_MetricsCollector* collector, const char* name, uint64_t value);

// 观察 Histogram 类型指标
int idcu_metrics_observe(idcu_MetricsCollector* collector, const char* name, uint64_t value);

// 获取指标值
uint64_t idcu_metrics_get(idcu_MetricsCollector* collector, const char* name);
```

### 指标导出

```c
// 导出为文本格式
int idcu_metrics_export_text(idcu_MetricsCollector* collector, idcu_StringBuf* buf);

// 导出为 Prometheus 格式
int idcu_metrics_export_prometheus(idcu_MetricsCollector* collector, idcu_StringBuf* buf);
```

### Prometheus 导出器

```c
#include <idcu/metrics/prometheus_exporter.h>

// 初始化 Prometheus 导出器
int idcu_prometheus_exporter_init(idcu_PrometheusExporter* exporter,
                                    idcu_MetricsCollector* metrics,
                                    const char* bind_address,
                                    uint16_t port);

// 销毁 Prometheus 导出器
void idcu_prometheus_exporter_destroy(idcu_PrometheusExporter* exporter);

// 启动 Prometheus 导出器
int idcu_prometheus_exporter_start(idcu_PrometheusExporter* exporter);

// 停止 Prometheus 导出器
int idcu_prometheus_exporter_stop(idcu_PrometheusExporter* exporter);

// 轮询处理请求
int idcu_prometheus_exporter_poll(idcu_PrometheusExporter* exporter, int timeout_ms);
```

### 指标类型

```c
typedef enum {
    IDCU_METRIC_COUNTER = 0,  // 计数器，只增不减
    IDCU_METRIC_GAUGE,        // 仪表盘，可增可减
    IDCU_METRIC_HISTOGRAM      // 直方图，统计分布
} idcu_MetricType;
```

## 测试

```bash
cd build
ctest
```

## 示例

库提供了以下示例代码：

- [简单指标示例](examples/example_basic.c) - 演示指标的基本使用
- [Prometheus 导出示例](examples/example_prometheus.c) - 演示如何启动 Prometheus 导出器

### 编译和运行示例

```bash
cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build .

# 运行简单指标示例
./example_basic

# 运行 Prometheus 导出示例
./example_prometheus
```

## 依赖

- idcu-common - IDCU 基础通用组件库
- idcu-log - IDCU 日志库
- idcu-network - IDCU 网络库

## 许可证

详见项目根目录的 LICENSE 文件。
