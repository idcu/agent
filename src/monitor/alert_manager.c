#include "monitor/alert_manager.h"
#include "utils/log.h"
#include "monitor/health_check.h"
#include <string.h>
#include <stdio.h>

static uint64_t get_current_time_ms_alrt(void)
{
    return idcu_health_get_uptime_ms();
}

static int evaluate_condition(uint64_t value, idcu_AlertCondition cond, uint64_t threshold)
{
    switch (cond) {
        case IDCU_ALERT_COND_GREATER:
            return value > threshold;
        case IDCU_ALERT_COND_LESS:
            return value < threshold;
        case IDCU_ALERT_COND_EQUAL:
            return value == threshold;
        case IDCU_ALERT_COND_NOT_EQUAL:
            return value != threshold;
        case IDCU_ALERT_COND_GREATER_EQUAL:
            return value >= threshold;
        case IDCU_ALERT_COND_LESS_EQUAL:
            return value <= threshold;
        default:
            return 0;
    }
}

static void add_event(idcu_AlertManager* mgr, const idcu_AlertEvent* event)
{
    if (mgr->event_count >= IDCU_ALERT_MAX_RULES * 2) {
        memmove(&mgr->events[0], &mgr->events[1], 
                (mgr->event_count - 1) * sizeof(idcu_AlertEvent));
        mgr->event_count--;
    }
    memcpy(&mgr->events[mgr->event_count++], event, sizeof(idcu_AlertEvent));
    
    if (mgr->callback) {
        mgr->callback(event, mgr->callback_user_data);
    }
    
    IDCU_LOG_INFO("alert event: %s [%s] %s", 
                   event->alert_name,
                   event->state == IDCU_ALERT_STATE_FIRING ? "FIRING" : "RESOLVED",
                   event->message);
}

int idcu_alert_manager_init(idcu_AlertManager* mgr)
{
    if (!mgr) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    memset(mgr, 0, sizeof(idcu_AlertManager));
    
    int ret = idcu_mutex_init(&mgr->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("alert manager: failed to init lock");
        return ret;
    }
    
    mgr->initialized = 1;
    IDCU_LOG_INFO("alert manager initialized");
    return IDCU_ERR_SUCCESS;
}

void idcu_alert_manager_destroy(idcu_AlertManager* mgr)
{
    if (!mgr || !mgr->initialized) {
        return;
    }
    
    idcu_mutex_destroy(&mgr->lock);
    mgr->initialized = 0;
    IDCU_LOG_INFO("alert manager destroyed");
}

int idcu_alert_manager_add_rule(idcu_AlertManager* mgr, const idcu_AlertRule* rule)
{
    if (!mgr || !mgr->initialized || !rule) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&mgr->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    for (int i = 0; i < mgr->rule_count; i++) {
        if (strcmp(mgr->rules[i].name, rule->name) == 0) {
            idcu_mutex_unlock(&mgr->lock);
            IDCU_LOG_WARN("alert rule %s already exists", rule->name);
            return IDCU_ERR_ALREADY_EXISTS;
        }
    }
    
    if (mgr->rule_count >= IDCU_ALERT_MAX_RULES) {
        idcu_mutex_unlock(&mgr->lock);
        IDCU_LOG_ERROR("alert manager: max rules reached");
        return IDCU_ERR_QUEUE_FULL;
    }
    
    memcpy(&mgr->rules[mgr->rule_count], rule, sizeof(idcu_AlertRule));
    mgr->rules[mgr->rule_count].state = IDCU_ALERT_STATE_INACTIVE;
    mgr->rules[mgr->rule_count].last_triggered_ms = 0;
    mgr->rule_count++;
    
    idcu_mutex_unlock(&mgr->lock);
    IDCU_LOG_INFO("alert rule added: %s", rule->name);
    return IDCU_ERR_SUCCESS;
}

int idcu_alert_manager_remove_rule(idcu_AlertManager* mgr, const char* name)
{
    if (!mgr || !mgr->initialized || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&mgr->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    int found_idx = -1;
    for (int i = 0; i < mgr->rule_count; i++) {
        if (strcmp(mgr->rules[i].name, name) == 0) {
            found_idx = i;
            break;
        }
    }
    
    if (found_idx < 0) {
        idcu_mutex_unlock(&mgr->lock);
        return IDCU_ERR_NOT_FOUND;
    }
    
    if (found_idx < mgr->rule_count - 1) {
        memmove(&mgr->rules[found_idx], &mgr->rules[found_idx + 1],
                (mgr->rule_count - found_idx - 1) * sizeof(idcu_AlertRule));
    }
    mgr->rule_count--;
    
    idcu_mutex_unlock(&mgr->lock);
    IDCU_LOG_INFO("alert rule removed: %s", name);
    return IDCU_ERR_SUCCESS;
}

int idcu_alert_manager_set_callback(idcu_AlertManager* mgr, idcu_AlertCallback cb, void* user_data)
{
    if (!mgr || !mgr->initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&mgr->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    mgr->callback = cb;
    mgr->callback_user_data = user_data;
    
    idcu_mutex_unlock(&mgr->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_alert_manager_evaluate(idcu_AlertManager* mgr, idcu_MetricsCollector* metrics)
{
    if (!mgr || !mgr->initialized || !metrics) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&mgr->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    uint64_t now = get_current_time_ms_alrt();
    
    for (int i = 0; i < mgr->rule_count; i++) {
        idcu_AlertRule* rule = &mgr->rules[i];
        uint64_t value = idcu_metrics_get(metrics, rule->metric_name);
        int condition_met = evaluate_condition(value, rule->condition, rule->threshold);
        
        if (condition_met) {
            if (rule->state == IDCU_ALERT_STATE_INACTIVE) {
                rule->state = IDCU_ALERT_STATE_PENDING;
                rule->last_triggered_ms = now;
                IDCU_LOG_DEBUG("alert %s pending", rule->name);
            } else if (rule->state == IDCU_ALERT_STATE_PENDING) {
                if (now - rule->last_triggered_ms >= rule->pending_duration_ms) {
                    rule->state = IDCU_ALERT_STATE_FIRING;
                    
                    idcu_AlertEvent event;
                    memset(&event, 0, sizeof(event));
                    strncpy(event.alert_name, rule->name, sizeof(event.alert_name) - 1);
                    event.level = rule->level;
                    event.state = IDCU_ALERT_STATE_FIRING;
                    event.started_at_ms = now;
                    strncpy(event.message, rule->message, sizeof(event.message) - 1);
                    memcpy(event.labels, rule->labels, rule->label_count * sizeof(idcu_AlertLabel));
                    event.label_count = rule->label_count;
                    
                    add_event(mgr, &event);
                }
            }
        } else {
            if (rule->state == IDCU_ALERT_STATE_FIRING) {
                rule->state = IDCU_ALERT_STATE_RESOLVED;
                
                idcu_AlertEvent event;
                memset(&event, 0, sizeof(event));
                strncpy(event.alert_name, rule->name, sizeof(event.alert_name) - 1);
                event.level = rule->level;
                event.state = IDCU_ALERT_STATE_RESOLVED;
                event.resolved_at_ms = now;
                snprintf(event.message, sizeof(event.message), "Resolved: %s", rule->message);
                memcpy(event.labels, rule->labels, rule->label_count * sizeof(idcu_AlertLabel));
                event.label_count = rule->label_count;
                
                add_event(mgr, &event);
                
                rule->state = IDCU_ALERT_STATE_INACTIVE;
            } else if (rule->state == IDCU_ALERT_STATE_PENDING) {
                rule->state = IDCU_ALERT_STATE_INACTIVE;
            }
        }
    }
    
    idcu_mutex_unlock(&mgr->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_alert_manager_get_events(idcu_AlertManager* mgr, idcu_AlertEvent* events, int max_events, int* count)
{
    if (!mgr || !mgr->initialized || !events || !count) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&mgr->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    int copy_count = (mgr->event_count < max_events) ? mgr->event_count : max_events;
    memcpy(events, mgr->events, copy_count * sizeof(idcu_AlertEvent));
    *count = copy_count;
    
    idcu_mutex_unlock(&mgr->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_alert_manager_clear_resolved(idcu_AlertManager* mgr)
{
    if (!mgr || !mgr->initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&mgr->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    int write_idx = 0;
    for (int i = 0; i < mgr->event_count; i++) {
        if (mgr->events[i].state != IDCU_ALERT_STATE_RESOLVED) {
            if (write_idx != i) {
                memcpy(&mgr->events[write_idx], &mgr->events[i], sizeof(idcu_AlertEvent));
            }
            write_idx++;
        }
    }
    mgr->event_count = write_idx;
    
    idcu_mutex_unlock(&mgr->lock);
    return IDCU_ERR_SUCCESS;
}

void idcu_alert_rule_init(idcu_AlertRule* rule, const char* name, idcu_AlertLevel level,
                           const char* metric_name, idcu_AlertCondition cond, uint64_t threshold)
{
    if (!rule || !name || !metric_name) {
        return;
    }
    
    memset(rule, 0, sizeof(idcu_AlertRule));
    strncpy(rule->name, name, sizeof(rule->name) - 1);
    rule->level = level;
    strncpy(rule->metric_name, metric_name, sizeof(rule->metric_name) - 1);
    rule->condition = cond;
    rule->threshold = threshold;
    rule->state = IDCU_ALERT_STATE_INACTIVE;
    rule->pending_duration_ms = 0;
    rule->label_count = 0;
    snprintf(rule->message, sizeof(rule->message), 
             "Alert %s triggered for metric %s", name, metric_name);
}

int idcu_alert_rule_add_label(idcu_AlertRule* rule, const char* key, const char* value)
{
    if (!rule || !key || !value) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    if (rule->label_count >= IDCU_ALERT_MAX_LABELS) {
        return IDCU_ERR_QUEUE_FULL;
    }
    
    strncpy(rule->labels[rule->label_count].key, key, sizeof(rule->labels[rule->label_count].key) - 1);
    strncpy(rule->labels[rule->label_count].value, value, sizeof(rule->labels[rule->label_count].value) - 1);
    rule->label_count++;
    return IDCU_ERR_SUCCESS;
}

void idcu_alert_rule_set_message(idcu_AlertRule* rule, const char* msg)
{
    if (!rule || !msg) {
        return;
    }
    strncpy(rule->message, msg, sizeof(rule->message) - 1);
}

void idcu_alert_rule_set_pending_duration(idcu_AlertRule* rule, uint64_t ms)
{
    if (!rule) {
        return;
    }
    rule->pending_duration_ms = ms;
}
