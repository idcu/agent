#include "idcu/device_collector/device_collector.h"
#include "idcu/log/log.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct idcu_DeviceCollectorInternal {
    idcu_DeviceInfo devices[IDCU_DEVICE_MAX_DEVICES];
    int device_count;
    idcu_DeviceDataCallback data_callback;
    void* data_callback_user_data;
    idcu_DeviceStatusCallback status_callback;
    void* status_callback_user_data;
};

static uint64_t get_current_time_ms(void) {
#ifdef _WIN32
    return (uint64_t)GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

int idcu_device_collector_init(idcu_DeviceCollector* collector) {
    if (!collector) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_DeviceCollectorInternal* internal = 
        (struct idcu_DeviceCollectorInternal*)malloc(sizeof(struct idcu_DeviceCollectorInternal));
    if (!internal) return IDCU_ERR_NO_MEMORY;
    
    memset(internal, 0, sizeof(struct idcu_DeviceCollectorInternal));
    collector->internal = internal;
    
    return IDCU_ERR_OK;
}

void idcu_device_collector_destroy(idcu_DeviceCollector* collector) {
    if (!collector || !collector->internal) return;
    free(collector->internal);
    collector->internal = NULL;
}

static int find_device_index(struct idcu_DeviceCollectorInternal* internal, const char* device_name) {
    for (int i = 0; i < internal->device_count; i++) {
        if (strcmp(internal->devices[i].name, device_name) == 0) {
            return i;
        }
    }
    return -1;
}

int idcu_device_collector_register_device(idcu_DeviceCollector* collector, const idcu_DeviceInfo* device) {
    if (!collector || !collector->internal || !device) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_DeviceCollectorInternal* internal = 
        (struct idcu_DeviceCollectorInternal*)collector->internal;
    
    if (internal->device_count >= IDCU_DEVICE_MAX_DEVICES) {
        return IDCU_ERR_NO_MEMORY;
    }
    
    if (find_device_index(internal, device->name) >= 0) {
        return IDCU_ERR_ALREADY_EXISTS;
    }
    
    internal->devices[internal->device_count] = *device;
    internal->devices[internal->device_count].last_seen_ms = get_current_time_ms();
    internal->device_count++;
    
    if (internal->status_callback) {
        internal->status_callback(device->name, device->status, internal->status_callback_user_data);
    }
    
    return IDCU_ERR_OK;
}

int idcu_device_collector_unregister_device(idcu_DeviceCollector* collector, const char* device_name) {
    if (!collector || !collector->internal || !device_name) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_DeviceCollectorInternal* internal = 
        (struct idcu_DeviceCollectorInternal*)collector->internal;
    
    int index = find_device_index(internal, device_name);
    if (index < 0) {
        return IDCU_ERR_NOT_FOUND;
    }
    
    for (int i = index; i < internal->device_count - 1; i++) {
        internal->devices[i] = internal->devices[i + 1];
    }
    internal->device_count--;
    
    return IDCU_ERR_OK;
}

int idcu_device_collector_get_device(idcu_DeviceCollector* collector, const char* device_name, idcu_DeviceInfo* device) {
    if (!collector || !collector->internal || !device_name || !device) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_DeviceCollectorInternal* internal = 
        (struct idcu_DeviceCollectorInternal*)collector->internal;
    
    int index = find_device_index(internal, device_name);
    if (index < 0) {
        return IDCU_ERR_NOT_FOUND;
    }
    
    *device = internal->devices[index];
    return IDCU_ERR_OK;
}

int idcu_device_collector_list_devices(idcu_DeviceCollector* collector, idcu_DeviceInfo* devices, int* count) {
    if (!collector || !collector->internal || !devices || !count) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_DeviceCollectorInternal* internal = 
        (struct idcu_DeviceCollectorInternal*)collector->internal;
    
    int max_count = *count;
    *count = internal->device_count < max_count ? internal->device_count : max_count;
    
    for (int i = 0; i < *count; i++) {
        devices[i] = internal->devices[i];
    }
    
    return IDCU_ERR_OK;
}

int idcu_device_collector_update_device_status(idcu_DeviceCollector* collector, const char* device_name, idcu_DeviceStatus status) {
    if (!collector || !collector->internal || !device_name) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_DeviceCollectorInternal* internal = 
        (struct idcu_DeviceCollectorInternal*)collector->internal;
    
    int index = find_device_index(internal, device_name);
    if (index < 0) {
        return IDCU_ERR_NOT_FOUND;
    }
    
    idcu_DeviceStatus old_status = internal->devices[index].status;
    internal->devices[index].status = status;
    internal->devices[index].last_seen_ms = get_current_time_ms();
    
    if (old_status != status && internal->status_callback) {
        internal->status_callback(device_name, status, internal->status_callback_user_data);
    }
    
    return IDCU_ERR_OK;
}

int idcu_device_collector_add_data_point(idcu_DeviceCollector* collector, const char* device_name, const idcu_DeviceDataPoint* data) {
    if (!collector || !collector->internal || !device_name || !data) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_DeviceCollectorInternal* internal = 
        (struct idcu_DeviceCollectorInternal*)collector->internal;
    
    int index = find_device_index(internal, device_name);
    if (index < 0) {
        return IDCU_ERR_NOT_FOUND;
    }
    
    idcu_DeviceInfo* device = &internal->devices[index];
    if (device->data_point_count >= IDCU_DEVICE_DATA_MAX) {
        for (int i = 0; i < IDCU_DEVICE_DATA_MAX - 1; i++) {
            device->data_points[i] = device->data_points[i + 1];
        }
        device->data_point_count--;
    }
    
    device->data_points[device->data_point_count] = *data;
    device->data_point_count++;
    device->last_seen_ms = get_current_time_ms();
    
    if (internal->data_callback) {
        internal->data_callback(device_name, data, internal->data_callback_user_data);
    }
    
    return IDCU_ERR_OK;
}

int idcu_device_collector_clear_data_points(idcu_DeviceCollector* collector, const char* device_name) {
    if (!collector || !collector->internal || !device_name) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_DeviceCollectorInternal* internal = 
        (struct idcu_DeviceCollectorInternal*)collector->internal;
    
    int index = find_device_index(internal, device_name);
    if (index < 0) {
        return IDCU_ERR_NOT_FOUND;
    }
    
    internal->devices[index].data_point_count = 0;
    return IDCU_ERR_OK;
}

int idcu_device_collector_set_data_callback(idcu_DeviceCollector* collector, idcu_DeviceDataCallback callback, void* user_data) {
    if (!collector || !collector->internal) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_DeviceCollectorInternal* internal = 
        (struct idcu_DeviceCollectorInternal*)collector->internal;
    
    internal->data_callback = callback;
    internal->data_callback_user_data = user_data;
    
    return IDCU_ERR_OK;
}

int idcu_device_collector_set_status_callback(idcu_DeviceCollector* collector, idcu_DeviceStatusCallback callback, void* user_data) {
    if (!collector || !collector->internal) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_DeviceCollectorInternal* internal = 
        (struct idcu_DeviceCollectorInternal*)collector->internal;
    
    internal->status_callback = callback;
    internal->status_callback_user_data = user_data;
    
    return IDCU_ERR_OK;
}

static void append_data_point_json(char* buffer, size_t* offset, size_t buffer_size, const idcu_DeviceDataPoint* data) {
    const char* type_str = "";
    switch (data->type) {
        case IDCU_DATA_TYPE_INT: type_str = "int"; break;
        case IDCU_DATA_TYPE_FLOAT: type_str = "float"; break;
        case IDCU_DATA_TYPE_STRING: type_str = "string"; break;
        case IDCU_DATA_TYPE_BOOL: type_str = "bool"; break;
    }
    
    int written = snprintf(buffer + *offset, buffer_size - *offset,
        "{\"key\":\"%s\",\"type\":\"%s\",",
        data->key, type_str
    );
    
    if (written > 0) *offset += written;
    
    switch (data->type) {
        case IDCU_DATA_TYPE_INT:
            written = snprintf(buffer + *offset, buffer_size - *offset,
                "\"value\":%lld,", (long long)data->value.int_val
            );
            break;
        case IDCU_DATA_TYPE_FLOAT:
            written = snprintf(buffer + *offset, buffer_size - *offset,
                "\"value\":%.2f,", data->value.float_val
            );
            break;
        case IDCU_DATA_TYPE_STRING:
            written = snprintf(buffer + *offset, buffer_size - *offset,
                "\"value\":\"%s\",", data->value.str_val
            );
            break;
        case IDCU_DATA_TYPE_BOOL:
            written = snprintf(buffer + *offset, buffer_size - *offset,
                "\"value\":%s,", data->value.bool_val ? "true" : "false"
            );
            break;
    }
    
    if (written > 0) *offset += written;
    
    written = snprintf(buffer + *offset, buffer_size - *offset,
        "\"timestamp\":%llu}", (unsigned long long)data->timestamp_ms
    );
    
    if (written > 0) *offset += written;
}

static void append_device_json(char* buffer, size_t* offset, size_t buffer_size, const idcu_DeviceInfo* device) {
    const char* status_str = "";
    switch (device->status) {
        case IDCU_DEVICE_STATUS_OFFLINE: status_str = "offline"; break;
        case IDCU_DEVICE_STATUS_ONLINE: status_str = "online"; break;
        case IDCU_DEVICE_STATUS_WARNING: status_str = "warning"; break;
        case IDCU_DEVICE_STATUS_ERROR: status_str = "error"; break;
    }
    
    int written = snprintf(buffer + *offset, buffer_size - *offset,
        "{\"name\":\"%s\",\"type\":\"%s\",\"location\":\"%s\",\"status\":\"%s\","
        "\"last_seen\":%llu,\"tags\":[",
        device->name, device->device_type, device->location, status_str,
        (unsigned long long)device->last_seen_ms
    );
    
    if (written > 0) *offset += written;
    
    for (int i = 0; i < device->tag_count; i++) {
        if (i > 0) {
            written = snprintf(buffer + *offset, buffer_size - *offset, ",");
            if (written > 0) *offset += written;
        }
        written = snprintf(buffer + *offset, buffer_size - *offset, "\"%s\"", device->tags[i]);
        if (written > 0) *offset += written;
    }
    
    written = snprintf(buffer + *offset, buffer_size - *offset, "],\"data\":[");
    if (written > 0) *offset += written;
    
    for (int i = 0; i < device->data_point_count; i++) {
        if (i > 0) {
            written = snprintf(buffer + *offset, buffer_size - *offset, ",");
            if (written > 0) *offset += written;
        }
        append_data_point_json(buffer, offset, buffer_size, &device->data_points[i]);
    }
    
    written = snprintf(buffer + *offset, buffer_size - *offset, "]}");
    if (written > 0) *offset += written;
}

int idcu_device_collector_export_json(idcu_DeviceCollector* collector, char* buffer, size_t buffer_size, size_t* output_size) {
    if (!collector || !collector->internal || !buffer || !output_size) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_DeviceCollectorInternal* internal = 
        (struct idcu_DeviceCollectorInternal*)collector->internal;
    
    size_t offset = 0;
    int written = snprintf(buffer + offset, buffer_size - offset, "{\"devices\":[");
    if (written < 0) return IDCU_ERR_NO_MEMORY;
    offset += written;
    
    for (int i = 0; i < internal->device_count; i++) {
        if (i > 0) {
            written = snprintf(buffer + offset, buffer_size - offset, ",");
            if (written < 0) return IDCU_ERR_NO_MEMORY;
            offset += written;
        }
        append_device_json(buffer, &offset, buffer_size, &internal->devices[i]);
    }
    
    written = snprintf(buffer + offset, buffer_size - offset, "]}");
    if (written < 0) return IDCU_ERR_NO_MEMORY;
    offset += written;
    
    *output_size = offset;
    return IDCU_ERR_OK;
}

int idcu_device_collector_export_device_json(idcu_DeviceCollector* collector, const char* device_name, char* buffer, size_t buffer_size, size_t* output_size) {
    if (!collector || !collector->internal || !device_name || !buffer || !output_size) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_DeviceCollectorInternal* internal = 
        (struct idcu_DeviceCollectorInternal*)collector->internal;
    
    int index = find_device_index(internal, device_name);
    if (index < 0) {
        return IDCU_ERR_NOT_FOUND;
    }
    
    size_t offset = 0;
    append_device_json(buffer, &offset, buffer_size, &internal->devices[index]);
    
    *output_size = offset;
    return IDCU_ERR_OK;
}
