#include <idcu/alert_module/alert_module.h>
#include <string.h>

int idcu_alert_module_init(idcu_AlertModule* am) {
    if (!am) {
        return IDCU_ERR_INVALID_ARG;
    }

    memset(am, 0, sizeof(idcu_AlertModule));
    am->initialized = 0;

    int ret = idcu_alert_manager_create(&am->alert_manager);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    am->initialized = 1;
    return IDCU_ERR_OK;
}

int idcu_alert_module_start(idcu_AlertModule* am) {
    if (!am || !am->initialized) {
        return IDCU_ERR_INVALID_STATE;
    }
    return IDCU_ERR_OK;
}

int idcu_alert_module_stop(idcu_AlertModule* am) {
    if (!am || !am->initialized) {
        return IDCU_ERR_INVALID_STATE;
    }
    return IDCU_ERR_OK;
}

void idcu_alert_module_destroy(idcu_AlertModule* am) {
    if (!am) {
        return;
    }

    if (am->alert_manager) {
        idcu_alert_manager_destroy(am->alert_manager);
        am->alert_manager = NULL;
    }

    am->initialized = 0;
    memset(am, 0, sizeof(idcu_AlertModule));
}

int idcu_alert_module_trigger(idcu_AlertModule* am, const char* event_name, void* data) {
    if (!am || !am->alert_manager || !event_name) {
        return IDCU_ERR_INVALID_ARG;
    }
    (void)data;
    return IDCU_ERR_OK;
}

idcu_AlertManager* idcu_alert_module_get_alert_manager(idcu_AlertModule* am) {
    if (!am) {
        return NULL;
    }
    return am->alert_manager;
}
