#ifndef IDCU_CONFIG_CONFIG_H
#define IDCU_CONFIG_CONFIG_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/config/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_config_init(idcu_Config_Context** ctx);
void idcu_config_destroy(idcu_Config_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
