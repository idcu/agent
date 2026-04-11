#ifndef IDCU_CONFIG_MODULE_CONFIG_MODULE_H
#define IDCU_CONFIG_MODULE_CONFIG_MODULE_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/config/config.h>
#include <idcu/yaml/yaml.h>
#include <idcu/json/json.h>
#include <idcu/msgbus/msgbus.h>
#include <idcu/sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IDCU_CONFIG_FORMAT_YAML = 0,
    IDCU_CONFIG_FORMAT_JSON,
    IDCU_CONFIG_FORMAT_COUNT
} idcu_ConfigFormat;

typedef struct {
    char config_path[1024];
    char backup_dir[1024];
    int enable_hot_reload;
    int enable_auto_backup;
    int enable_encryption;
    uint64_t max_versions;
    idcu_ConfigFormat default_format;
} idcu_ConfigModuleConfig;

typedef struct {
    idcu_ConfigModuleConfig config;
    idcu_ConfigManager* config_manager;
    idcu_MsgBus* msgbus;
    int initialized;
    void* user_data;
} idcu_ConfigModule;

typedef void (*idcu_ConfigChangeCallback)(const char* key, const void* old_value, const void* new_value, void* user_data);

int idcu_config_module_config_init(idcu_ConfigModuleConfig* config);

int idcu_config_module_init(idcu_ConfigModule* cm, const idcu_ConfigModuleConfig* config);

int idcu_config_module_start(idcu_ConfigModule* cm);

int idcu_config_module_stop(idcu_ConfigModule* cm);

void idcu_config_module_destroy(idcu_ConfigModule* cm);

int idcu_config_module_load(idcu_ConfigModule* cm);

int idcu_config_module_load_from_file(idcu_ConfigModule* cm, const char* file_path);

int idcu_config_module_save(idcu_ConfigModule* cm);

int idcu_config_module_save_to_file(idcu_ConfigModule* cm, const char* file_path);

int idcu_config_module_reload(idcu_ConfigModule* cm);

int idcu_config_module_backup(idcu_ConfigModule* cm, const char* message);

int idcu_config_module_rollback(idcu_ConfigModule* cm, uint64_t version_id);

int idcu_config_module_get_string(idcu_ConfigModule* cm, const char* key, const char** out_value);

int idcu_config_module_set_string(idcu_ConfigModule* cm, const char* key, const char* value);

int idcu_config_module_get_int(idcu_ConfigModule* cm, const char* key, int* out_value);

int idcu_config_module_set_int(idcu_ConfigModule* cm, const char* key, int value);

int idcu_config_module_get_bool(idcu_ConfigModule* cm, const char* key, bool* out_value);

int idcu_config_module_set_bool(idcu_ConfigModule* cm, const char* key, bool value);

idcu_ConfigManager* idcu_config_module_get_config_manager(idcu_ConfigModule* cm);

int idcu_config_module_register_change_callback(idcu_ConfigModule* cm, idcu_ConfigChangeCallback callback, void* user_data);

int idcu_config_module_unregister_change_callback(idcu_ConfigModule* cm, idcu_ConfigChangeCallback callback);

#ifdef __cplusplus
}
#endif

#endif
