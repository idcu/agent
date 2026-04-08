#include <idcu/common/error_code.h>
#include <idcu/device_collector/device_collector.h>
#include <stdio.h>
#include <string.h>

static void device_data_callback(const char *device_name, const idcu_DeviceDataPoint *data,
                                 void *user_data) {
    printf("[DATA] Device: %s, Key: %s\n", device_name, data->key);
}

static void device_status_callback(const char *device_name, idcu_DeviceStatus status,
                                   void *user_data) {
    const char *status_str = "UNKNOWN";
    switch (status) {
    case IDCU_DEVICE_STATUS_OFFLINE:
        status_str = "OFFLINE";
        break;
    case IDCU_DEVICE_STATUS_ONLINE:
        status_str = "ONLINE";
        break;
    case IDCU_DEVICE_STATUS_WARNING:
        status_str = "WARNING";
        break;
    case IDCU_DEVICE_STATUS_ERROR:
        status_str = "ERROR";
        break;
    }
    printf("[STATUS] Device: %s, Status: %s\n", device_name, status_str);
}

int main(void) {
    printf("=== idcu-device-collector Basic Example ===\n\n");

    int ret;
    idcu_DeviceCollector collector;

    printf("1. Initialize Device Collector\n");
    printf("--------------------------------\n");
    ret = idcu_device_collector_init(&collector);
    if (ret != IDCU_ERR_OK) {
        printf("Device collector init failed: %s\n", idcu_err_to_str(ret));
        return 1;
    }
    printf("Device collector initialized\n\n");

    printf("2. Set Callbacks\n");
    printf("------------------\n");
    ret = idcu_device_collector_set_data_callback(&collector, device_data_callback, NULL);
    if (ret == IDCU_ERR_OK) {
        printf("Data callback set\n");
    }

    ret = idcu_device_collector_set_status_callback(&collector, device_status_callback, NULL);
    if (ret == IDCU_ERR_OK) {
        printf("Status callback set\n");
    }
    printf("\n");

    printf("3. Register Devices\n");
    printf("--------------------\n");

    idcu_DeviceInfo temp_sensor = {0};
    strncpy(temp_sensor.name, "temperature_sensor_01", IDCU_DEVICE_NAME_MAX - 1);
    strncpy(temp_sensor.device_type, "sensor", IDCU_DEVICE_TYPE_MAX - 1);
    strncpy(temp_sensor.location, "room_101", IDCU_DEVICE_LOCATION_MAX - 1);
    strncpy(temp_sensor.tags[0], "environment", IDCU_DEVICE_TAG_MAX - 1);
    strncpy(temp_sensor.tags[1], "temperature", IDCU_DEVICE_TAG_MAX - 1);
    temp_sensor.tag_count = 2;
    temp_sensor.status = IDCU_DEVICE_STATUS_ONLINE;

    ret = idcu_device_collector_register_device(&collector, &temp_sensor);
    if (ret == IDCU_ERR_OK) {
        printf("Registered device: %s\n", temp_sensor.name);
    }

    idcu_DeviceInfo humidity_sensor = {0};
    strncpy(humidity_sensor.name, "humidity_sensor_01", IDCU_DEVICE_NAME_MAX - 1);
    strncpy(humidity_sensor.device_type, "sensor", IDCU_DEVICE_TYPE_MAX - 1);
    strncpy(humidity_sensor.location, "room_101", IDCU_DEVICE_LOCATION_MAX - 1);
    humidity_sensor.tag_count = 1;
    strncpy(humidity_sensor.tags[0], "environment", IDCU_DEVICE_TAG_MAX - 1);
    humidity_sensor.status = IDCU_DEVICE_STATUS_ONLINE;

    ret = idcu_device_collector_register_device(&collector, &humidity_sensor);
    if (ret == IDCU_ERR_OK) {
        printf("Registered device: %s\n", humidity_sensor.name);
    }
    printf("\n");

    printf("4. List Registered Devices\n");
    printf("---------------------------\n");
    idcu_DeviceInfo devices[IDCU_DEVICE_MAX_DEVICES];
    int device_count = IDCU_DEVICE_MAX_DEVICES;
    ret = idcu_device_collector_list_devices(&collector, devices, &device_count);
    if (ret == IDCU_ERR_OK) {
        printf("Found %d devices:\n", device_count);
        for (int i = 0; i < device_count; i++) {
            printf("  - %s (%s)\n", devices[i].name, devices[i].device_type);
        }
    }
    printf("\n");

    printf("5. Add Data Points\n");
    printf("--------------------\n");
    idcu_DeviceDataPoint data_point = {0};
    strncpy(data_point.key, "temperature", 63);
    data_point.type = IDCU_DATA_TYPE_FLOAT;
    data_point.value.float_val = 22.5;
    data_point.timestamp_ms = 0;

    ret = idcu_device_collector_add_data_point(&collector, "temperature_sensor_01", &data_point);
    if (ret == IDCU_ERR_OK) {
        printf("Added temperature data: %.1f C\n", data_point.value.float_val);
    }

    strncpy(data_point.key, "humidity", 63);
    data_point.type = IDCU_DATA_TYPE_FLOAT;
    data_point.value.float_val = 65.0;
    ret = idcu_device_collector_add_data_point(&collector, "humidity_sensor_01", &data_point);
    if (ret == IDCU_ERR_OK) {
        printf("Added humidity data: %.1f%%\n", data_point.value.float_val);
    }
    printf("\n");

    printf("6. Get Device Info\n");
    printf("--------------------\n");
    idcu_DeviceInfo retrieved_device;
    ret = idcu_device_collector_get_device(&collector, "temperature_sensor_01", &retrieved_device);
    if (ret == IDCU_ERR_OK) {
        printf("Device: %s\n", retrieved_device.name);
        printf("Status: %d\n", retrieved_device.status);
        printf("Data points: %d\n", retrieved_device.data_point_count);
    }
    printf("\n");

    printf("7. Update Device Status\n");
    printf("-------------------------\n");
    ret = idcu_device_collector_update_device_status(&collector, "temperature_sensor_01",
                                                     IDCU_DEVICE_STATUS_WARNING);
    if (ret == IDCU_ERR_OK) {
        printf("Updated temperature sensor status to WARNING\n");
    }
    printf("\n");

    printf("8. Destroy Device Collector\n");
    printf("-----------------------------\n");
    idcu_device_collector_destroy(&collector);
    printf("Device collector destroyed\n\n");

    printf("=== Example Complete ===\n");
    return 0;
}
