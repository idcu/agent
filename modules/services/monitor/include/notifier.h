#ifndef IDCU_MONITOR_NOTIFIER_H
#define IDCU_MONITOR_NOTIFIER_H

#include "error_code.h"
#include "alert_manager.h"
#include <stdint.h>
#include <stddef.h>

#define IDCU_NOTIFIER_MAX_CHANNELS 8
#define IDCU_NOTIFIER_WEBHOOK_URL_MAX 512
#define IDCU_NOTIFIER_FILE_PATH_MAX 512

typedef enum {
    IDCU_NOTIFIER_CHANNEL_LOG = 0,
    IDCU_NOTIFIER_CHANNEL_FILE,
    IDCU_NOTIFIER_CHANNEL_WEBHOOK
} idcu_NotifierChannelType;

typedef struct {
    idcu_NotifierChannelType type;
    int enabled;
    union {
        struct {
            char file_path[IDCU_NOTIFIER_FILE_PATH_MAX];
        } file;
        struct {
            char url[IDCU_NOTIFIER_WEBHOOK_URL_MAX];
            int timeout_ms;
        } webhook;
    } config;
} idcu_NotifierChannel;

typedef struct {
    idcu_NotifierChannel channels[IDCU_NOTIFIER_MAX_CHANNELS];
    int channel_count;
    int initialized;
} idcu_Notifier;

int idcu_notifier_init(idcu_Notifier* notifier);
void idcu_notifier_destroy(idcu_Notifier* notifier);

int idcu_notifier_add_log_channel(idcu_Notifier* notifier);
int idcu_notifier_add_file_channel(idcu_Notifier* notifier, const char* file_path);
int idcu_notifier_add_webhook_channel(idcu_Notifier* notifier, const char* url, int timeout_ms);

int idcu_notifier_send(idcu_Notifier* notifier, const idcu_AlertEvent* event);

void idcu_notifier_alert_callback(const idcu_AlertEvent* event, void* user_data);

#endif
