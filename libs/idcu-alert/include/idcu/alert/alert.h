#ifndef IDCU_ALERT_ALERT_H
#define IDCU_ALERT_ALERT_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/alert/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_alert_init(idcu_Alert_Context** ctx);
void idcu_alert_destroy(idcu_Alert_Context* ctx);
int idcu_alert_is_initialized(idcu_Alert_Context* ctx);
uint64_t idcu_alert_get_operation_count(idcu_Alert_Context* ctx);
uint64_t idcu_alert_get_error_count(idcu_Alert_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
