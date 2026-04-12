#ifndef IDCU_MODULE_SYSTEM_TYPES_H
#define IDCU_MODULE_SYSTEM_TYPES_H

#include <idcu/common/error_code.h>
#include <idcu/common/vector.h>
#include <idcu/common/hash_map.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct idcu_ModuleSystem idcu_ModuleSystem;

typedef enum
{
    IDCU_MODULE_STATE_UNREGISTERED = 0,
    IDCU_MODULE_STATE_REGISTERED   = 1,
    IDCU_MODULE_STATE_LOADED       = 2,
    IDCU_MODULE_STATE_INITIALIZED  = 3,
    IDCU_MODULE_STATE_STARTED      = 4,
    IDCU_MODULE_STATE_STOPPED      = 5,
    IDCU_MODULE_STATE_DESTROYED    = 6
} idcu_ModuleState;

typedef struct
{
    const char* name;
    const char* version;
} idcu_ModuleDependency;

typedef idcu_ErrorCode (*idcu_ModuleInitFunc)(void);
typedef idcu_ErrorCode (*idcu_ModuleStartFunc)(void);
typedef idcu_ErrorCode (*idcu_ModuleStopFunc)(void);
typedef idcu_ErrorCode (*idcu_ModuleDestroyFunc)(void);

typedef struct
{
    const char* name;
    const char* version_str;
    const char* description;
    const char* category;

    idcu_ModuleInitFunc init;
    idcu_ModuleStartFunc start;
    idcu_ModuleStopFunc stop;
    idcu_ModuleDestroyFunc destroy;

    const idcu_ModuleDependency* dependencies;
    size_t dependency_count;
} idcu_ModuleDef;

typedef struct
{
    const idcu_ModuleDef* def;
    idcu_ModuleState state;
    uint32_t module_id;
    void* user_data;
    void* library_handle;
} idcu_Module;

typedef struct
{
    const char* name;
    const char* version;
    uint32_t id;
    idcu_ModuleState state;
} idcu_ModuleInfo;

#ifdef __cplusplus
}
#endif

#endif
