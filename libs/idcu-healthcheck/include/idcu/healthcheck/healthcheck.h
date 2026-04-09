#ifndef IDCU_HEALTHCHECK_HEALTHCHECK_H
#define IDCU_HEALTHCHECK_HEALTHCHECK_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/healthcheck/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_healthcheck_init(idcu_Healthcheck_Context** ctx);
void idcu_healthcheck_destroy(idcu_Healthcheck_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
