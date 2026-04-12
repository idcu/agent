#ifndef IDCU_ALERT_ALERT_H
#define IDCU_ALERT_ALERT_H

#include <idcu/common/error_code.h>
#include <idcu/common/vector.h>
#include <idcu/common/lock.h>
#include <idcu/common/hash_map.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_ALERT_SEVERITY_INFO = 0,
    IDCU_ALERT_SEVERITY_WARNING,
    IDCU_ALERT_SEVERITY_ERROR,
    IDCU_ALERT_SEVERITY_CRITICAL
} idcu_AlertSeverity;

typedef enum
{
    IDCU_ALERT_STATUS_ACTIVE = 0,
    IDCU_ALERT_STATUS_ACKNOWLEDGED,
    IDCU_ALERT_STATUS_RESOLVED,
    IDCU_ALERT_STATUS_CLOSED
} idcu_AlertStatus;

typedef enum
{
    IDCU_ALERT_CHANNEL_EMAIL = 0,
    IDCU_ALERT_CHANNEL_SMS,
    IDCU_ALERT_CHANNEL_WEBHOOK,
    IDCU_ALERT_CHANNEL_SLACK,
    IDCU_ALERT_CHANNEL_CUSTOM
} idcu_AlertChannelType;

typedef struct
{
    char id[64];
    char name[128];
    char description[512];
    idcu_AlertSeverity severity;
    idcu_AlertStatus status;
    uint64_t created_at;
    uint64_t updated_at;
    char source[128];
    char labels[1024];
} idcu_Alert;

typedef int (*idcu_AlertChannelFunc)(void* user_data, const idcu_Alert* alert);

typedef struct idcu_AlertChannel
{
    char name[128];
    idcu_AlertChannelType type;
    idcu_AlertChannelFunc send_func;
    void* user_data;
    int enabled;
    idcu_AlertSeverity min_severity;
} idcu_AlertChannel;

typedef struct
{
    idcu_Vector alerts;
    idcu_Vector channels;
    idcu_HashMap alerts_by_id;
    idcu_Mutex lock;
    int initialized;
    uint64_t next_alert_id;
} idcu_AlertManager;

int  idcu_alert_manager_init(idcu_AlertManager* manager);
void idcu_alert_manager_destroy(idcu_AlertManager* manager);

int  idcu_alert_create(idcu_Alert* alert, const char* name, idcu_AlertSeverity severity, const char* description);
void idcu_alert_destroy(idcu_Alert* alert);

int  idcu_alert_manager_add_alert(idcu_AlertManager* manager, const idcu_Alert* alert);
int  idcu_alert_manager_remove_alert(idcu_AlertManager* manager, const char* id);
idcu_Alert* idcu_alert_manager_get_alert(idcu_AlertManager* manager, const char* id);

int  idcu_alert_manager_acknowledge_alert(idcu_AlertManager* manager, const char* id);
int  idcu_alert_manager_resolve_alert(idcu_AlertManager* manager, const char* id);
int  idcu_alert_manager_close_alert(idcu_AlertManager* manager, const char* id);

int  idcu_alert_manager_add_channel(idcu_AlertManager* manager, const idcu_AlertChannel* channel);
int  idcu_alert_manager_remove_channel(idcu_AlertManager* manager, const char* name);

int  idcu_alert_manager_send_alert(idcu_AlertManager* manager, const idcu_Alert* alert);
int  idcu_alert_manager_get_alerts_by_severity(idcu_AlertManager* manager, idcu_AlertSeverity severity, idcu_Vector* results);
int  idcu_alert_manager_get_alerts_by_status(idcu_AlertManager* manager, idcu_AlertStatus status, idcu_Vector* results);

const char* idcu_alert_severity_to_string(idcu_AlertSeverity severity);
const char* idcu_alert_status_to_string(idcu_AlertStatus status);

#ifdef __cplusplus
}
#endif

#endif
