#include "plugin/plugin_ecosystem.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int idcu_plugin_version_parse(const char* version_str, uint32_t* major, uint32_t* minor, uint32_t* patch) {
    if (!version_str || !major || !minor || !patch) return IDCU_ERR_INVALID_PARAM;
    
    char* endptr;
    unsigned long maj = strtoul(version_str, &endptr, 10);
    if (endptr == version_str || *endptr != '.') return IDCU_ERR_INVALID_PARAM;
    
    unsigned long min = strtoul(endptr + 1, &endptr, 10);
    if (endptr == version_str + 1 || *endptr != '.') return IDCU_ERR_INVALID_PARAM;
    
    unsigned long pat = strtoul(endptr + 1, &endptr, 10);
    if (endptr == version_str + 2) return IDCU_ERR_INVALID_PARAM;
    
    *major = (uint32_t)maj;
    *minor = (uint32_t)min;
    *patch = (uint32_t)pat;
    
    return IDCU_ERR_OK;
}

int idcu_plugin_version_compare(uint32_t maj1, uint32_t min1, uint32_t pat1, uint32_t maj2, uint32_t min2, uint32_t pat2) {
    if (maj1 != maj2) {
        return (maj1 < maj2) ? -1 : 1;
    }
    if (min1 != min2) {
        return (min1 < min2) ? -1 : 1;
    }
    if (pat1 != pat2) {
        return (pat1 < pat2) ? -1 : 1;
    }
    return 0;
}

int idcu_plugin_ecosystem_init(idcu_PluginEcosystem* eco) {
    if (!eco) return IDCU_ERR_INVALID_PARAM;
    memset(eco, 0, sizeof(idcu_PluginEcosystem));
    eco->plugin_count = 0;
    return IDCU_ERR_OK;
}

void idcu_plugin_ecosystem_destroy(idcu_PluginEcosystem* eco) {
    if (!eco) return;
    memset(eco, 0, sizeof(idcu_PluginEcosystem));
}

int idcu_plugin_ecosystem_add_plugin(idcu_PluginEcosystem* eco, const char* name, const char* version, const char* description) {
    if (!eco || !name || !version) return IDCU_ERR_INVALID_PARAM;
    if (eco->plugin_count >= IDCU_MAX_PLUGINS) return IDCU_ERR_NO_MEMORY;
    if (idcu_plugin_ecosystem_find_plugin(eco, name)) return IDCU_ERR_ALREADY_EXISTS;
    
    strncpy(eco->plugins[eco->plugin_count].name, name, IDCU_MAX_PLUGIN_NAME - 1);
    eco->plugins[eco->plugin_count].name[IDCU_MAX_PLUGIN_NAME - 1] = '\0';
    
    strncpy(eco->plugins[eco->plugin_count].version, version, IDCU_MAX_VERSION_STRING - 1);
    eco->plugins[eco->plugin_count].version[IDCU_MAX_VERSION_STRING - 1] = '\0';
    
    if (description) {
        strncpy(eco->plugins[eco->plugin_count].description, description, IDCU_MAX_PLUGIN_DESC - 1);
        eco->plugins[eco->plugin_count].description[IDCU_MAX_PLUGIN_DESC - 1] = '\0';
    } else {
        eco->plugins[eco->plugin_count].description[0] = '\0';
    }
    
    idcu_plugin_version_parse(version, &eco->plugins[eco->plugin_count].major, 
                         &eco->plugins[eco->plugin_count].minor, 
                         &eco->plugins[eco->plugin_count].patch);
    
    eco->plugins[eco->plugin_count].status = IDCU_PLUGIN_STATUS_AVAILABLE;
    eco->plugins[eco->plugin_count].rating = 0.0f;
    eco->plugins[eco->plugin_count].rating_count = 0;
    eco->plugins[eco->plugin_count].dep_count = 0;
    eco->plugin_count++;
    
    return IDCU_ERR_OK;
}

int idcu_plugin_ecosystem_remove_plugin(idcu_PluginEcosystem* eco, const char* name) {
    if (!eco || !name) return IDCU_ERR_INVALID_PARAM;
    
    for (int i = 0; i < eco->plugin_count; i++) {
        if (strcmp(eco->plugins[i].name, name) == 0) {
            for (int j = i; j < eco->plugin_count - 1; j++) {
                eco->plugins[j] = eco->plugins[j + 1];
            }
            eco->plugin_count--;
            return IDCU_ERR_OK;
        }
    }
    
    return IDCU_ERR_NOT_FOUND;
}

idcu_PluginInfo* idcu_plugin_ecosystem_find_plugin(idcu_PluginEcosystem* eco, const char* name) {
    if (!eco || !name) return NULL;
    
    for (int i = 0; i < eco->plugin_count; i++) {
        if (strcmp(eco->plugins[i].name, name) == 0) {
            return &eco->plugins[i];
        }
    }
    
    return NULL;
}

int idcu_plugin_ecosystem_install_plugin(idcu_PluginEcosystem* eco, const char* name) {
    if (!eco || !name) return IDCU_ERR_INVALID_PARAM;
    
    idcu_PluginInfo* info = idcu_plugin_ecosystem_find_plugin(eco, name);
    if (!info) return IDCU_ERR_NOT_FOUND;
    if (info->status == IDCU_PLUGIN_STATUS_INSTALLED || info->status == IDCU_PLUGIN_STATUS_ACTIVE) {
        return IDCU_ERR_ALREADY_EXISTS;
    }
    
    info->status = IDCU_PLUGIN_STATUS_INSTALLED;
    return IDCU_ERR_OK;
}

int idcu_plugin_ecosystem_uninstall_plugin(idcu_PluginEcosystem* eco, const char* name) {
    if (!eco || !name) return IDCU_ERR_INVALID_PARAM;
    
    idcu_PluginInfo* info = idcu_plugin_ecosystem_find_plugin(eco, name);
    if (!info) return IDCU_ERR_NOT_FOUND;
    if (info->status == IDCU_PLUGIN_STATUS_AVAILABLE) {
        return IDCU_ERR_OK;
    }
    
    info->status = IDCU_PLUGIN_STATUS_AVAILABLE;
    return IDCU_ERR_OK;
}

int idcu_plugin_ecosystem_activate_plugin(idcu_PluginEcosystem* eco, const char* name) {
    if (!eco || !name) return IDCU_ERR_INVALID_PARAM;
    
    idcu_PluginInfo* info = idcu_plugin_ecosystem_find_plugin(eco, name);
    if (!info) return IDCU_ERR_NOT_FOUND;
    if (info->status != IDCU_PLUGIN_STATUS_INSTALLED) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    info->status = IDCU_PLUGIN_STATUS_ACTIVE;
    return IDCU_ERR_OK;
}

int idcu_plugin_ecosystem_deactivate_plugin(idcu_PluginEcosystem* eco, const char* name) {
    if (!eco || !name) return IDCU_ERR_INVALID_PARAM;
    
    idcu_PluginInfo* info = idcu_plugin_ecosystem_find_plugin(eco, name);
    if (!info) return IDCU_ERR_NOT_FOUND;
    if (info->status != IDCU_PLUGIN_STATUS_ACTIVE) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    info->status = IDCU_PLUGIN_STATUS_INSTALLED;
    return IDCU_ERR_OK;
}

int idcu_plugin_ecosystem_rate_plugin(idcu_PluginEcosystem* eco, const char* name, float rating) {
    if (!eco || !name) return IDCU_ERR_INVALID_PARAM;
    if (rating < 0.0f || rating > 5.0f) return IDCU_ERR_INVALID_PARAM;
    
    idcu_PluginInfo* info = idcu_plugin_ecosystem_find_plugin(eco, name);
    if (!info) return IDCU_ERR_NOT_FOUND;
    
    float total = info->rating * info->rating_count;
    total += rating;
    info->rating_count++;
    info->rating = total / info->rating_count;
    
    return IDCU_ERR_OK;
}

int idcu_plugin_ecosystem_set_dependencies(idcu_PluginEcosystem* eco, const char* name, const idcu_PluginDependency* deps, int count) {
    if (!eco || !name || !deps || count < 0 || count > IDCU_MAX_DEPENDENCIES) return IDCU_ERR_INVALID_PARAM;
    
    idcu_PluginInfo* info = idcu_plugin_ecosystem_find_plugin(eco, name);
    if (!info) return IDCU_ERR_NOT_FOUND;
    
    memcpy(info->dependencies, deps, count * sizeof(idcu_PluginDependency));
    info->dep_count = count;
    
    return IDCU_ERR_OK;
}

int idcu_plugin_ecosystem_check_compatibility(idcu_PluginEcosystem* eco, const char* name, const char* required_version) {
    if (!eco || !name || !required_version) return IDCU_ERR_INVALID_PARAM;
    
    idcu_PluginInfo* info = idcu_plugin_ecosystem_find_plugin(eco, name);
    if (!info) return IDCU_ERR_NOT_FOUND;
    
    uint32_t req_major, req_minor, req_patch;
    if (idcu_plugin_version_parse(required_version, &req_major, &req_minor, &req_patch) != IDCU_ERR_OK) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int cmp = idcu_plugin_version_compare(info->major, info->minor, info->patch, req_major, req_minor, req_patch);
    if (cmp < 0) {
        return IDCU_ERR_VERSION_MISMATCH;
    }
    
    return IDCU_ERR_OK;
}
