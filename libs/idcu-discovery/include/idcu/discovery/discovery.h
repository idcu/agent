#ifndef IDCU_DISCOVERY_DISCOVERY_H
#define IDCU_DISCOVERY_DISCOVERY_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/discovery/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_discovery_init(idcu_Discovery_Context** ctx);
void idcu_discovery_destroy(idcu_Discovery_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
