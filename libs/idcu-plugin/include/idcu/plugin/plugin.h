#ifndef IDCU_PLUGIN_PLUGIN_H
#define IDCU_PLUGIN_PLUGIN_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/plugin/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_plugin_init(idcu_Plugin_Context** ctx);
void idcu_plugin_destroy(idcu_Plugin_Context* ctx);
int idcu_plugin_is_initialized(idcu_Plugin_Context* ctx);
uint64_t idcu_plugin_get_operation_count(idcu_Plugin_Context* ctx);
uint64_t idcu_plugin_get_error_count(idcu_Plugin_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
