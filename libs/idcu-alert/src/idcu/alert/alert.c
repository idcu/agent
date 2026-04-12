#include <idcu/alert/alert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <inttypes.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

static uint64_t get_time_ms(void) {
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER li;
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    return (li.QuadPart - 116444736000000000ULL) / 10000;
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

const char* idcu_alert_severity_to_string(idcu_AlertSeverity severity) {
    switch (severity) {
        case IDCU_ALERT_SEVERITY_INFO: return "info";
        case IDCU_ALERT_SEVERITY_WARNING: return "warning";
        case IDCU_ALERT_SEVERITY_ERROR: return "error";
        case IDCU_ALERT_SEVERITY_CRITICAL: return "critical";
        default: return "unknown";
    }
}

const char* idcu_alert_status_to_string(idcu_AlertStatus status) {
    switch (status) {
        case IDCU_ALERT_STATUS_ACTIVE: return "active";
        case IDCU_ALERT_STATUS_ACKNOWLEDGED: return "acknowledged";
        case IDCU_ALERT_STATUS_RESOLVED: return "resolved";
        case IDCU_ALERT_STATUS_CLOSED: return "closed";
        default: return "unknown";
    }
}

int idcu_alert_create(idcu_Alert* alert, const char* name, idcu_AlertSeverity severity, const char* description) {
    if (!alert || !name) {
        return IDCU_ERR_INVALID_ARG;
    }
    memset(alert, 0, sizeof(idcu_Alert));
    strncpy(alert->name, name, sizeof(alert->name) - 1);
    alert->severity = severity;
    alert->status = IDCU_ALERT_STATUS_ACTIVE;
    alert->created_at = get_time_ms();
    alert->updated_at = alert->created_at;
    if (description) {
        strncpy(alert->description, description, sizeof(alert->description) - 1);
    }
    return IDCU_ERR_OK;
}

void idcu_alert_destroy(idcu_Alert* alert) {
    (void)alert;
}

static void alert_dtor(void* element) {
    idcu_Alert* alert = (idcu_Alert*)element;
    idcu_alert_destroy(alert);
}

static void channel_dtor(void* element) {
    (void)element;
}

int idcu_alert_manager_init(idcu_AlertManager* manager) {
    if (!manager) {
        return IDCU_ERR_INVALID_ARG;
    }
    memset(manager, 0, sizeof(idcu_AlertManager));
    int ret = idcu_vector_init_with_dtor(&manager->alerts, sizeof(idcu_Alert), 16, alert_dtor);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    ret = idcu_vector_init_with_dtor(&manager->channels, sizeof(idcu_AlertChannel), 8, channel_dtor);
    if (ret != IDCU_ERR_OK) {
        idcu_vector_destroy(&manager->alerts);
        return ret;
    }
    ret = idcu_hash_map_init(&manager->alerts_by_id, 64, sizeof(idcu_Alert*));
    if (ret != IDCU_ERR_OK) {
        idcu_vector_destroy(&manager->channels);
        idcu_vector_destroy(&manager->alerts);
        return ret;
    }
    ret = idcu_mutex_init(&manager->lock);
    if (ret != IDCU_ERR_OK) {
        idcu_hash_map_destroy(&manager->alerts_by_id);
        idcu_vector_destroy(&manager->channels);
        idcu_vector_destroy(&manager->alerts);
        return ret;
    }
    manager->next_alert_id = 1;
    manager->initialized = 1;
    return IDCU_ERR_OK;
}

void idcu_alert_manager_destroy(idcu_AlertManager* manager) {
    if (!manager) {
        return;
    }
    idcu_mutex_lock(&manager->lock);
    idcu_hash_map_destroy(&manager->alerts_by_id);
    idcu_vector_destroy(&manager->channels);
    idcu_vector_destroy(&manager->alerts);
    idcu_Mutex lock_copy = manager->lock;
    manager->initialized = 0;
    idcu_mutex_unlock(&lock_copy);
    idcu_mutex_destroy(&lock_copy);
}

int idcu_alert_manager_add_alert(idcu_AlertManager* manager, const idcu_Alert* alert) {
    if (!manager || !manager->initialized || !alert) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&manager->lock);
    idcu_Alert new_alert = *alert;
    snprintf(new_alert.id, sizeof(new_alert.id), "alert-%" PRIu64, manager->next_alert_id++);
    int ret = idcu_vector_push_back(&manager->alerts, &new_alert);
    if (ret == IDCU_ERR_OK) {
        idcu_Alert* stored_alert = (idcu_Alert*)idcu_vector_back(&manager->alerts);
        idcu_hash_map_set(&manager->alerts_by_id, stored_alert->id, stored_alert);
    }
    idcu_mutex_unlock(&manager->lock);
    return ret;
}

int idcu_alert_manager_remove_alert(idcu_AlertManager* manager, const char* id) {
    if (!manager || !manager->initialized || !id) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&manager->lock);
    size_t count = idcu_vector_size(&manager->alerts);
    for (size_t i = 0; i < count; i++) {
        idcu_Alert* alert = (idcu_Alert*)idcu_vector_at(&manager->alerts, i);
        if (strcmp(alert->id, id) == 0) {
            idcu_hash_map_remove(&manager->alerts_by_id, id);
            idcu_vector_erase(&manager->alerts, i);
            idcu_mutex_unlock(&manager->lock);
            return IDCU_ERR_OK;
        }
    }
    idcu_mutex_unlock(&manager->lock);
    return IDCU_ERR_NOT_FOUND;
}

idcu_Alert* idcu_alert_manager_get_alert(idcu_AlertManager* manager, const char* id) {
    if (!manager || !manager->initialized || !id) {
        return NULL;
    }
    idcu_mutex_lock(&manager->lock);
    idcu_Alert* alert = (idcu_Alert*)idcu_hash_map_get_ptr(&manager->alerts_by_id, id);
    idcu_mutex_unlock(&manager->lock);
    return alert;
}

static int update_alert_status(idcu_AlertManager* manager, const char* id, idcu_AlertStatus new_status) {
    if (!manager || !manager->initialized || !id) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&manager->lock);
    idcu_Alert* alert = (idcu_Alert*)idcu_hash_map_get_ptr(&manager->alerts_by_id, id);
    if (alert) {
        alert->status = new_status;
        alert->updated_at = get_time_ms();
        idcu_mutex_unlock(&manager->lock);
        return IDCU_ERR_OK;
    }
    idcu_mutex_unlock(&manager->lock);
    return IDCU_ERR_NOT_FOUND;
}

int idcu_alert_manager_acknowledge_alert(idcu_AlertManager* manager, const char* id) {
    return update_alert_status(manager, id, IDCU_ALERT_STATUS_ACKNOWLEDGED);
}

int idcu_alert_manager_resolve_alert(idcu_AlertManager* manager, const char* id) {
    return update_alert_status(manager, id, IDCU_ALERT_STATUS_RESOLVED);
}

int idcu_alert_manager_close_alert(idcu_AlertManager* manager, const char* id) {
    return update_alert_status(manager, id, IDCU_ALERT_STATUS_CLOSED);
}

int idcu_alert_manager_add_channel(idcu_AlertManager* manager, const idcu_AlertChannel* channel) {
    if (!manager || !manager->initialized || !channel) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&manager->lock);
    int ret = idcu_vector_push_back(&manager->channels, channel);
    idcu_mutex_unlock(&manager->lock);
    return ret;
}

int idcu_alert_manager_remove_channel(idcu_AlertManager* manager, const char* name) {
    if (!manager || !manager->initialized || !name) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&manager->lock);
    size_t count = idcu_vector_size(&manager->channels);
    for (size_t i = 0; i < count; i++) {
        idcu_AlertChannel* channel = (idcu_AlertChannel*)idcu_vector_at(&manager->channels, i);
        if (strcmp(channel->name, name) == 0) {
            idcu_vector_erase(&manager->channels, i);
            idcu_mutex_unlock(&manager->lock);
            return IDCU_ERR_OK;
        }
    }
    idcu_mutex_unlock(&manager->lock);
    return IDCU_ERR_NOT_FOUND;
}

int idcu_alert_manager_send_alert(idcu_AlertManager* manager, const idcu_Alert* alert) {
    if (!manager || !manager->initialized || !alert) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&manager->lock);
    size_t count = idcu_vector_size(&manager->channels);
    for (size_t i = 0; i < count; i++) {
        idcu_AlertChannel* channel = (idcu_AlertChannel*)idcu_vector_at(&manager->channels, i);
        if (channel->enabled && channel->send_func && alert->severity >= channel->min_severity) {
            channel->send_func(channel->user_data, alert);
        }
    }
    idcu_mutex_unlock(&manager->lock);
    return IDCU_ERR_OK;
}

int idcu_alert_manager_get_alerts_by_severity(idcu_AlertManager* manager, idcu_AlertSeverity severity, idcu_Vector* results) {
    if (!manager || !manager->initialized || !results) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&manager->lock);
    size_t count = idcu_vector_size(&manager->alerts);
    for (size_t i = 0; i < count; i++) {
        idcu_Alert* alert = (idcu_Alert*)idcu_vector_at(&manager->alerts, i);
        if (alert->severity == severity) {
            idcu_vector_push_back(results, alert);
        }
    }
    idcu_mutex_unlock(&manager->lock);
    return IDCU_ERR_OK;
}

int idcu_alert_manager_get_alerts_by_status(idcu_AlertManager* manager, idcu_AlertStatus status, idcu_Vector* results) {
    if (!manager || !manager->initialized || !results) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&manager->lock);
    size_t count = idcu_vector_size(&manager->alerts);
    for (size_t i = 0; i < count; i++) {
        idcu_Alert* alert = (idcu_Alert*)idcu_vector_at(&manager->alerts, i);
        if (alert->status == status) {
            idcu_vector_push_back(results, alert);
        }
    }
    idcu_mutex_unlock(&manager->lock);
    return IDCU_ERR_OK;
}
