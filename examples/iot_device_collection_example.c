#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "idcu/device_collector/device_collector.h"
#include "idcu/scheduler/scheduler.h"
#include "idcu/log/log.h"

static idcu_DeviceCollector g_collector;
static idcu_Scheduler g_scheduler;
static int g_running = 1;

static uint64_t get_current_time_ms(void) {
#ifdef _WIN32
    return (uint64_t)GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

static void sleep_ms(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

static void on_device_data(const char* device_name, const idcu_DeviceDataPoint* data, void* user_data) {
    printf("[Data] Device: %s, Key: %s, ", device_name, data->key);
    switch (data->type) {
        case IDCU_DATA_TYPE_INT:
            printf("Value: %lld\n", (long long)data->value.int_val);
            break;
        case IDCU_DATA_TYPE_FLOAT:
            printf("Value: %.2f\n", data->value.float_val);
            break;
        case IDCU_DATA_TYPE_STRING:
            printf("Value: %s\n", data->value.str_val);
            break;
        case IDCU_DATA_TYPE_BOOL:
            printf("Value: %s\n", data->value.bool_val ? "true" : "false");
            break;
    }
}

static void on_device_status(const char* device_name, idcu_DeviceStatus status, void* user_data) {
    const char* status_str = "unknown";
    switch (status) {
        case IDCU_DEVICE_STATUS_OFFLINE: status_str = "offline"; break;
        case IDCU_DEVICE_STATUS_ONLINE: status_str = "online"; break;
        case IDCU_DEVICE_STATUS_WARNING: status_str = "warning"; break;
        case IDCU_DEVICE_STATUS_ERROR: status_str = "error"; break;
    }
    printf("[Status] Device: %s, Status: %s\n", device_name, status_str);
}

static void simulate_sensor_data(void* user_data) {
    const char* device_name = (const char*)user_data;
    
    idcu_DeviceDataPoint temp_data;
    strcpy(temp_data.key, "temperature");
    temp_data.type = IDCU_DATA_TYPE_FLOAT;
    temp_data.value.float_val = 20.0 + (rand() % 200) / 10.0;
    temp_data.timestamp_ms = get_current_time_ms();
    idcu_device_collector_add_data_point(&g_collector, device_name, &temp_data);
    
    idcu_DeviceDataPoint humidity_data;
    strcpy(humidity_data.key, "humidity");
    humidity_data.type = IDCU_DATA_TYPE_FLOAT;
    humidity_data.value.float_val = 40.0 + (rand() % 400) / 10.0;
    humidity_data.timestamp_ms = get_current_time_ms();
    idcu_device_collector_add_data_point(&g_collector, device_name, &humidity_data);
}

static void export_device_data(void* user_data) {
    char buffer[8192];
    size_t size;
    
    if (idcu_device_collector_export_json(&g_collector, buffer, sizeof(buffer), &size) == IDCU_ERR_OK) {
        printf("\n=== Device Data Export ===\n");
        printf("%s\n", buffer);
        printf("=========================\n\n");
    }
}

int main(int argc, char* argv[]) {
    printf("=== IoT Device Collection Example ===\n\n");
    
    idcu_log_init();
    idcu_device_collector_init(&g_collector);
    idcu_scheduler_init(&g_scheduler);
    
    idcu_device_collector_set_data_callback(&g_collector, on_device_data, NULL);
    idcu_device_collector_set_status_callback(&g_collector, on_device_status, NULL);
    
    idcu_DeviceInfo temp_sensor;
    memset(&temp_sensor, 0, sizeof(temp_sensor));
    strcpy(temp_sensor.name, "temp-sensor-001");
    strcpy(temp_sensor.device_type, "temperature");
    strcpy(temp_sensor.location, "room-101");
    temp_sensor.status = IDCU_DEVICE_STATUS_ONLINE;
    temp_sensor.tag_count = 2;
    strcpy(temp_sensor.tags[0], "indoor");
    strcpy(temp_sensor.tags[1], "floor-1");
    idcu_device_collector_register_device(&g_collector, &temp_sensor);
    
    idcu_DeviceInfo humidity_sensor;
    memset(&humidity_sensor, 0, sizeof(humidity_sensor));
    strcpy(humidity_sensor.name, "humidity-sensor-001");
    strcpy(humidity_sensor.device_type, "humidity");
    strcpy(humidity_sensor.location, "room-101");
    humidity_sensor.status = IDCU_DEVICE_STATUS_ONLINE;
    humidity_sensor.tag_count = 2;
    strcpy(humidity_sensor.tags[0], "indoor");
    strcpy(humidity_sensor.tags[1], "floor-1");
    idcu_device_collector_register_device(&g_collector, &humidity_sensor);
    
    idcu_scheduler_start(&g_scheduler);
    
    idcu_scheduler_add_interval_task(&g_scheduler, "simulate-temp", 2000, 1, 
                                      simulate_sensor_data, "temp-sensor-001");
    idcu_scheduler_add_interval_task(&g_scheduler, "simulate-humidity", 3000, 1, 
                                      simulate_sensor_data, "humidity-sensor-001");
    idcu_scheduler_add_interval_task(&g_scheduler, "export-data", 10000, 1, 
                                      export_device_data, NULL);
    
    printf("Devices registered. Starting collection...\n");
    printf("Press Ctrl+C to stop.\n\n");
    
    while (g_running) {
        idcu_scheduler_process(&g_scheduler);
        sleep_ms(100);
    }
    
    idcu_scheduler_stop(&g_scheduler);
    idcu_scheduler_destroy(&g_scheduler);
    idcu_device_collector_destroy(&g_collector);
    idcu_log_destroy();
    
    printf("\nExample completed.\n");
    return 0;
}
