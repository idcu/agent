#ifndef IDCU_MODULE_SYSTEM_MODULE_SYSTEM_H
#define IDCU_MODULE_SYSTEM_MODULE_SYSTEM_H

#include "idcu/common/error_code.h"

#include <stddef.h>
#include <stdint.h>

#define IDCU_MODULE_NAME_MAX    128
#define IDCU_MODULE_VERSION_MAX 32

typedef struct idcu_ModuleInfo
{
    char name[IDCU_MODULE_NAME_MAX];
    char version[IDCU_MODULE_VERSION_MAX];
    char author[128];
    char description[512];
    int  priority;
} idcu_ModuleInfo;

typedef struct idcu_ModuleHandle idcu_ModuleHandle;

typedef int (*idcu_ModuleInitFunc)(void);
typedef void (*idcu_ModuleCleanupFunc)(void);
typedef int (*idcu_ModuleUpdateFunc)(uint64_t delta_ms);

int  idcu_module_system_init(void);
void idcu_module_system_shutdown(void);

int idcu_module_register(const char* name, const idcu_ModuleInfo* info, idcu_ModuleInitFunc init,
                         idcu_ModuleCleanupFunc cleanup, idcu_ModuleUpdateFunc update);
int idcu_module_unregister(const char* name);

int idcu_module_load(const char* name, idcu_ModuleHandle** handle);
int idcu_module_unload(idcu_ModuleHandle* handle);

int idcu_module_start(idcu_ModuleHandle* handle);
int idcu_module_stop(idcu_ModuleHandle* handle);
int idcu_module_restart(idcu_ModuleHandle* handle);

int idcu_module_is_loaded(const char* name);
int idcu_module_is_running(const char* name);

int idcu_module_get_info(const char* name, idcu_ModuleInfo* info);
int idcu_module_get_count(void);
int idcu_module_get_all_names(char** names, size_t max_names, size_t* actual_count);

int idcu_module_system_update(uint64_t delta_ms);
int idcu_module_system_save_state(const char* file_path);
int idcu_module_system_load_state(const char* file_path);

#endif  // IDCU_MODULE_SYSTEM_MODULE_SYSTEM_H
