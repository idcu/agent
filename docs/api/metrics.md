# idcu-metrics API 文档

指标收集库。

## 快速开始

```c
#include <idcu/metrics/metrics.h>

int main() {
    idcu_MetricsRegistry* registry = NULL;
    idcu_metrics_registry_init(&registry);
    
    // 创建和使用指标
    idcu_Metric* requests = idcu_metrics_counter_create(registry, "http_requests_total", "总 HTTP 请求数");
    idcu_metrics_counter_inc(requests);
    
    idcu_Metric* memory = idcu_metrics_gauge_create(registry, "memory_usage_bytes", "当前内存使用量");
    idcu_metrics_gauge_set(memory, 1024 * 1024);
    
    // 导出指标
    char* prometheus = idcu_metrics_export_prometheus(registry);
    printf("%s\n", prometheus);
    free(prometheus);
    
    idcu_metrics_registry_destroy(registry);
    return 0;
}
```

## 注册表

### 注册表类型

```c
typedef struct idcu_MetricsRegistry idcu_MetricsRegistry;
```

### 注册表函数

```c
idcu_ErrorCode idcu_metrics_registry_init(idcu_MetricsRegistry** registry);
void idcu_metrics_registry_destroy(idcu_MetricsRegistry* registry);
idcu_ErrorCode idcu_metrics_registry_register(idcu_MetricsRegistry* registry, idcu_Metric* metric);
idcu_ErrorCode idcu_metrics_registry_unregister(idcu_MetricsRegistry* registry, const char* name);
```

## 指标类型

### 指标类型

```c
typedef struct idcu_Metric idcu_Metric;
```

## 计数器

计数器是一种累积指标，表示一个单调递增的计数器，其值只能增加或在重启时重置为零。

### 创建计数器

```c
idcu_Metric* idcu_metrics_counter_create(idcu_MetricsRegistry* registry, const char* name, const char* help);
```

创建一个计数器指标。

### 计数器操作

```c
void idcu_metrics_counter_inc(idcu_Metric* metric);
void idcu_metrics_counter_add(idcu_Metric* metric, double value);
double idcu_metrics_counter_get(idcu_Metric* metric);
void idcu_metrics_counter_reset(idcu_Metric* metric);
```

### 带标签的计数器

```c
idcu_Metric* idcu_metrics_counter_create_with_labels(idcu_MetricsRegistry* registry, const char* name, const char* help, const char** label_names, size_t label_count);
void idcu_metrics_counter_inc_with_labels(idcu_Metric* metric, const char** label_values, size_t label_count);
void idcu_metrics_counter_add_with_labels(idcu_Metric* metric, double value, const char** label_values, size_t label_count);
```

## 仪表盘

仪表盘表示一个可以任意上下变动的数值。

### 创建仪表盘

```c
idcu_Metric* idcu_metrics_gauge_create(idcu_MetricsRegistry* registry, const char* name, const char* help);
```

创建一个仪表盘指标。

### 仪表盘操作

```c
void idcu_metrics_gauge_set(idcu_Metric* metric, double value);
void idcu_metrics_gauge_inc(idcu_Metric* metric);
void idcu_metrics_gauge_dec(idcu_Metric* metric);
void idcu_metrics_gauge_add(idcu_Metric* metric, double value);
void idcu_metrics_gauge_sub(idcu_Metric* metric, double value);
double idcu_metrics_gauge_get(idcu_Metric* metric);
```

### 带标签的仪表盘

```c
idcu_Metric* idcu_metrics_gauge_create_with_labels(idcu_MetricsRegistry* registry, const char* name, const char* help, const char** label_names, size_t label_count);
void idcu_metrics_gauge_set_with_labels(idcu_Metric* metric, double value, const char** label_values, size_t label_count);
```

## 直方图

直方图对观察值（通常是请求持续时间或响应大小等内容）进行采样，并将它们计数到可配置的桶中。

### 创建直方图

```c
idcu_Metric* idcu_metrics_histogram_create(idcu_MetricsRegistry* registry, const char* name, const char* help, const double* buckets, size_t bucket_count);
```

创建一个直方图指标。

### 直方图操作

```c
void idcu_metrics_histogram_observe(idcu_Metric* metric, double value);
```

### 带标签的直方图

```c
idcu_Metric* idcu_metrics_histogram_create_with_labels(idcu_MetricsRegistry* registry, const char* name, const char* help, const double* buckets, size_t bucket_count, const char** label_names, size_t label_count);
void idcu_metrics_histogram_observe_with_labels(idcu_Metric* metric, double value, const char** label_values, size_t label_count);
```

## 导出

### Prometheus 格式

```c
char* idcu_metrics_export_prometheus(idcu_MetricsRegistry* registry);
```

以 Prometheus 格式导出指标。返回一个必须释放的字符串。

### JSON 格式

```c
char* idcu_metrics_export_json(idcu_MetricsRegistry* registry);
```

以 JSON 格式导出指标。返回一个必须释放的字符串。

## 示例

### 基本计数器

```c
idcu_MetricsRegistry* registry = NULL;
idcu_metrics_registry_init(&registry);

idcu_Metric* requests = idcu_metrics_counter_create(registry, "http_requests_total", "总 HTTP 请求数");

idcu_metrics_counter_inc(requests);
idcu_metrics_counter_inc(requests);
idcu_metrics_counter_add(requests, 5);

double value = idcu_metrics_counter_get(requests);
printf("请求数: %.0f\n", value); // 7

idcu_metrics_registry_destroy(registry);
```

### 内存使用仪表盘

```c
idcu_Metric* memory = idcu_metrics_gauge_create(registry, "memory_usage_bytes", "当前内存使用量");

idcu_metrics_gauge_set(memory, 1000);
idcu_metrics_gauge_inc(memory);       // 1001
idcu_metrics_gauge_add(memory, 500);   // 1501
idcu_metrics_gauge_dec(memory);       // 1500
idcu_metrics_gauge_sub(memory, 200);   // 1300
```

### 响应时间直方图

```c
double buckets[] = {0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1, 2.5, 5, 10};

idcu_Metric* response_time = idcu_metrics_histogram_create(
    registry, 
    "http_request_duration_seconds", 
    "HTTP 请求持续时间",
    buckets,
    sizeof(buckets) / sizeof(buckets[0])
);

idcu_metrics_histogram_observe(response_time, 0.042);
idcu_metrics_histogram_observe(response_time, 0.123);
idcu_metrics_histogram_observe(response_time, 0.567);
```

### 带标签的指标

```c
const char* label_names[] = {"method", "status"};
idcu_Metric* requests = idcu_metrics_counter_create_with_labels(
    registry,
    "http_requests_total",
    "总 HTTP 请求数",
    label_names,
    2
);

const char* get_200[] = {"GET", "200"};
const char* post_500[] = {"POST", "500"};

idcu_metrics_counter_inc_with_labels(requests, get_200, 2);
idcu_metrics_counter_inc_with_labels(requests, post_500, 2);
```

### 导出到 Prometheus

```c
char* prom_output = idcu_metrics_export_prometheus(registry);
printf("%s", prom_output);

// 输出将如下所示：
// # HELP http_requests_total 总 HTTP 请求数
// # TYPE http_requests_total counter
// http_requests_total 7
// # HELP memory_usage_bytes 当前内存使用量
// # TYPE memory_usage_bytes gauge
// memory_usage_bytes 1300

free(prom_output);
```

### 导出到 JSON

```c
char* json_output = idcu_metrics_export_json(registry);
printf("%s", json_output);
free(json_output);
```

### 完整 Web 服务器示例

```c
void metrics_handler(idcu_HttpRequest* req, idcu_HttpResponse* res, void* user_data) {
    idcu_MetricsRegistry* registry = (idcu_MetricsRegistry*)user_data;
    
    char* prometheus = idcu_metrics_export_prometheus(registry);
    idcu_http_response_set_content_type(res, "text/plain; version=0.0.4");
    idcu_http_response_write_string(res, prometheus);
    
    free(prometheus);
}

// 添加路由: /metrics
idcu_http_server_get(server, "/metrics", metrics_handler, registry);
```
