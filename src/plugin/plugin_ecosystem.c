#include "plugin/plugin_ecosystem.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int plugin_version_parse(const char* version_str, uint32_t* major, uint32_t* minor, uint32_t* patch) {
    if (!version_str || !major || !minor || !patch) return ERR_INVALID_PARAM;
    
    char* endptr;
    unsigned long maj = strtoul(version_str, &endptr, 10);
    if (endptr == version_str || *endptr != '.') return ERR_INVALID_PARAM;
    
    unsigned long min = strtoul(endptr + 1, &endptr, 10);
    if (endptr == version_str + 1 || *endptr != '.') return ERR_INVALID_PARAM;
    
    unsigned long pat = strtoul(endptr + 1, &endptr, 10);
    if (endptr == version_str + 2) return ERR_INVALID_PARAM;
    
    *major = (uint32_t)maj;
    *minor = (uint32_t)min;
    *patch = (uint32_t)pat;
    
    return ERR_OK;
}

int plugin_version_compare(uint32_t maj1, uint32_t min1, uint32_t pat1, uint32_t maj2, uint32_t min2, uint32_t pat2) {
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

int plugin_ecosystem_init(PluginEcosystem* eco) {
    if (!eco) return ERR_INVALID_PARAM;
    memset(eco, 0, sizeof(PluginEcosystem));
    eco->plugin_count = 0;
    return ERR_OK;
}

void plugin_ecosystem_destroy(PluginEcosystem* eco) {
    if (!eco) return;
    memset(eco, 0, sizeof(PluginEcosystem));
}

int plugin_ecosystem_add_plugin(PluginEcosystem* eco, const char* name, const char* version, const char* description) {
    if (!eco || !name || !version) return ERR_INVALID_PARAM;
    if (eco->plugin_count >= MAX_PLUGINS) return ERR_NO_MEMORY;
    if (plugin_ecosystem_find_plugin(eco, name)) return ERR_ALREADY_EXISTS;
    
    strncpy(eco->plugins[eco->plugin_count].name, name, MAX_PLUGIN_NAME - 1);
    eco->plugins[eco->plugin_count].name[MAX_PLUGIN_NAME - 1] = '\0';
    
    strncpy(eco->plugins[eco->plugin_count].version, version, MAX_VERSION_STRING - 1);
    eco->plugins[eco->plugin_count].version[MAX_VERSION_STRING - 1] = '\0';
    
    if (description) {
        strncpy(eco->plugins[eco->plugin_count].description, description, MAX_PLUGIN_DESC - 1);
        eco->plugins[eco->plugin_count].description[MAX_PLUGIN_DESC - 1] = '\0';
    } else {
        eco->plugins[eco->plugin_count].description[0] = '\0';
    }
    
    plugin_version_parse(version, &eco->plugins[eco->plugin_count].major, 
                         &eco->plugins[eco->plugin_count].minor, 
                         &eco->plugins[eco->plugin_count].patch);
    
    eco->plugins[eco->plugin_count].status = PLUGIN_STATUS_AVAILABLE;
    eco->plugins[eco->plugin_count].rating = 0.0f;
    eco->plugins[eco->plugin_count].rating_count = 0;
    eco->plugins[eco->plugin_count].dep_count = 0;
    eco->plugin_count++;
    
    return ERR_OK;
}

int plugin_ecosystem_remove_plugin(PluginEcosystem* eco, const char* name) {
    if (!eco || !name) return ERR_INVALID_PARAM;
    
    for (int i = 0; i < eco->plugin_count; i++) {
        if (strcmp(eco->plugins[i].name, name) == 0) {
            for (int j = i; j < eco->plugin_count - 1; j++) {
                eco->plugins[j] = eco->plugins[j + 1];
            }
            eco->plugin_count--;
            return ERR_OK;
        }
    }
    
    return ERR_NOT_FOUND;
}

PluginInfo* plugin_ecosystem_find_plugin(PluginEcosystem* eco, const char* name) {
    if (!eco || !name) return NULL;
    
    for (int i = 0; i < eco->plugin_count; i++) {
        if (strcmp(eco->plugins[i].name, name) == 0) {
            return &eco->plugins[i];
        }
    }
    
    return NULL;
}

int plugin_ecosystem_install_plugin(PluginEcosystem* eco, const char* name) {
    if (!eco || !name) return ERR_INVALID_PARAM;
    
    PluginInfo* info = plugin_ecosystem_find_plugin(eco, name);
    if (!info) return ERR_NOT_FOUND;
    if (info->status == PLUGIN_STATUS_INSTALLED || info->status == PLUGIN_STATUS_ACTIVE) {
        return ERR_ALREADY_EXISTS;
    }
    
    info->status = PLUGIN_STATUS_INSTALLED;
    return ERR_OK;
}

int plugin_ecosystem_uninstall_plugin(PluginEcosystem* eco, const char* name) {
    if (!eco || !name) return ERR_INVALID_PARAM;
    
    PluginInfo* info = plugin_ecosystem_find_plugin(eco, name);
    if (!info) return ERR_NOT_FOUND;
    if (info->status == PLUGIN_STATUS_AVAILABLE) {
        return ERR_OK;
    }
    
    info->status = PLUGIN_STATUS_AVAILABLE;
    return ERR_OK;
}

int plugin_ecosystem_activate_plugin(PluginEcosystem* eco, const char* name) {
    if (!eco || !name) return ERR_INVALID_PARAM;
    
    PluginInfo* info = plugin_ecosystem_find_plugin(eco, name);
    if (!info) return ERR_NOT_FOUND;
    if (info->status != PLUGIN_STATUS_INSTALLED) {
        return ERR_INVALID_PARAM;
    }
    
    info->status = PLUGIN_STATUS_ACTIVE;
    return ERR_OK;
}

int plugin_ecosystem_deactivate_plugin(PluginEcosystem* eco, const char* name) {
    if (!eco || !name) return ERR_INVALID_PARAM;
    
    PluginInfo* info = plugin_ecosystem_find_plugin(eco, name);
    if (!info) return ERR_NOT_FOUND;
    if (info->status != PLUGIN_STATUS_ACTIVE) {
        return ERR_INVALID_PARAM;
    }
    
    info->status = PLUGIN_STATUS_INSTALLED;
    return ERR_OK;
}

int plugin_ecosystem_rate_plugin(PluginEcosystem* eco, const char* name, float rating) {
    if (!eco || !name) return ERR_INVALID_PARAM;
    if (rating < 0.0f || rating > 5.0f) return ERR_INVALID_PARAM;
    
    PluginInfo* info = plugin_ecosystem_find_plugin(eco, name);
    if (!info) return ERR_NOT_FOUND;
    
    float total = info->rating * info->rating_count;
    total += rating;
    info->rating_count++;
    info->rating = total / info->rating_count;
    
    return ERR_OK;
}

int plugin_ecosystem_set_dependencies(PluginEcosystem* eco, const char* name, const PluginDependency* deps, int count) {
    if (!eco || !name || !deps || count < 0 || count > MAX_DEPENDENCIES) return ERR_INVALID_PARAM;
    
    PluginInfo* info = plugin_ecosystem_find_plugin(eco, name);
    if (!info) return ERR_NOT_FOUND;
    
    memcpy(info->dependencies, deps, count * sizeof(PluginDependency));
    info->dep_count = count;
    
    return ERR_OK;
}

int plugin_ecosystem_check_compatibility(PluginEcosystem* eco, const char* name, const char* required_version) {
    if (!eco || !name || !required_version) return ERR_INVALID_PARAM;
    
    PluginInfo* info = plugin_ecosystem_find_plugin(eco, name);
    if (!info) return ERR_NOT_FOUND;
    
    uint32_t req_major, req_minor, req_patch;
    if (plugin_version_parse(required_version, &req_major, &req_minor, &req_patch) != ERR_OK) {
        return ERR_INVALID_PARAM;
    }
    
    int cmp = plugin_version_compare(info->major, info->minor, info->patch, req_major, req_minor, req_patch);
    if (cmp < 0) {
        return ERR_VERSION_MISMATCH;
    }
    
    return ERR_OK;
}
