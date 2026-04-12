#ifndef IDCU_MODULE_SYSTEM_MODULE_SYSTEM_H
#define IDCU_MODULE_SYSTEM_MODULE_SYSTEM_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct idcu_ModuleSystem
{
    idcu_HashMap modules_by_name;
    idcu_HashMap modules_by_id;
    idcu_HashMap modules_by_category;
    idcu_Vector all_modules;
    uint32_t next_module_id;
    bool initialized;
};

int idcu_module_system_init(idcu_ModuleSystem* system);
void idcu_module_system_destroy(idcu_ModuleSystem* system);

int idcu_module_system_register(idcu_ModuleSystem* system, const idcu_ModuleDef* def);
int idcu_module_system_unregister(idcu_ModuleSystem* system, const char* name);

int idcu_module_system_find_by_name(idcu_ModuleSystem* system, const char* name, idcu_Module** out_module);
int idcu_module_system_find_by_id(idcu_ModuleSystem* system, uint32_t id, idcu_Module** out_module);
int idcu_module_system_get_all(idcu_ModuleSystem* system, idcu_ModuleInfo** out_infos, size_t* out_count);
int idcu_module_system_get_by_category(idcu_ModuleSystem* system, const char* category, idcu_ModuleInfo** out_infos, size_t* out_count);

int idcu_module_system_init_all(idcu_ModuleSystem* system);
int idcu_module_system_start_all(idcu_ModuleSystem* system);
int idcu_module_system_stop_all(idcu_ModuleSystem* system);
int idcu_module_system_destroy_all(idcu_ModuleSystem* system);

int idcu_module_system_init_module(idcu_ModuleSystem* system, const char* name);
int idcu_module_system_start_module(idcu_ModuleSystem* system, const char* name);
int idcu_module_system_stop_module(idcu_ModuleSystem* system, const char* name);
int idcu_module_system_destroy_module(idcu_ModuleSystem* system, const char* name);

int idcu_module_system_load_directory(idcu_ModuleSystem* system, const char* dir_path);
int idcu_module_system_load_module(idcu_ModuleSystem* system, const char* module_path);

const char* idcu_module_state_to_str(idcu_ModuleState state);

#ifdef __cplusplus
}
#endif

#endif
