#ifndef IDCU_MODULE_MODULE_MANAGER_H
#define IDCU_MODULE_MODULE_MANAGER_H

#include "module_def.h"
#include "idcu/common/error_code.h"
#include "idcu/common/lock.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // IDCU_MODULE_MODULE_MANAGER_H

#define IDCU_MAX_MODULES 128
#define IDCU_MAX_DEPENDENCIES 32
#define IDCU_MODULE_NAME_MAX 64
#define IDCU_MODULE_VERSION_MAX 32
#define IDCU_MODULE_DESCRIPTION_MAX 256

typedef enum {
    IDCU_MODULE_EVENT_INIT_START = 0,
    IDCU_MODULE_EVENT_INIT_DONE,
    IDCU_MODULE_EVENT_INIT_FAIL,
    IDCU_MODULE_EVENT_START,
    IDCU_MODULE_EVENT_STOP,
    IDCU_MODULE_EVENT_ERROR,
    IDCU_MODULE_EVENT_UNLOAD
} idcu_ModuleEvent;

typedef void (*idcu_ModuleEventHandler)(uint32_t module_id, idcu_ModuleEvent event, void* user_data);

typedef struct {
    char dependency_name[IDCU_MODULE_NAME_MAX];
    char required_version[IDCU_MODULE_VERSION_MAX];
} idcu_ModuleDependency;

typedef struct {
    char name[IDCU_MODULE_NAME_MAX];
    char version[IDCU_MODULE_VERSION_MAX];
    char description[IDCU_MODULE_DESCRIPTION_MAX];
    idcu_ModuleInterface* iface;
    idcu_ModuleState state;
    idcu_ModulePrio priority;
    uint32_t module_id;
    uint32_t ref_count;
    uint64_t created_time;
    uint64_t last_error_time;
    int last_error_code;
    char last_error_message[256];
    idcu_ModuleDependency dependencies[IDCU_MAX_DEPENDENCIES];
    int dependency_count;
    idcu_ModuleEventHandler event_handler;
    void* event_handler_user_data;
    void* user_data;
} idcu_ModuleInfo;

typedef struct {
    idcu_ModuleInfo modules[IDCU_MAX_MODULES];
    uint32_t count;
    uint32_t next_id;
    idcu_Mutex lock;
    int initialized;
    int running;
} idcu_ModuleManager;

int idcu_module_manager_init(idcu_ModuleManager* manager);
void idcu_module_manager_destroy(idcu_ModuleManager* manager);

int idcu_module_register(idcu_ModuleManager* manager, const char* name, const char* version, const char* description, idcu_ModuleInterface* iface, idcu_ModulePrio priority);
int idcu_module_register_with_deps(idcu_ModuleManager* manager, const char* name, const char* version, const char* description, idcu_ModuleInterface* iface, idcu_ModulePrio priority, const idcu_ModuleDependency* dependencies, int dep_count);

int idcu_module_unregister(idcu_ModuleManager* manager, uint32_t module_id);
int idcu_module_unregister_by_name(idcu_ModuleManager* manager, const char* name);

idcu_ModuleInfo* idcu_module_find_by_id(idcu_ModuleManager* manager, uint32_t module_id);
idcu_ModuleInfo* idcu_module_find_by_name(idcu_ModuleManager* manager, const char* name);

int idcu_module_get_count(idcu_ModuleManager* manager);
idcu_ModuleInfo* idcu_module_get_at(idcu_ModuleManager* manager, uint32_t index);

int idcu_module_add_dependency(idcu_ModuleManager* manager, uint32_t module_id, const char* dep_name, const char* required_version);
int idcu_module_check_dependencies(idcu_ModuleManager* manager, uint32_t module_id);

int idcu_module_init(idcu_ModuleManager* manager, uint32_t module_id);
int idcu_module_run(idcu_ModuleManager* manager, uint32_t module_id);
int idcu_module_stop(idcu_ModuleManager* manager, uint32_t module_id);
int idcu_module_restart(idcu_ModuleManager* manager, uint32_t module_id);

int idcu_module_init_all(idcu_ModuleManager* manager);
int idcu_module_run_all(idcu_ModuleManager* manager);
int idcu_module_stop_all(idcu_ModuleManager* manager);
int idcu_module_shutdown(idcu_ModuleManager* manager);

int idcu_module_set_event_handler(idcu_ModuleManager* manager, uint32_t module_id, idcu_ModuleEventHandler handler, void* user_data);

idcu_ModuleState idcu_module_get_state(idcu_ModuleManager* manager, uint32_t module_id);
int idcu_module_get_error(idcu_ModuleManager* manager, uint32_t module_id, char* buffer, size_t buffer_size);
void idcu_module_clear_error(idcu_ModuleManager* manager, uint32_t module_id);

int idcu_module_version_parse(const char* version_str, uint32_t* major, uint32_t* minor, uint32_t* patch);
int idcu_module_version_compare(const char* v1, const char* v2);
int idcu_module_version_satisfies(const char* required, const char* actual);

#ifdef __cplusplus
}
#endif // IDCU_MODULE_MODULE_MANAGER_H

#endif // IDCU_MODULE_MODULE_MANAGER_H
