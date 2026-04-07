/**
 * @file custom_collector_example.c
 * @brief 自定义数据采集模块示例
 *
 * 本示例演示如何使用IDCU SDK开发一个完整的自定义数据采集模块，
 * 包括数据采集、存储、统计和上报功能。
 */

#include "sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>
#define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

#define MAX_DATA_POINTS 100
#define COLLECT_INTERVAL_MS 1000

typedef struct {
    int64_t timestamp;
    double value;
} DataPoint;

typedef struct {
    DataPoint data_points[MAX_DATA_POINTS];
    int data_count;
    double sum;
    double min;
    double max;
    int collect_count;
    int64_t last_collect_time;
} CollectorData;

static double get_random_value(void) {
    return (double)rand() / RAND_MAX * 100.0;
}

static int64_t get_current_timestamp(void) {
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return (uli.QuadPart - 116444736000000000LL) / 10000;
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
}

static void update_statistics(CollectorData* data, double value) {
    data->sum += value;
    if (data->data_count == 0 || value < data->min) {
        data->min = value;
    }
    if (data->data_count == 0 || value > data->max) {
        data->max = value;
    }
}

static void collect_data(idcu_SdkContext* ctx, CollectorData* data) {
    int64_t now = get_current_timestamp();
    
    if (now - data->last_collect_time < COLLECT_INTERVAL_MS) {
        return;
    }
    
    data->last_collect_time = now;
    
    double value = get_random_value();
    
    int index = data->data_count % MAX_DATA_POINTS;
    data->data_points[index].timestamp = now;
    data->data_points[index].value = value;
    
    if (data->data_count < MAX_DATA_POINTS) {
        data->data_count++;
    }
    
    update_statistics(data, value);
    data->collect_count++;
    
    idcu_sdk_log_info(ctx, "Collected data: %.2f (total: %d)", value, data->collect_count);
}

static void print_statistics(idcu_SdkContext* ctx, CollectorData* data) {
    if (data->data_count == 0) {
        idcu_sdk_log_info(ctx, "No data collected yet");
        return;
    }
    
    double avg = data->sum / data->data_count;
    idcu_sdk_log_info(ctx, "=== Statistics ===");
    idcu_sdk_log_info(ctx, "Data points: %d", data->data_count);
    idcu_sdk_log_info(ctx, "Total collects: %d", data->collect_count);
    idcu_sdk_log_info(ctx, "Min: %.2f", data->min);
    idcu_sdk_log_info(ctx, "Max: %.2f", data->max);
    idcu_sdk_log_info(ctx, "Avg: %.2f", avg);
    idcu_sdk_log_info(ctx, "Sum: %.2f", data->sum);
    idcu_sdk_log_info(ctx, "==================");
}

static int collector_init(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Initializing custom collector module");
    
    CollectorData* data = (CollectorData*)malloc(sizeof(CollectorData));
    if (!data) {
        idcu_sdk_log_error(ctx, "Failed to allocate memory");
        return IDCU_ERR_NO_MEMORY;
    }
    
    memset(data, 0, sizeof(CollectorData));
    data->min = 0.0;
    data->max = 0.0;
    data->last_collect_time = get_current_timestamp();
    
    srand((unsigned int)time(NULL));
    
    idcu_sdk_set_user_data(ctx, data);
    
    idcu_sdk_log_info(ctx, "Custom collector module initialized");
    return IDCU_ERR_OK;
}

static int collector_start(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Starting custom collector module");
    return IDCU_ERR_OK;
}

static int collector_stop(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Stopping custom collector module");
    
    CollectorData* data = (CollectorData*)idcu_sdk_get_user_data(ctx);
    if (data) {
        print_statistics(ctx, data);
    }
    
    return IDCU_ERR_OK;
}

static void collector_destroy(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Destroying custom collector module");
    
    CollectorData* data = (CollectorData*)idcu_sdk_get_user_data(ctx);
    if (data) {
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    custom_collector,
    "1.0.0",
    "A custom data collector module example",
    collector_init,
    collector_start,
    collector_stop,
    collector_destroy
);

int main(void) {
    printf("====================================\n");
    printf("Custom Collector Example\n");
    printf("====================================\n\n");
    printf("This example demonstrates how to build a custom data collector module.\n");
    printf("To use this as a module, compile it into the IDCU agent.\n\n");
    printf("Key features demonstrated:\n");
    printf("  - Data collection with timestamp\n");
    printf("  - Circular buffer for data storage\n");
    printf("  - Real-time statistics calculation\n");
    printf("  - SDK integration for logging\n\n");
    
    return 0;
}
