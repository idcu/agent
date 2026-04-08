#ifndef IDCU_PLUGIN_PLUGIN_H
#define IDCU_PLUGIN_PLUGIN_H

#include "idcu/common/error_code.h"

#include <stddef.h>
#include <stdint.h>

#define IDCU_PLUGIN_NAME_MAX    128
#define IDCU_PLUGIN_VERSION_MAX 32
#define IDCU_PLUGIN_PATH_MAX    512

typedef struct idcu_PluginInfo
{
    char     name[IDCU_PLUGIN_NAME_MAX];
    char     version[IDCU_PLUGIN_VERSION_MAX];
    char     author[128];
    char     description[512];
    int      api_version;
    uint32_t flags;
} idcu_PluginInfo;

typedef struct idcu_PluginHandle idcu_PluginHandle;

typedef int (*idcu_PluginInitFunc)(void);
typedef void (*idcu_PluginCleanupFunc)(void);
typedef int (*idcu_PluginGetInfoFunc)(idcu_PluginInfo* info);

int  idcu_plugin_system_init(void);
void idcu_plugin_system_shutdown(void);

int idcu_plugin_load(const char* path, idcu_PluginHandle** handle);
int idcu_plugin_unload(idcu_PluginHandle* handle);

int idcu_plugin_get_info(idcu_PluginHandle* handle, idcu_PluginInfo* info);
int idcu_plugin_get_name(idcu_PluginHandle* handle, char* buffer, size_t buffer_size);
int idcu_plugin_is_loaded(const char* name);

int idcu_plugin_scan_directory(const char* directory);
int idcu_plugin_load_all(void);
int idcu_plugin_unload_all(void);

int idcu_plugin_get_count(void);
int idcu_plugin_get_all_names(char** names, size_t max_names, size_t* actual_count);

int idcu_plugin_get_symbol(idcu_PluginHandle* handle, const char* symbol_name, void** symbol);

#endif  // IDCU_PLUGIN_PLUGIN_H
