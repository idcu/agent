#ifndef IDCU_DEVICE_COLLECTOR_H
#define IDCU_DEVICE_COLLECTOR_H

#include "idcu/common/error_code.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define IDCU_DEVICE_NAME_MAX     128
#define IDCU_DEVICE_TYPE_MAX     64
#define IDCU_DEVICE_LOCATION_MAX 256
#define IDCU_DEVICE_TAG_MAX      32
#define IDCU_DEVICE_MAX_TAGS     8
#define IDCU_DEVICE_DATA_MAX     64
#define IDCU_DEVICE_MAX_DEVICES  32

    typedef enum
    {
        IDCU_DEVICE_STATUS_OFFLINE = 0,
        IDCU_DEVICE_STATUS_ONLINE,
        IDCU_DEVICE_STATUS_WARNING,
        IDCU_DEVICE_STATUS_ERROR
    } idcu_DeviceStatus;

    typedef enum
    {
        IDCU_DATA_TYPE_INT = 0,
        IDCU_DATA_TYPE_FLOAT,
        IDCU_DATA_TYPE_STRING,
        IDCU_DATA_TYPE_BOOL
    } idcu_DataType;

    typedef struct
    {
        char          key[64];
        idcu_DataType type;
        union
        {
            int64_t int_val;
            double  float_val;
            char    str_val[256];
            int     bool_val;
        } value;
        uint64_t timestamp_ms;
    } idcu_DeviceDataPoint;

    typedef struct
    {
        char                 name[IDCU_DEVICE_NAME_MAX];
        char                 device_type[IDCU_DEVICE_TYPE_MAX];
        char                 location[IDCU_DEVICE_LOCATION_MAX];
        char                 tags[IDCU_DEVICE_MAX_TAGS][IDCU_DEVICE_TAG_MAX];
        int                  tag_count;
        idcu_DeviceStatus    status;
        uint64_t             last_seen_ms;
        idcu_DeviceDataPoint data_points[IDCU_DEVICE_DATA_MAX];
        int                  data_point_count;
    } idcu_DeviceInfo;

    typedef void (*idcu_DeviceDataCallback)(const char*                 device_name,
                                            const idcu_DeviceDataPoint* data, void* user_data);
    typedef void (*idcu_DeviceStatusCallback)(const char* device_name, idcu_DeviceStatus status,
                                              void* user_data);

    typedef struct
    {
        void* internal;
    } idcu_DeviceCollector;

    int  idcu_device_collector_init(idcu_DeviceCollector* collector);
    void idcu_device_collector_destroy(idcu_DeviceCollector* collector);

    int idcu_device_collector_register_device(idcu_DeviceCollector*  collector,
                                              const idcu_DeviceInfo* device);
    int idcu_device_collector_unregister_device(idcu_DeviceCollector* collector,
                                                const char*           device_name);
    int idcu_device_collector_get_device(idcu_DeviceCollector* collector, const char* device_name,
                                         idcu_DeviceInfo* device);
    int idcu_device_collector_list_devices(idcu_DeviceCollector* collector,
                                           idcu_DeviceInfo* devices, int* count);

    int idcu_device_collector_update_device_status(idcu_DeviceCollector* collector,
                                                   const char*           device_name,
                                                   idcu_DeviceStatus     status);
    int idcu_device_collector_add_data_point(idcu_DeviceCollector*       collector,
                                             const char*                 device_name,
                                             const idcu_DeviceDataPoint* data);
    int idcu_device_collector_clear_data_points(idcu_DeviceCollector* collector,
                                                const char*           device_name);

    int idcu_device_collector_set_data_callback(idcu_DeviceCollector*   collector,
                                                idcu_DeviceDataCallback callback, void* user_data);
    int idcu_device_collector_set_status_callback(idcu_DeviceCollector*     collector,
                                                  idcu_DeviceStatusCallback callback,
                                                  void*                     user_data);

    int idcu_device_collector_export_json(idcu_DeviceCollector* collector, char* buffer,
                                          size_t buffer_size, size_t* output_size);
    int idcu_device_collector_export_device_json(idcu_DeviceCollector* collector,
                                                 const char* device_name, char* buffer,
                                                 size_t buffer_size, size_t* output_size);

#ifdef __cplusplus
}
#endif

#endif
