#ifndef IDCU_PLUGIN_PLUGIN_H
#define IDCU_PLUGIN_PLUGIN_H

#include "idcu/common/error_code.h"
#include "idcu/plugin/types.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_plugin_system_init(idcu_Plugin_Context** ctx);
void idcu_plugin_system_shutdown(idcu_Plugin_Context* ctx);

int idcu_plugin_load(idcu_Plugin_Context* ctx, const char* path, idcu_PluginHandle** handle);
int idcu_plugin_unload(idcu_Plugin_Context* ctx, idcu_PluginHandle* handle);

int idcu_plugin_get_info(idcu_PluginHandle* handle, idcu_PluginInfo* info);
int idcu_plugin_get_name(idcu_PluginHandle* handle, char* buffer, size_t buffer_size);
int idcu_plugin_is_loaded(idcu_Plugin_Context* ctx, const char* name);

int idcu_plugin_scan_directory(idcu_Plugin_Context* ctx, const char* directory);
int idcu_plugin_load_all(idcu_Plugin_Context* ctx);
int idcu_plugin_unload_all(idcu_Plugin_Context* ctx);

int idcu_plugin_get_count(idcu_Plugin_Context* ctx);
int idcu_plugin_get_all_names(idcu_Plugin_Context* ctx, char** names, size_t max_names, size_t* actual_count);

int idcu_plugin_get_symbol(idcu_PluginHandle* handle, const char* symbol_name, void** symbol);

#ifdef __cplusplus
}
#endif

#endif  // IDCU_PLUGIN_PLUGIN_H
