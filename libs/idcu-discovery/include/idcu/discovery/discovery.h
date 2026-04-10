#ifndef IDCU_DISCOVERY_DISCOVERY_H
#define IDCU_DISCOVERY_DISCOVERY_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/discovery/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_discovery_init(idcu_Discovery_Context** ctx);
void idcu_discovery_destroy(idcu_Discovery_Context* ctx);
int idcu_discovery_is_initialized(idcu_Discovery_Context* ctx);
uint64_t idcu_discovery_get_operation_count(idcu_Discovery_Context* ctx);
uint64_t idcu_discovery_get_error_count(idcu_Discovery_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
