#ifndef PLUGIN_ECOSYSTEM_H
#define PLUGIN_ECOSYSTEM_H

#include "common/error_code.h"
#include <stdint.h>
#include <stddef.h>

#define MAX_PLUGINS 64
#define MAX_PLUGIN_NAME 64
#define MAX_PLUGIN_DESC 256
#define MAX_DEPENDENCIES 16
#define MAX_VERSION_STRING 32

typedef enum {
    PLUGIN_STATUS_AVAILABLE = 0,
    PLUGIN_STATUS_INSTALLED,
    PLUGIN_STATUS_ACTIVE,
    PLUGIN_STATUS_ERROR
} PluginStatus;

typedef struct {
    char name[MAX_PLUGIN_NAME];
    char version[MAX_VERSION_STRING];
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
} PluginDependency;

typedef struct {
    char name[MAX_PLUGIN_NAME];
    char version[MAX_VERSION_STRING];
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    char description[MAX_PLUGIN_DESC];
    PluginStatus status;
    float rating;
    uint32_t rating_count;
    PluginDependency dependencies[MAX_DEPENDENCIES];
    int dep_count;
} PluginInfo;

typedef struct {
    PluginInfo plugins[MAX_PLUGINS];
    int plugin_count;
} PluginEcosystem;

int plugin_ecosystem_init(PluginEcosystem* eco);
void plugin_ecosystem_destroy(PluginEcosystem* eco);
int plugin_ecosystem_add_plugin(PluginEcosystem* eco, const char* name, const char* version, const char* description);
int plugin_ecosystem_remove_plugin(PluginEcosystem* eco, const char* name);
PluginInfo* plugin_ecosystem_find_plugin(PluginEcosystem* eco, const char* name);
int plugin_ecosystem_install_plugin(PluginEcosystem* eco, const char* name);
int plugin_ecosystem_uninstall_plugin(PluginEcosystem* eco, const char* name);
int plugin_ecosystem_activate_plugin(PluginEcosystem* eco, const char* name);
int plugin_ecosystem_deactivate_plugin(PluginEcosystem* eco, const char* name);
int plugin_ecosystem_rate_plugin(PluginEcosystem* eco, const char* name, float rating);
int plugin_ecosystem_set_dependencies(PluginEcosystem* eco, const char* name, const PluginDependency* deps, int count);
int plugin_ecosystem_check_compatibility(PluginEcosystem* eco, const char* name, const char* required_version);
int plugin_version_parse(const char* version_str, uint32_t* major, uint32_t* minor, uint32_t* patch);
int plugin_version_compare(uint32_t maj1, uint32_t min1, uint32_t pat1, uint32_t maj2, uint32_t min2, uint32_t pat2);

#endif
