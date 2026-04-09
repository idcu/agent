#ifndef IDCU_ALERT_ALERT_H
#define IDCU_ALERT_ALERT_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/alert/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_alert_init(idcu_Alert_Context** ctx);
void idcu_alert_destroy(idcu_Alert_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
