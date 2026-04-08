#include "module_category.h"
#include "idcu/config/config.h"
#include "idcu/log/log.h"
#include <string.h>

static const char *s_level_names[] = {"core", "foundation", "service", "business", "extension"};

int idcu_module_category_manager_init(idcu_ModuleCategoryManager *mgr) {
    if (!mgr) {
        return IDCU_ERR_INVALID_PARAM;
    }
    memset(mgr, 0, sizeof(idcu_ModuleCategoryManager));
    mgr->initialized = 1;

    idcu_module_category_register(mgr, "core", "核心基础模块", IDCU_CAT_LEVEL_CORE);
    idcu_module_category_register(mgr, "foundation", "基础服务模块", IDCU_CAT_LEVEL_FOUNDATION);
    idcu_module_category_register(mgr, "monitoring", "监控相关模块", IDCU_CAT_LEVEL_SERVICE);
    idcu_module_category_register(mgr, "networking", "网络相关模块", IDCU_CAT_LEVEL_SERVICE);
    idcu_module_category_register(mgr, "security", "安全相关模块", IDCU_CAT_LEVEL_SERVICE);
    idcu_module_category_register(mgr, "business", "业务功能模块", IDCU_CAT_LEVEL_BUSINESS);
    idcu_module_category_register(mgr, "extension", "扩展功能模块", IDCU_CAT_LEVEL_EXTENSION);

    IDCU_LOG_INFO("module category manager initialized");
    return IDCU_ERR_SUCCESS;
}

void idcu_module_category_manager_destroy(idcu_ModuleCategoryManager *mgr) {
    if (!mgr) {
        return;
    }
    memset(mgr, 0, sizeof(idcu_ModuleCategoryManager));
}

int idcu_module_category_register(idcu_ModuleCategoryManager *mgr, const char *name,
                                  const char *description, idcu_CategoryLevel level) {
    if (!mgr || !name || !mgr->initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }

    for (uint32_t i = 0; i < mgr->category_count; i++) {
        if (strcmp(mgr->categories[i].name, name) == 0) {
            return IDCU_ERR_ALREADY_EXISTS;
        }
    }

    if (mgr->category_count >= IDCU_MAX_CATEGORIES) {
        return IDCU_ERR_QUEUE_FULL;
    }

    idcu_ModuleCategory *cat = &mgr->categories[mgr->category_count];
    strncpy(cat->name, name, IDCU_CATEGORY_NAME_MAX - 1);
    strncpy(cat->description, description, 127);
    cat->level = level;
    cat->module_count = 0;
    mgr->category_count++;

    IDCU_LOG_INFO("category registered: %s (level: %s)", name, s_level_names[level]);
    return IDCU_ERR_SUCCESS;
}

int idcu_module_category_add_module(idcu_ModuleCategoryManager *mgr, const char *category_name,
                                    uint32_t module_id) {
    if (!mgr || !category_name || !mgr->initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_ModuleCategory *cat = NULL;
    for (uint32_t i = 0; i < mgr->category_count; i++) {
        if (strcmp(mgr->categories[i].name, category_name) == 0) {
            cat = &mgr->categories[i];
            break;
        }
    }

    if (!cat) {
        IDCU_LOG_ERROR("category not found: %s", category_name);
        return IDCU_ERR_NOT_FOUND;
    }

    if (cat->module_count >= IDCU_MAX_MODULES_PER_CATEGORY) {
        return IDCU_ERR_QUEUE_FULL;
    }

    cat->module_ids[cat->module_count++] = module_id;
    return IDCU_ERR_SUCCESS;
}

int idcu_module_category_load_config(idcu_ModuleCategoryManager *mgr, const char *config_file) {
    if (!mgr || !config_file || !mgr->initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_config_init(config_file);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("failed to load config file: %s", config_file);
    }

    const char *known_modules[] = {
        "base_log",           "core_module",    "biz_collect",  "heartbeat_module",
        "healthcheck_module", "metrics_module", "alert_module", NULL};

    for (int i = 0;
         known_modules[i] != NULL && mgr->module_config_count < IDCU_MAX_REGISTERED_MODULES; i++) {
        const char *module_name = known_modules[i];

        idcu_ModuleConfig *mod_cfg = &mgr->module_configs[mgr->module_config_count];
        strncpy(mod_cfg->module_name, module_name, IDCU_MODULE_NAME_MAX - 1);
        strncpy(mod_cfg->category_name, "business", IDCU_CATEGORY_NAME_MAX - 1);

        if (strcmp(module_name, "base_log") == 0 || strcmp(module_name, "core_module") == 0) {
            strncpy(mod_cfg->category_name, "core", IDCU_CATEGORY_NAME_MAX - 1);
        }

        char enable_key[256];
        snprintf(enable_key, sizeof(enable_key), "enable_%s", module_name);
        mod_cfg->enabled = idcu_config_get_bool("modules", enable_key, 1);

        char prio_key[256];
        snprintf(prio_key, sizeof(prio_key), "%s.priority", module_name);
        const char *prio_str = idcu_config_get_string("modules", prio_key, "normal");

        if (strcmp(prio_str, "low") == 0) {
            mod_cfg->priority = IDCU_MOD_PRIO_LOW;
        } else if (strcmp(prio_str, "high") == 0) {
            mod_cfg->priority = IDCU_MOD_PRIO_HIGH;
        } else if (strcmp(prio_str, "realtime") == 0) {
            mod_cfg->priority = IDCU_MOD_PRIO_REALTIME;
        } else {
            mod_cfg->priority = IDCU_MOD_PRIO_NORMAL;
        }

        snprintf(mod_cfg->config_prefix, IDCU_CONFIG_SECTION_MAX, "module.%s", module_name);

        mgr->module_config_count++;
        IDCU_LOG_INFO("loaded module config: %s (category: %s, enabled: %d)", module_name,
                      mod_cfg->category_name, mod_cfg->enabled);
    }

    IDCU_LOG_INFO("loaded %u module configs from %s", mgr->module_config_count, config_file);
    return IDCU_ERR_SUCCESS;
}

int idcu_module_category_is_module_enabled(idcu_ModuleCategoryManager *mgr,
                                           const char *module_name) {
    if (!mgr || !module_name || !mgr->initialized) {
        return 0;
    }

    for (uint32_t i = 0; i < mgr->module_config_count; i++) {
        if (strcmp(mgr->module_configs[i].module_name, module_name) == 0) {
            return mgr->module_configs[i].enabled;
        }
    }

    return 1;
}

idcu_ModulePrio idcu_module_category_get_module_priority(idcu_ModuleCategoryManager *mgr,
                                                         const char *module_name) {
    if (!mgr || !module_name || !mgr->initialized) {
        return IDCU_MOD_PRIO_NORMAL;
    }

    for (uint32_t i = 0; i < mgr->module_config_count; i++) {
        if (strcmp(mgr->module_configs[i].module_name, module_name) == 0) {
            return mgr->module_configs[i].priority;
        }
    }

    return IDCU_MOD_PRIO_NORMAL;
}

const char *idcu_module_category_get_config_prefix(idcu_ModuleCategoryManager *mgr,
                                                   const char *module_name) {
    if (!mgr || !module_name || !mgr->initialized) {
        return "";
    }

    for (uint32_t i = 0; i < mgr->module_config_count; i++) {
        if (strcmp(mgr->module_configs[i].module_name, module_name) == 0) {
            return mgr->module_configs[i].config_prefix;
        }
    }

    return "";
}

int idcu_module_category_get_modules_by_level(idcu_ModuleCategoryManager *mgr,
                                              idcu_CategoryLevel level, uint32_t *module_ids,
                                              uint32_t *count) {
    if (!mgr || !module_ids || !count || !mgr->initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }

    uint32_t total = 0;
    for (uint32_t i = 0; i < mgr->category_count; i++) {
        if (mgr->categories[i].level == level) {
            for (uint32_t j = 0; j < mgr->categories[i].module_count; j++) {
                if (total < *count) {
                    module_ids[total++] = mgr->categories[i].module_ids[j];
                }
            }
        }
    }

    *count = total;
    return IDCU_ERR_SUCCESS;
}

const idcu_ModuleCategory *idcu_module_category_find(idcu_ModuleCategoryManager *mgr,
                                                     const char *name) {
    if (!mgr || !name || !mgr->initialized) {
        return NULL;
    }

    for (uint32_t i = 0; i < mgr->category_count; i++) {
        if (strcmp(mgr->categories[i].name, name) == 0) {
            return &mgr->categories[i];
        }
    }

    return NULL;
}
