#include "performance_profiler.h"
#include <idcu/common/hash_map.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

static idcu_HashMap metrics;
static int initialized = 0;

static uint64_t get_time_ns(void) {
#ifdef _WIN32
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (uint64_t)(count.QuadPart * 1000000000ULL / freq.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
#endif
}

int idcu_performance_profiler_init(void) {
    if (initialized) {
        return 0;
    }
    
    idcu_hash_map_init(&metrics, sizeof(idcu_PerformanceMetric));
    initialized = 1;
    return 0;
}

void idcu_performance_profiler_destroy(void) {
    if (!initialized) {
        return;
    }
    
    idcu_performance_profiler_print_report();
    idcu_hash_map_destroy(&metrics);
    initialized = 0;
}

void idcu_performance_start(const char* name) {
    if (!initialized) {
        idcu_performance_profiler_init();
    }
    
    idcu_PerformanceMetric* metric = (idcu_PerformanceMetric*)idcu_hash_map_get(&metrics, name);
    if (!metric) {
        idcu_PerformanceMetric new_metric;
        new_metric.name = name;
        new_metric.call_count = 0;
        new_metric.total_time_ns = 0;
        new_metric.min_time_ns = UINT64_MAX;
        new_metric.max_time_ns = 0;
        new_metric.start_time_ns = get_time_ns();
        new_metric.active = 1;
        idcu_hash_map_set(&metrics, name, &new_metric);
    } else if (!metric->active) {
        metric->start_time_ns = get_time_ns();
        metric->active = 1;
    }
}

void idcu_performance_stop(const char* name) {
    if (!initialized) {
        return;
    }
    
    idcu_PerformanceMetric* metric = (idcu_PerformanceMetric*)idcu_hash_map_get(&metrics, name);
    if (metric && metric->active) {
        uint64_t elapsed = get_time_ns() - metric->start_time_ns;
        metric->call_count++;
        metric->total_time_ns += elapsed;
        if (elapsed < metric->min_time_ns) {
            metric->min_time_ns = elapsed;
        }
        if (elapsed > metric->max_time_ns) {
            metric->max_time_ns = elapsed;
        }
        metric->active = 0;
    }
}

void idcu_performance_profiler_print_report(void) {
    if (!initialized) {
        return;
    }
    
    printf("=== Performance Profiler Report ===\n");
    printf("%-30s %10s %15s %15s %15s %15s\n",
           "Name", "Calls", "Total (ms)", "Avg (ns)", "Min (ns)", "Max (ns)");
    printf("--------------------------------------------------------------------------------------------------------\n");
    
    for (size_t i = 0; i < metrics.capacity; i++) {
        idcu_HashMapItem* item = &metrics.items[i];
        if (item->key) {
            idcu_PerformanceMetric* metric = (idcu_PerformanceMetric*)&item->value;
            double avg_ns = metric->call_count > 0 ? 
                (double)metric->total_time_ns / metric->call_count : 0;
            double total_ms = (double)metric->total_time_ns / 1000000.0;
            
            printf("%-30s %10" PRIu64 " %15.3f %15.0f %15" PRIu64 " %15" PRIu64 "\n",
                   metric->name,
                   metric->call_count,
                   total_ms,
                   avg_ns,
                   metric->min_time_ns == UINT64_MAX ? 0 : metric->min_time_ns,
                   metric->max_time_ns);
        }
    }
    
    printf("========================================================================================================\n");
}

void idcu_performance_profiler_reset(void) {
    if (!initialized) {
        return;
    }
    
    for (size_t i = 0; i < metrics.capacity; i++) {
        idcu_HashMapItem* item = &metrics.items[i];
        if (item->key) {
            idcu_PerformanceMetric* metric = (idcu_PerformanceMetric*)&item->value;
            metric->call_count = 0;
            metric->total_time_ns = 0;
            metric->min_time_ns = UINT64_MAX;
            metric->max_time_ns = 0;
            metric->active = 0;
        }
    }
}
