#ifndef IDCU_PLUGIN_PLUGIN_H
#define IDCU_PLUGIN_PLUGIN_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/plugin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_plugin_init(idcu_Plugin_Context** ctx);
void idcu_plugin_destroy(idcu_Plugin_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
