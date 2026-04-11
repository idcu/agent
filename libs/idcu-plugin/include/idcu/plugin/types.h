#ifndef IDCU_PLUGIN_TYPES_H
#define IDCU_PLUGIN_TYPES_H

#include <idcu/common/config.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_PLUGIN_NAME_MAX    128
#define IDCU_PLUGIN_VERSION_MAX 32
#define IDCU_PLUGIN_PATH_MAX    512
#define IDCU_PLUGIN_MAX_ITEMS   1024

typedef struct idcu_PluginInfo {
    char     name[IDCU_PLUGIN_NAME_MAX];
    char     version[IDCU_PLUGIN_VERSION_MAX];
    char     author[128];
    char     description[512];
    int      api_version;
    uint32_t flags;
} idcu_PluginInfo;

typedef struct idcu_PluginHandle {
    void*      handle;
    char       path[IDCU_PLUGIN_PATH_MAX];
    idcu_PluginInfo info;
    int        loaded;
} idcu_PluginHandle;

typedef struct idcu_Plugin_Context {
    idcu_PluginHandle* plugins[IDCU_PLUGIN_MAX_ITEMS];
    int                 plugin_count;
    int                 initialized;
    idcu_Mutex          lock;
    uint64_t            operation_count;
    uint64_t            error_count;
} idcu_Plugin_Context;

typedef int (*idcu_PluginInitFunc)(void);
typedef void (*idcu_PluginCleanupFunc)(void);
typedef int (*idcu_PluginGetInfoFunc)(idcu_PluginInfo* info);

#ifdef __cplusplus
}
#endif

#endif
