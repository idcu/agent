#include "module_def.h"
#include "idcu/config/config.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <string.h>

static int g_initialized = 0;
static uint64_t g_last_modified_time = 0;
static int g_check_interval = 5000;
static int g_counter = 0;

static void config_change_callback(const char* section, const char* key, 
                                    const char* old_value, const char* new_value, void* user_data) {
    IDCU_LOG_INFO("[config_module] Config changed: [%s] %s: '%s' -> '%s'",
                  section, key,
                  old_value ? old_value : "(null)",
                  new_value ? new_value : "(null)");
}

static int config_module_init() {
    IDCU_LOG_INFO("[config_module] Initializing config module");
    
    const char* config_path = idcu_config_get_string("general", "config_path", "config/agent.cfg");
    int ret = idcu_config_init(config_path);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("[config_module] Failed to initialize config: %d", ret);
        return -1;
    }

    ret = idcu_config_register_change_callback(config_change_callback, NULL);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("[config_module] Failed to register change callback: %d", ret);
    }

    idcu_config_get_last_modified_time(&g_last_modified_time);
    g_check_interval = idcu_config_get_int("config", "check_interval_ms", 5000);
    
    g_initialized = 1;
    IDCU_LOG_INFO("[config_module] Config module initialized successfully");
    return 0;
}

static int config_module_run() {
    if (!g_initialized) return 0;

    g_counter++;
    if (g_counter % (g_check_interval / 10) == 0) {
        uint64_t current_time = 0;
        int ret = idcu_config_get_last_modified_time(&current_time);
        if (ret == IDCU_ERR_SUCCESS && current_time != g_last_modified_time) {
            IDCU_LOG_INFO("[config_module] Config file modified, reloading...");
            ret = idcu_config_reload();
            if (ret == IDCU_ERR_SUCCESS) {
                g_last_modified_time = current_time;
                IDCU_LOG_INFO("[config_module] Config reloaded successfully");
            } else {
                IDCU_LOG_ERROR("[config_module] Failed to reload config: %d", ret);
            }
        }
    }

    return 0;
}

static int config_module_stop() {
    if (g_initialized) {
        IDCU_LOG_INFO("[config_module] Stopping config module");
        idcu_config_unregister_change_callback(config_change_callback);
        idcu_config_shutdown();
        g_initialized = 0;
    }
    return 0;
}

int idcu_config_module_reload(void) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[config_module] Module not initialized");
        return -1;
    }
    return idcu_config_reload();
}

IDCU_REGISTER_MODULE(config_module, IDCU_MODULE_VERSION(1, 0, 0), 
                     config_module_init, config_module_run, config_module_stop);
