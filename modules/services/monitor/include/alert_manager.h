#ifndef IDCU_MONITOR_ALERT_MANAGER_H
#define IDCU_MONITOR_ALERT_MANAGER_H

#include "error_code.h"
#include "lock.h"
#include "metrics.h"
#include <stdint.h>
#include <stddef.h>

#define IDCU_ALERT_MAX_RULES 64
#define IDCU_ALERT_MAX_LABELS 8
#define IDCU_ALERT_LABEL_KEY_MAX 64
#define IDCU_ALERT_LABEL_VALUE_MAX 128
#define IDCU_ALERT_NAME_MAX 64
#define IDCU_ALERT_MSG_MAX 256

typedef enum {
    IDCU_ALERT_LEVEL_INFO = 0,
    IDCU_ALERT_LEVEL_WARNING,
    IDCU_ALERT_LEVEL_ERROR,
    IDCU_ALERT_LEVEL_CRITICAL
} idcu_AlertLevel;

typedef enum {
    IDCU_ALERT_COND_GREATER = 0,
    IDCU_ALERT_COND_LESS,
    IDCU_ALERT_COND_EQUAL,
    IDCU_ALERT_COND_NOT_EQUAL,
    IDCU_ALERT_COND_GREATER_EQUAL,
    IDCU_ALERT_COND_LESS_EQUAL
} idcu_AlertCondition;

typedef enum {
    IDCU_ALERT_STATE_INACTIVE = 0,
    IDCU_ALERT_STATE_PENDING,
    IDCU_ALERT_STATE_FIRING,
    IDCU_ALERT_STATE_RESOLVED
} idcu_AlertState;

typedef struct {
    char key[IDCU_ALERT_LABEL_KEY_MAX];
    char value[IDCU_ALERT_LABEL_VALUE_MAX];
} idcu_AlertLabel;

typedef struct {
    char name[IDCU_ALERT_NAME_MAX];
    idcu_AlertLevel level;
    char metric_name[64];
    idcu_AlertCondition condition;
    uint64_t threshold;
    uint64_t pending_duration_ms;
    uint64_t last_triggered_ms;
    idcu_AlertState state;
    idcu_AlertLabel labels[IDCU_ALERT_MAX_LABELS];
    int label_count;
    char message[IDCU_ALERT_MSG_MAX];
} idcu_AlertRule;

typedef struct {
    char alert_name[IDCU_ALERT_NAME_MAX];
    idcu_AlertLevel level;
    idcu_AlertState state;
    uint64_t started_at_ms;
    uint64_t resolved_at_ms;
    char message[IDCU_ALERT_MSG_MAX];
    idcu_AlertLabel labels[IDCU_ALERT_MAX_LABELS];
    int label_count;
} idcu_AlertEvent;

typedef void (*idcu_AlertCallback)(const idcu_AlertEvent* event, void* user_data);

typedef struct {
    idcu_AlertRule rules[IDCU_ALERT_MAX_RULES];
    int rule_count;
    idcu_AlertEvent events[IDCU_ALERT_MAX_RULES * 2];
    int event_count;
    idcu_AlertCallback callback;
    void* callback_user_data;
    idcu_Mutex lock;
    int initialized;
} idcu_AlertManager;

int idcu_alert_manager_init(idcu_AlertManager* mgr);
void idcu_alert_manager_destroy(idcu_AlertManager* mgr);

int idcu_alert_manager_add_rule(idcu_AlertManager* mgr, const idcu_AlertRule* rule);
int idcu_alert_manager_remove_rule(idcu_AlertManager* mgr, const char* name);
int idcu_alert_manager_set_callback(idcu_AlertManager* mgr, idcu_AlertCallback cb, void* user_data);

int idcu_alert_manager_evaluate(idcu_AlertManager* mgr, idcu_MetricsCollector* metrics);
int idcu_alert_manager_get_events(idcu_AlertManager* mgr, idcu_AlertEvent* events, int max_events, int* count);
int idcu_alert_manager_clear_resolved(idcu_AlertManager* mgr);

void idcu_alert_rule_init(idcu_AlertRule* rule, const char* name, idcu_AlertLevel level,
                           const char* metric_name, idcu_AlertCondition cond, uint64_t threshold);
int idcu_alert_rule_add_label(idcu_AlertRule* rule, const char* key, const char* value);
void idcu_alert_rule_set_message(idcu_AlertRule* rule, const char* msg);
void idcu_alert_rule_set_pending_duration(idcu_AlertRule* rule, uint64_t ms);

#endif
