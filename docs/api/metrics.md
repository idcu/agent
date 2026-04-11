# idcu-metrics API Documentation

Metrics collection library.

## Quick Start

```c
#include <idcu/metrics/metrics.h>

int main() {
    idcu_MetricsRegistry* registry = NULL;
    idcu_metrics_registry_init(&registry);
    
    // Create and use metrics
    idcu_Metric* requests = idcu_metrics_counter_create(registry, "http_requests_total", "Total HTTP requests");
    idcu_metrics_counter_inc(requests);
    
    idcu_Metric* memory = idcu_metrics_gauge_create(registry, "memory_usage_bytes", "Current memory usage");
    idcu_metrics_gauge_set(memory, 1024 * 1024);
    
    // Export metrics
    char* prometheus = idcu_metrics_export_prometheus(registry);
    printf("%s\n", prometheus);
    free(prometheus);
    
    idcu_metrics_registry_destroy(registry);
    return 0;
}
```

## Registry

### Registry Type

```c
typedef struct idcu_MetricsRegistry idcu_MetricsRegistry;
```

### Registry Functions

```c
idcu_ErrorCode idcu_metrics_registry_init(idcu_MetricsRegistry** registry);
void idcu_metrics_registry_destroy(idcu_MetricsRegistry* registry);
idcu_ErrorCode idcu_metrics_registry_register(idcu_MetricsRegistry* registry, idcu_Metric* metric);
idcu_ErrorCode idcu_metrics_registry_unregister(idcu_MetricsRegistry* registry, const char* name);
```

## Metric Types

### Metric Type

```c
typedef struct idcu_Metric idcu_Metric;
```

## Counter

A counter is a cumulative metric that represents a single monotonically increasing counter whose value can only increase or be reset to zero on restart.

### Create Counter

```c
idcu_Metric* idcu_metrics_counter_create(idcu_MetricsRegistry* registry, const char* name, const char* help);
```

Create a counter metric.

### Counter Operations

```c
void idcu_metrics_counter_inc(idcu_Metric* metric);
void idcu_metrics_counter_add(idcu_Metric* metric, double value);
double idcu_metrics_counter_get(idcu_Metric* metric);
void idcu_metrics_counter_reset(idcu_Metric* metric);
```

### Counter with Labels

```c
idcu_Metric* idcu_metrics_counter_create_with_labels(idcu_MetricsRegistry* registry, const char* name, const char* help, const char** label_names, size_t label_count);
void idcu_metrics_counter_inc_with_labels(idcu_Metric* metric, const char** label_values, size_t label_count);
void idcu_metrics_counter_add_with_labels(idcu_Metric* metric, double value, const char** label_values, size_t label_count);
```

## Gauge

A gauge represents a single numerical value that can arbitrarily go up and down.

### Create Gauge

```c
idcu_Metric* idcu_metrics_gauge_create(idcu_MetricsRegistry* registry, const char* name, const char* help);
```

Create a gauge metric.

### Gauge Operations

```c
void idcu_metrics_gauge_set(idcu_Metric* metric, double value);
void idcu_metrics_gauge_inc(idcu_Metric* metric);
void idcu_metrics_gauge_dec(idcu_Metric* metric);
void idcu_metrics_gauge_add(idcu_Metric* metric, double value);
void idcu_metrics_gauge_sub(idcu_Metric* metric, double value);
double idcu_metrics_gauge_get(idcu_Metric* metric);
```

### Gauge with Labels

```c
idcu_Metric* idcu_metrics_gauge_create_with_labels(idcu_MetricsRegistry* registry, const char* name, const char* help, const char** label_names, size_t label_count);
void idcu_metrics_gauge_set_with_labels(idcu_Metric* metric, double value, const char** label_values, size_t label_count);
```

## Histogram

A histogram samples observations (usually things like request durations or response sizes) and counts them in configurable buckets.

### Create Histogram

```c
idcu_Metric* idcu_metrics_histogram_create(idcu_MetricsRegistry* registry, const char* name, const char* help, const double* buckets, size_t bucket_count);
```

Create a histogram metric.

### Histogram Operations

```c
void idcu_metrics_histogram_observe(idcu_Metric* metric, double value);
```

### Histogram with Labels

```c
idcu_Metric* idcu_metrics_histogram_create_with_labels(idcu_MetricsRegistry* registry, const char* name, const char* help, const double* buckets, size_t bucket_count, const char** label_names, size_t label_count);
void idcu_metrics_histogram_observe_with_labels(idcu_Metric* metric, double value, const char** label_values, size_t label_count);
```

## Export

### Prometheus Format

```c
char* idcu_metrics_export_prometheus(idcu_MetricsRegistry* registry);
```

Export metrics in Prometheus format. Returns a string that must be freed.

### JSON Format

```c
char* idcu_metrics_export_json(idcu_MetricsRegistry* registry);
```

Export metrics in JSON format. Returns a string that must be freed.

## Examples

### Basic Counter

```c
idcu_MetricsRegistry* registry = NULL;
idcu_metrics_registry_init(&registry);

idcu_Metric* requests = idcu_metrics_counter_create(registry, "http_requests_total", "Total HTTP requests");

idcu_metrics_counter_inc(requests);
idcu_metrics_counter_inc(requests);
idcu_metrics_counter_add(requests, 5);

double value = idcu_metrics_counter_get(requests);
printf("Requests: %.0f\n", value); // 7

idcu_metrics_registry_destroy(registry);
```

### Gauge for Memory Usage

```c
idcu_Metric* memory = idcu_metrics_gauge_create(registry, "memory_usage_bytes", "Current memory usage");

idcu_metrics_gauge_set(memory, 1000);
idcu_metrics_gauge_inc(memory);       // 1001
idcu_metrics_gauge_add(memory, 500);   // 1501
idcu_metrics_gauge_dec(memory);       // 1500
idcu_metrics_gauge_sub(memory, 200);   // 1300
```

### Histogram for Response Times

```c
double buckets[] = {0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1, 2.5, 5, 10};

idcu_Metric* response_time = idcu_metrics_histogram_create(
    registry, 
    "http_request_duration_seconds", 
    "HTTP request duration",
    buckets,
    sizeof(buckets) / sizeof(buckets[0])
);

idcu_metrics_histogram_observe(response_time, 0.042);
idcu_metrics_histogram_observe(response_time, 0.123);
idcu_metrics_histogram_observe(response_time, 0.567);
```

### Metrics with Labels

```c
const char* label_names[] = {"method", "status"};
idcu_Metric* requests = idcu_metrics_counter_create_with_labels(
    registry,
    "http_requests_total",
    "Total HTTP requests",
    label_names,
    2
);

const char* get_200[] = {"GET", "200"};
const char* post_500[] = {"POST", "500"};

idcu_metrics_counter_inc_with_labels(requests, get_200, 2);
idcu_metrics_counter_inc_with_labels(requests, post_500, 2);
```

### Export to Prometheus

```c
char* prom_output = idcu_metrics_export_prometheus(registry);
printf("%s", prom_output);

// Output would look like:
// # HELP http_requests_total Total HTTP requests
// # TYPE http_requests_total counter
// http_requests_total 7
// # HELP memory_usage_bytes Current memory usage
// # TYPE memory_usage_bytes gauge
// memory_usage_bytes 1300

free(prom_output);
```

### Export to JSON

```c
char* json_output = idcu_metrics_export_json(registry);
printf("%s", json_output);
free(json_output);
```

### Complete Web Server Example

```c
void metrics_handler(idcu_HttpRequest* req, idcu_HttpResponse* res, void* user_data) {
    idcu_MetricsRegistry* registry = (idcu_MetricsRegistry*)user_data;
    
    char* prometheus = idcu_metrics_export_prometheus(registry);
    idcu_http_response_set_content_type(res, "text/plain; version=0.0.4");
    idcu_http_response_write_string(res, prometheus);
    
    free(prometheus);
}

// Add route: /metrics
idcu_http_server_get(server, "/metrics", metrics_handler, registry);
```
