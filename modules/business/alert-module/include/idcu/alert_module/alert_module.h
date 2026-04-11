#ifndef IDCU_ALERT_MODULE_ALERT_MODULE_H
#define IDCU_ALERT_MODULE_ALERT_MODULE_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/alert/alert.h>
#include <idcu/sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    idcu_AlertManager* alert_manager;
    int initialized;
    void* user_data;
} idcu_AlertModule;

int idcu_alert_module_init(idcu_AlertModule* am);
int idcu_alert_module_start(idcu_AlertModule* am);
int idcu_alert_module_stop(idcu_AlertModule* am);
void idcu_alert_module_destroy(idcu_AlertModule* am);

int idcu_alert_module_trigger(idcu_AlertModule* am, const char* event_name, void* data);
idcu_AlertManager* idcu_alert_module_get_alert_manager(idcu_AlertModule* am);

#ifdef __cplusplus
}
#endif

#endif
