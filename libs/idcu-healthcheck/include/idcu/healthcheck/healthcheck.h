#ifndef IDCU_HEALTHCHECK_HEALTHCHECK_H
#define IDCU_HEALTHCHECK_HEALTHCHECK_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/healthcheck/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_healthcheck_init(idcu_Healthcheck_Context** ctx);
void idcu_healthcheck_destroy(idcu_Healthcheck_Context* ctx);
int idcu_healthcheck_is_initialized(idcu_Healthcheck_Context* ctx);
uint64_t idcu_healthcheck_get_operation_count(idcu_Healthcheck_Context* ctx);
uint64_t idcu_healthcheck_get_error_count(idcu_Healthcheck_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
