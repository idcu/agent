#ifndef IDCU_MODULE_MODULE_CATEGORY_H
#define IDCU_MODULE_MODULE_CATEGORY_H

#include "idcu/common/config.h"
#include "idcu/common/error_code.h"
#include "module_def.h"
#include <stdint.h>

#define IDCU_MAX_CATEGORIES 16
#define IDCU_CATEGORY_NAME_MAX 64
#define IDCU_MAX_MODULES_PER_CATEGORY 32
#define IDCU_MAX_REGISTERED_MODULES 64

typedef enum {
    IDCU_CAT_LEVEL_CORE = 0,
    IDCU_CAT_LEVEL_FOUNDATION = 1,
    IDCU_CAT_LEVEL_SERVICE = 2,
    IDCU_CAT_LEVEL_BUSINESS = 3,
    IDCU_CAT_LEVEL_EXTENSION = 4
} idcu_CategoryLevel;

typedef struct {
    char name[IDCU_CATEGORY_NAME_MAX];
    char description[128];
    idcu_CategoryLevel level;
    uint32_t module_ids[IDCU_MAX_MODULES_PER_CATEGORY];
    uint32_t module_count;
} idcu_ModuleCategory;

typedef struct {
    char module_name[IDCU_MODULE_NAME_MAX];
    char category_name[IDCU_CATEGORY_NAME_MAX];
    int enabled;
    idcu_ModulePrio priority;
    char config_prefix[IDCU_CONFIG_SECTION_MAX];
} idcu_ModuleConfig;

typedef struct {
    idcu_ModuleCategory categories[IDCU_MAX_CATEGORIES];
    uint32_t category_count;
    idcu_ModuleConfig module_configs[IDCU_MAX_REGISTERED_MODULES];
    uint32_t module_config_count;
    int initialized;
} idcu_ModuleCategoryManager;

int idcu_module_category_manager_init(idcu_ModuleCategoryManager* mgr);
void idcu_module_category_manager_destroy(idcu_ModuleCategoryManager* mgr);

int idcu_module_category_register(idcu_ModuleCategoryManager* mgr, 
                                   const char* name, 
                                   const char* description,
                                   idcu_CategoryLevel level);

int idcu_module_category_add_module(idcu_ModuleCategoryManager* mgr,
                                     const char* category_name,
                                     uint32_t module_id);

int idcu_module_category_load_config(idcu_ModuleCategoryManager* mgr,
                                      const char* config_file);

int idcu_module_category_is_module_enabled(idcu_ModuleCategoryManager* mgr,
                                            const char* module_name);

idcu_ModulePrio idcu_module_category_get_module_priority(idcu_ModuleCategoryManager* mgr,
                                                           const char* module_name);

const char* idcu_module_category_get_config_prefix(idcu_ModuleCategoryManager* mgr,
                                                     const char* module_name);

int idcu_module_category_get_modules_by_level(idcu_ModuleCategoryManager* mgr,
                                               idcu_CategoryLevel level,
                                               uint32_t* module_ids,
                                               uint32_t* count);

const idcu_ModuleCategory* idcu_module_category_find(idcu_ModuleCategoryManager* mgr,
                                                       const char* name);

#endif
