#ifndef IDCU_PLUGIN_PLUGIN_ECOSYSTEM_H
#define IDCU_PLUGIN_PLUGIN_ECOSYSTEM_H

#include "common/error_code.h"
#include <stdint.h>
#include <stddef.h>

#define IDCU_MAX_PLUGINS 64
#define IDCU_MAX_PLUGIN_NAME 64
#define IDCU_MAX_PLUGIN_DESC 256
#define IDCU_MAX_DEPENDENCIES 16
#define IDCU_MAX_VERSION_STRING 32

typedef enum {
    IDCU_PLUGIN_STATUS_AVAILABLE = 0,
    IDCU_PLUGIN_STATUS_INSTALLED,
    IDCU_PLUGIN_STATUS_ACTIVE,
    IDCU_PLUGIN_STATUS_ERROR
} idcu_PluginStatus;

typedef struct {
    char name[IDCU_MAX_PLUGIN_NAME];
    char version[IDCU_MAX_VERSION_STRING];
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
} idcu_PluginDependency;

typedef struct {
    char name[IDCU_MAX_PLUGIN_NAME];
    char version[IDCU_MAX_VERSION_STRING];
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    char description[IDCU_MAX_PLUGIN_DESC];
    idcu_PluginStatus status;
    float rating;
    uint32_t rating_count;
    idcu_PluginDependency dependencies[IDCU_MAX_DEPENDENCIES];
    int dep_count;
} idcu_PluginInfo;

typedef struct {
    idcu_PluginInfo plugins[IDCU_MAX_PLUGINS];
    int plugin_count;
} idcu_PluginEcosystem;

int idcu_plugin_ecosystem_init(idcu_PluginEcosystem* eco);
void idcu_plugin_ecosystem_destroy(idcu_PluginEcosystem* eco);
int idcu_plugin_ecosystem_add_plugin(idcu_PluginEcosystem* eco, const char* name, const char* version, const char* description);
int idcu_plugin_ecosystem_remove_plugin(idcu_PluginEcosystem* eco, const char* name);
idcu_PluginInfo* idcu_plugin_ecosystem_find_plugin(idcu_PluginEcosystem* eco, const char* name);
int idcu_plugin_ecosystem_install_plugin(idcu_PluginEcosystem* eco, const char* name);
int idcu_plugin_ecosystem_uninstall_plugin(idcu_PluginEcosystem* eco, const char* name);
int idcu_plugin_ecosystem_activate_plugin(idcu_PluginEcosystem* eco, const char* name);
int idcu_plugin_ecosystem_deactivate_plugin(idcu_PluginEcosystem* eco, const char* name);
int idcu_plugin_ecosystem_rate_plugin(idcu_PluginEcosystem* eco, const char* name, float rating);
int idcu_plugin_ecosystem_set_dependencies(idcu_PluginEcosystem* eco, const char* name, const idcu_PluginDependency* deps, int count);
int idcu_plugin_ecosystem_check_compatibility(idcu_PluginEcosystem* eco, const char* name, const char* required_version);
int idcu_plugin_version_parse(const char* version_str, uint32_t* major, uint32_t* minor, uint32_t* patch);
int idcu_plugin_version_compare(uint32_t maj1, uint32_t min1, uint32_t pat1, uint32_t maj2, uint32_t min2, uint32_t pat2);

#endif // IDCU_PLUGIN_PLUGIN_ECOSYSTEM_H
