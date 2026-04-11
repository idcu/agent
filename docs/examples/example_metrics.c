#include <idcu/metrics/metrics.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("=== IDCU Metrics Example ===\n\n");
    
    idcu_MetricsRegistry registry;
    
    // Initialize registry
    int ret = idcu_metrics_registry_init(&registry);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init metrics registry: %s\n", idcu_error_message(ret));
        return 1;
    }
    
    // Example 1: Counter
    printf("Example 1: Counter\n");
    idcu_MetricCounter* requests = idcu_metrics_counter_create(
        "http_requests_total", 
        "Total number of HTTP requests",
        NULL
    );
    
    idcu_metrics_registry_register_counter(&registry, requests);
    
    idcu_metrics_counter_inc(requests);
    idcu_metrics_counter_inc(requests);
    idcu_metrics_counter_add(requests, 5);
    
    uint64_t request_count = idcu_metrics_counter_get(requests);
    printf("Total requests: %" PRIu64 "\n\n", request_count);
    
    // Example 2: Gauge
    printf("Example 2: Gauge\n");
    idcu_MetricGauge* memory = idcu_metrics_gauge_create(
        "memory_usage_bytes", 
        "Current memory usage in bytes",
        NULL
    );
    
    idcu_metrics_registry_register_gauge(&registry, memory);
    
    idcu_metrics_gauge_set(memory, 1024 * 1024);
    printf("Memory: %.0f bytes\n", idcu_metrics_gauge_get(memory));
    
    idcu_metrics_gauge_inc(memory);
    idcu_metrics_gauge_add(memory, 500000);
    printf("Memory after inc/add: %.0f bytes\n", idcu_metrics_gauge_get(memory));
    
    idcu_metrics_gauge_sub(memory, 250000);
    printf("Memory after sub: %.0f bytes\n\n", idcu_metrics_gauge_get(memory));
    
    // Example 3: Histogram
    printf("Example 3: Histogram\n");
    double buckets[] = {0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1, 2.5, 5, 10};
    
    idcu_MetricHistogram* response_time = idcu_metrics_histogram_create(
        "http_request_duration_seconds", 
        "HTTP request duration in seconds",
        NULL,
        buckets,
        sizeof(buckets) / sizeof(buckets[0])
    );
    
    idcu_metrics_registry_register_histogram(&registry, response_time);
    
    idcu_metrics_histogram_observe(response_time, 0.042);
    idcu_metrics_histogram_observe(response_time, 0.123);
    idcu_metrics_histogram_observe(response_time, 0.567);
    idcu_metrics_histogram_observe(response_time, 1.234);
    idcu_metrics_histogram_observe(response_time, 0.089);
    
    printf("Recorded 5 response time observations\n\n");
    
    // Example 4: Export to Prometheus
    printf("Example 4: Prometheus Export\n");
    printf("--- Prometheus Format ---\n");
    char prometheus[8192];
    ret = idcu_metrics_to_prometheus(&registry, prometheus, sizeof(prometheus));
    if (ret == IDCU_ERR_OK) {
        printf("%s", prometheus);
    }
    printf("--- End of Prometheus Format ---\n\n");
    
    // Example 5: Export to JSON
    printf("Example 5: JSON Export\n");
    printf("--- JSON Format ---\n");
    char json[8192];
    ret = idcu_metrics_to_json(&registry, json, sizeof(json));
    if (ret == IDCU_ERR_OK) {
        printf("%s", json);
    }
    printf("--- End of JSON Format ---\n\n");
    
    // Cleanup
    idcu_metrics_registry_destroy(&registry);
    
    printf("=== Metrics Example Complete ===\n");
    return 0;
}
