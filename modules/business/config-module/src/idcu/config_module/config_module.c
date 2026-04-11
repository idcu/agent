#include <idcu/config_module/config_module.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int idcu_config_module_config_init(idcu_ConfigModuleConfig* config) {
    if (!config) {
        return IDCU_ERR_INVALID_ARG;
    }

    memset(config, 0, sizeof(idcu_ConfigModuleConfig));
    strncpy(config->config_path, "config/app.yaml", sizeof(config->config_path) - 1);
    strncpy(config->backup_dir, "config/backups", sizeof(config->backup_dir) - 1);
    config->enable_hot_reload = 0;
    config->enable_auto_backup = 1;
    config->enable_encryption = 0;
    config->max_versions = 10;
    config->default_format = IDCU_CONFIG_FORMAT_YAML;

    return IDCU_ERR_OK;
}

int idcu_config_module_init(idcu_ConfigModule* cm, const idcu_ConfigModuleConfig* config) {
    if (!cm || !config) {
        return IDCU_ERR_INVALID_ARG;
    }

    memset(cm, 0, sizeof(idcu_ConfigModule));
    memcpy(&cm->config, config, sizeof(idcu_ConfigModuleConfig));
    cm->initialized = 0;

    return IDCU_ERR_OK;
}

int idcu_config_module_start(idcu_ConfigModule* cm) {
    if (!cm) {
        return IDCU_ERR_INVALID_ARG;
    }

    int ret = idcu_config_manager_create(&cm->config_manager);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    if (cm->config.config_path[0] != '\0') {
        ret = idcu_config_module_load_from_file(cm, cm->config.config_path);
    }

    cm->initialized = 1;

    return IDCU_ERR_OK;
}

int idcu_config_module_stop(idcu_ConfigModule* cm) {
    if (!cm) {
        return IDCU_ERR_INVALID_ARG;
    }

    if (cm->config_manager) {
        idcu_config_manager_destroy(cm->config_manager);
        cm->config_manager = NULL;
    }

    cm->initialized = 0;

    return IDCU_ERR_OK;
}

void idcu_config_module_destroy(idcu_ConfigModule* cm) {
    if (!cm) {
        return;
    }

    if (cm->initialized) {
        idcu_config_module_stop(cm);
    }

    memset(cm, 0, sizeof(idcu_ConfigModule));
}

int idcu_config_module_load(idcu_ConfigModule* cm) {
    if (!cm || !cm->config_manager) {
        return IDCU_ERR_INVALID_ARG;
    }

    return idcu_config_manager_load_from_file(cm->config_manager, cm->config.config_path);
}

int idcu_config_module_load_from_file(idcu_ConfigModule* cm, const char* file_path) {
    if (!cm || !cm->config_manager || !file_path) {
        return IDCU_ERR_INVALID_ARG;
    }

    return idcu_config_manager_load_from_file(cm->config_manager, file_path);
}

int idcu_config_module_save(idcu_ConfigModule* cm) {
    if (!cm || !cm->config_manager) {
        return IDCU_ERR_INVALID_ARG;
    }

    return idcu_config_manager_save_to_file(cm->config_manager, cm->config.config_path);
}

int idcu_config_module_save_to_file(idcu_ConfigModule* cm, const char* file_path) {
    if (!cm || !cm->config_manager || !file_path) {
        return IDCU_ERR_INVALID_ARG;
    }

    return idcu_config_manager_save_to_file(cm->config_manager, file_path);
}

int idcu_config_module_reload(idcu_ConfigModule* cm) {
    if (!cm) {
        return IDCU_ERR_INVALID_ARG;
    }

    return idcu_config_module_load(cm);
}

int idcu_config_module_backup(idcu_ConfigModule* cm, const char* message) {
    (void)cm;
    (void)message;
    return IDCU_ERR_OK;
}

int idcu_config_module_rollback(idcu_ConfigModule* cm, uint64_t version_id) {
    (void)cm;
    (void)version_id;
    return IDCU_ERR_OK;
}

int idcu_config_module_get_string(idcu_ConfigModule* cm, const char* key, const char** out_value) {
    if (!cm || !cm->config_manager || !key || !out_value) {
        return IDCU_ERR_INVALID_ARG;
    }

    return idcu_config_manager_get_string(cm->config_manager, key, out_value);
}

int idcu_config_module_set_string(idcu_ConfigModule* cm, const char* key, const char* value) {
    if (!cm || !cm->config_manager || !key || !value) {
        return IDCU_ERR_INVALID_ARG;
    }

    return idcu_config_manager_set_string(cm->config_manager, key, value);
}

int idcu_config_module_get_int(idcu_ConfigModule* cm, const char* key, int* out_value) {
    if (!cm || !cm->config_manager || !key || !out_value) {
        return IDCU_ERR_INVALID_ARG;
    }

    return idcu_config_manager_get_int(cm->config_manager, key, out_value);
}

int idcu_config_module_set_int(idcu_ConfigModule* cm, const char* key, int value) {
    if (!cm || !cm->config_manager || !key) {
        return IDCU_ERR_INVALID_ARG;
    }

    return idcu_config_manager_set_int(cm->config_manager, key, value);
}

int idcu_config_module_get_bool(idcu_ConfigModule* cm, const char* key, bool* out_value) {
    if (!cm || !cm->config_manager || !key || !out_value) {
        return IDCU_ERR_INVALID_ARG;
    }

    return idcu_config_manager_get_bool(cm->config_manager, key, out_value);
}

int idcu_config_module_set_bool(idcu_ConfigModule* cm, const char* key, bool value) {
    if (!cm || !cm->config_manager || !key) {
        return IDCU_ERR_INVALID_ARG;
    }

    return idcu_config_manager_set_bool(cm->config_manager, key, value);
}

idcu_ConfigManager* idcu_config_module_get_config_manager(idcu_ConfigModule* cm) {
    if (!cm) {
        return NULL;
    }
    return cm->config_manager;
}

int idcu_config_module_register_change_callback(idcu_ConfigModule* cm, idcu_ConfigChangeCallback callback, void* user_data) {
    (void)cm;
    (void)callback;
    (void)user_data;
    return IDCU_ERR_OK;
}

int idcu_config_module_unregister_change_callback(idcu_ConfigModule* cm, idcu_ConfigChangeCallback callback) {
    (void)cm;
    (void)callback;
    return IDCU_ERR_OK;
}
