#include <idcu/module/module.h>
#include <idcu/common/error_code.h>
#include <idcu/common/vector.h>
#include <idcu/common/hash_map.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

static void module_dtor(void* data)
{
    idcu_Module* module = (idcu_Module*)data;
    if (module->library_handle)
    {
#ifdef _WIN32
        FreeLibrary((HMODULE)module->library_handle);
#else
        dlclose(module->library_handle);
#endif
    }
}

int idcu_module_system_init(idcu_ModuleSystem* system)
{
    if (!system)
    {
        return IDCU_ERR_INVALID_PARAM;
    }

    int err;

    err = idcu_hash_map_init(&system->modules_by_name, 32, sizeof(idcu_Module*));
    if (err != IDCU_SUCCESS)
    {
        return err;
    }

    err = idcu_hash_map_init(&system->modules_by_id, 32, sizeof(idcu_Module*));
    if (err != IDCU_SUCCESS)
    {
        idcu_hash_map_destroy(&system->modules_by_name);
        return err;
    }

    err = idcu_hash_map_init(&system->modules_by_category, 32, sizeof(idcu_Vector*));
    if (err != IDCU_SUCCESS)
    {
        idcu_hash_map_destroy(&system->modules_by_name);
        idcu_hash_map_destroy(&system->modules_by_id);
        return err;
    }

    err = idcu_vector_init_with_dtor(&system->all_modules, sizeof(idcu_Module), 8, module_dtor);
    if (err != IDCU_SUCCESS)
    {
        idcu_hash_map_destroy(&system->modules_by_name);
        idcu_hash_map_destroy(&system->modules_by_id);
        idcu_hash_map_destroy(&system->modules_by_category);
        return err;
    }

    system->next_module_id = 1;
    system->initialized = true;

    return IDCU_SUCCESS;
}

void idcu_module_system_destroy(idcu_ModuleSystem* system)
{
    if (!system || !system->initialized)
    {
        return;
    }

    idcu_module_system_destroy_all(system);

    idcu_vector_destroy(&system->all_modules);
    idcu_hash_map_destroy(&system->modules_by_name);
    idcu_hash_map_destroy(&system->modules_by_id);
    idcu_hash_map_destroy(&system->modules_by_category);

    system->initialized = false;
}

int idcu_module_system_register(idcu_ModuleSystem* system, const idcu_ModuleDef* def)
{
    if (!system || !system->initialized || !def)
    {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (idcu_hash_map_contains(&system->modules_by_name, def->name))
    {
        return IDCU_ERR_ALREADY_EXISTS;
    }

    idcu_Module module;
    memset(&module, 0, sizeof(module));
    module.def = def;
    module.state = IDCU_MODULE_STATE_REGISTERED;
    module.module_id = system->next_module_id++;
    module.user_data = NULL;
    module.library_handle = NULL;

    int err = idcu_vector_push_back(&system->all_modules, &module);
    if (err != IDCU_SUCCESS)
    {
        return err;
    }

    idcu_Module* module_ptr = (idcu_Module*)idcu_vector_get(&system->all_modules, idcu_vector_size(&system->all_modules) - 1);

    err = idcu_hash_map_put(&system->modules_by_name, def->name, &module_ptr);
    if (err != IDCU_SUCCESS)
    {
        idcu_vector_pop_back(&system->all_modules, NULL);
        return err;
    }

    char id_str[32];
    snprintf(id_str, sizeof(id_str), "%u", module_ptr->module_id);
    err = idcu_hash_map_put(&system->modules_by_id, id_str, &module_ptr);
    if (err != IDCU_SUCCESS)
    {
        idcu_hash_map_remove(&system->modules_by_name, def->name);
        idcu_vector_pop_back(&system->all_modules, NULL);
        return err;
    }

    if (def->category)
    {
        idcu_Vector* category_modules = NULL;
        if (!idcu_hash_map_contains(&system->modules_by_category, def->category))
        {
            idcu_Vector new_vec;
            err = idcu_vector_init(&new_vec, sizeof(idcu_Module*), 4);
            if (err != IDCU_SUCCESS)
            {
                idcu_hash_map_remove(&system->modules_by_name, def->name);
                idcu_hash_map_remove(&system->modules_by_id, id_str);
                idcu_vector_pop_back(&system->all_modules, NULL);
                return err;
            }

            err = idcu_hash_map_put(&system->modules_by_category, def->category, &new_vec);
            if (err != IDCU_SUCCESS)
            {
                idcu_vector_destroy(&new_vec);
                idcu_hash_map_remove(&system->modules_by_name, def->name);
                idcu_hash_map_remove(&system->modules_by_id, id_str);
                idcu_vector_pop_back(&system->all_modules, NULL);
                return err;
            }
        }

        idcu_hash_map_get(&system->modules_by_category, def->category, &category_modules);
        if (category_modules)
        {
            err = idcu_vector_push_back(category_modules, &module_ptr);
            if (err != IDCU_SUCCESS)
            {
                return err;
            }
        }
    }

    return IDCU_SUCCESS;
}

int idcu_module_system_unregister(idcu_ModuleSystem* system, const char* name)
{
    (void)system;
    (void)name;
    return IDCU_ERR_GENERAL;
}

int idcu_module_system_find_by_name(idcu_ModuleSystem* system, const char* name, idcu_Module** out_module)
{
    if (!system || !system->initialized || !name || !out_module)
    {
        return IDCU_ERR_INVALID_PARAM;
    }

    return idcu_hash_map_get(&system->modules_by_name, name, out_module);
}

int idcu_module_system_find_by_id(idcu_ModuleSystem* system, uint32_t id, idcu_Module** out_module)
{
    if (!system || !system->initialized || !out_module)
    {
        return IDCU_ERR_INVALID_PARAM;
    }

    char id_str[32];
    snprintf(id_str, sizeof(id_str), "%u", id);
    return idcu_hash_map_get(&system->modules_by_id, id_str, out_module);
}

int idcu_module_system_get_all(idcu_ModuleSystem* system, idcu_ModuleInfo** out_infos, size_t* out_count)
{
    if (!system || !system->initialized || !out_infos || !out_count)
    {
        return IDCU_ERR_INVALID_PARAM;
    }

    size_t count = idcu_vector_size(&system->all_modules);
    *out_count = count;

    if (count == 0)
    {
        *out_infos = NULL;
        return IDCU_SUCCESS;
    }

    *out_infos = (idcu_ModuleInfo*)malloc(count * sizeof(idcu_ModuleInfo));
    if (!*out_infos)
    {
        return IDCU_ERR_NO_MEMORY;
    }

    for (size_t i = 0; i < count; ++i)
    {
        idcu_Module* module = (idcu_Module*)idcu_vector_get(&system->all_modules, i);
        (*out_infos)[i].name = module->def->name;
        (*out_infos)[i].version = module->def->version_str;
        (*out_infos)[i].id = module->module_id;
        (*out_infos)[i].state = module->state;
    }

    return IDCU_SUCCESS;
}

int idcu_module_system_get_by_category(idcu_ModuleSystem* system, const char* category, idcu_ModuleInfo** out_infos, size_t* out_count)
{
    (void)system;
    (void)category;
    (void)out_infos;
    (void)out_count;
    return IDCU_ERR_GENERAL;
}

static int check_dependencies(idcu_ModuleSystem* system, const idcu_ModuleDef* def)
{
    for (size_t i = 0; i < def->dependency_count; ++i)
    {
        idcu_Module* dep_module = NULL;
        int err = idcu_module_system_find_by_name(system, def->dependencies[i].name, &dep_module);
        if (err != IDCU_SUCCESS)
        {
            IDCU_ERR_SET(IDCU_ERR_DEPENDENCY, def->dependencies[i].name);
            return IDCU_ERR_DEPENDENCY;
        }
    }
    return IDCU_SUCCESS;
}

int idcu_module_system_init_module(idcu_ModuleSystem* system, const char* name)
{
    if (!system || !system->initialized || !name)
    {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_Module* module = NULL;
    int err = idcu_module_system_find_by_name(system, name, &module);
    if (err != IDCU_SUCCESS)
    {
        return err;
    }

    if (module->state != IDCU_MODULE_STATE_REGISTERED && module->state != IDCU_MODULE_STATE_LOADED)
    {
        return IDCU_ERR_MODULE_INVALID;
    }

    err = check_dependencies(system, module->def);
    if (err != IDCU_SUCCESS)
    {
        return err;
    }

    if (module->def->init)
    {
        err = module->def->init();
        if (err != IDCU_SUCCESS)
        {
            IDCU_ERR_SET(err, name);
            return err;
        }
    }

    module->state = IDCU_MODULE_STATE_INITIALIZED;
    return IDCU_SUCCESS;
}

int idcu_module_system_start_module(idcu_ModuleSystem* system, const char* name)
{
    if (!system || !system->initialized || !name)
    {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_Module* module = NULL;
    int err = idcu_module_system_find_by_name(system, name, &module);
    if (err != IDCU_SUCCESS)
    {
        return err;
    }

    if (module->state != IDCU_MODULE_STATE_INITIALIZED && module->state != IDCU_MODULE_STATE_STOPPED)
    {
        return IDCU_ERR_MODULE_INVALID;
    }

    if (module->def->start)
    {
        err = module->def->start();
        if (err != IDCU_SUCCESS)
        {
            IDCU_ERR_SET(err, name);
            return err;
        }
    }

    module->state = IDCU_MODULE_STATE_STARTED;
    return IDCU_SUCCESS;
}

int idcu_module_system_stop_module(idcu_ModuleSystem* system, const char* name)
{
    if (!system || !system->initialized || !name)
    {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_Module* module = NULL;
    int err = idcu_module_system_find_by_name(system, name, &module);
    if (err != IDCU_SUCCESS)
    {
        return err;
    }

    if (module->state != IDCU_MODULE_STATE_STARTED)
    {
        return IDCU_ERR_MODULE_INVALID;
    }

    if (module->def->stop)
    {
        err = module->def->stop();
        if (err != IDCU_SUCCESS)
        {
            IDCU_ERR_SET(err, name);
            return err;
        }
    }

    module->state = IDCU_MODULE_STATE_STOPPED;
    return IDCU_SUCCESS;
}

int idcu_module_system_destroy_module(idcu_ModuleSystem* system, const char* name)
{
    if (!system || !system->initialized || !name)
    {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_Module* module = NULL;
    int err = idcu_module_system_find_by_name(system, name, &module);
    if (err != IDCU_SUCCESS)
    {
        return err;
    }

    if (module->state == IDCU_MODULE_STATE_STARTED)
    {
        err = idcu_module_system_stop_module(system, name);
        if (err != IDCU_SUCCESS)
        {
            return err;
        }
    }

    if (module->def->destroy)
    {
        err = module->def->destroy();
        if (err != IDCU_SUCCESS)
        {
            IDCU_ERR_SET(err, name);
            return err;
        }
    }

    module->state = IDCU_MODULE_STATE_DESTROYED;
    return IDCU_SUCCESS;
}

int idcu_module_system_init_all(idcu_ModuleSystem* system)
{
    if (!system || !system->initialized)
    {
        return IDCU_ERR_INVALID_PARAM;
    }

    size_t count = idcu_vector_size(&system->all_modules);
    for (size_t i = 0; i < count; ++i)
    {
        idcu_Module* module = (idcu_Module*)idcu_vector_get(&system->all_modules, i);
        if (module->state == IDCU_MODULE_STATE_REGISTERED || module->state == IDCU_MODULE_STATE_LOADED)
        {
            int err = idcu_module_system_init_module(system, module->def->name);
            if (err != IDCU_SUCCESS)
            {
                return err;
            }
        }
    }
    return IDCU_SUCCESS;
}

int idcu_module_system_start_all(idcu_ModuleSystem* system)
{
    if (!system || !system->initialized)
    {
        return IDCU_ERR_INVALID_PARAM;
    }

    size_t count = idcu_vector_size(&system->all_modules);
    for (size_t i = 0; i < count; ++i)
    {
        idcu_Module* module = (idcu_Module*)idcu_vector_get(&system->all_modules, i);
        if (module->state == IDCU_MODULE_STATE_INITIALIZED || module->state == IDCU_MODULE_STATE_STOPPED)
        {
            int err = idcu_module_system_start_module(system, module->def->name);
            if (err != IDCU_SUCCESS)
            {
                return err;
            }
        }
    }
    return IDCU_SUCCESS;
}

int idcu_module_system_stop_all(idcu_ModuleSystem* system)
{
    if (!system || !system->initialized)
    {
        return IDCU_ERR_INVALID_PARAM;
    }

    size_t count = idcu_vector_size(&system->all_modules);
    for (size_t i = count; i > 0; --i)
    {
        idcu_Module* module = (idcu_Module*)idcu_vector_get(&system->all_modules, i - 1);
        if (module->state == IDCU_MODULE_STATE_STARTED)
        {
            int err = idcu_module_system_stop_module(system, module->def->name);
            if (err != IDCU_SUCCESS)
            {
                return err;
            }
        }
    }
    return IDCU_SUCCESS;
}

int idcu_module_system_destroy_all(idcu_ModuleSystem* system)
{
    if (!system || !system->initialized)
    {
        return IDCU_ERR_INVALID_PARAM;
    }

    size_t count = idcu_vector_size(&system->all_modules);
    for (size_t i = count; i > 0; --i)
    {
        idcu_Module* module = (idcu_Module*)idcu_vector_get(&system->all_modules, i - 1);
        if (module->state != IDCU_MODULE_STATE_UNREGISTERED && module->state != IDCU_MODULE_STATE_DESTROYED)
        {
            int err = idcu_module_system_destroy_module(system, module->def->name);
            if (err != IDCU_SUCCESS)
            {
                return err;
            }
        }
    }
    return IDCU_SUCCESS;
}

int idcu_module_system_load_module(idcu_ModuleSystem* system, const char* module_path)
{
    (void)system;
    (void)module_path;
    return IDCU_ERR_GENERAL;
}

int idcu_module_system_load_directory(idcu_ModuleSystem* system, const char* dir_path)
{
    (void)system;
    (void)dir_path;
    return IDCU_ERR_GENERAL;
}

const char* idcu_module_state_to_str(idcu_ModuleState state)
{
    switch (state)
    {
        case IDCU_MODULE_STATE_UNREGISTERED: return "UNREGISTERED";
        case IDCU_MODULE_STATE_REGISTERED:   return "REGISTERED";
        case IDCU_MODULE_STATE_LOADED:       return "LOADED";
        case IDCU_MODULE_STATE_INITIALIZED:  return "INITIALIZED";
        case IDCU_MODULE_STATE_STARTED:      return "STARTED";
        case IDCU_MODULE_STATE_STOPPED:      return "STOPPED";
        case IDCU_MODULE_STATE_DESTROYED:    return "DESTROYED";
        default:                              return "UNKNOWN";
    }
}
