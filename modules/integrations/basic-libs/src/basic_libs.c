#include "basic_libs.h"
#include "idcu/log/log.h"
#include "module_def.h"
#include <string.h>

static idcu_BasicLibs *g_basic_libs = NULL;

static int g_config_enabled = 1;
static int g_log_enabled = 1;
static int g_json_enabled = 1;
static int g_metrics_enabled = 1;
static int g_healthcheck_enabled = 1;
static int g_http_server_enabled = 1;
static int g_http_client_enabled = 1;
static int g_network_enabled = 1;

int idcu_basic_libs_init(idcu_BasicLibs *libs, const char *config_file, const char *log_file,
                         idcu_LogLevel log_level) {
    if (!libs) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memset(libs, 0, sizeof(idcu_BasicLibs));

    if (config_file) {
        strncpy(libs->config_file, config_file, sizeof(libs->config_file) - 1);
    }
    if (log_file) {
        strncpy(libs->log_file, log_file, sizeof(libs->log_file) - 1);
    }
    libs->log_level = log_level;

    int ret;

    if (g_log_enabled) {
        ret = idcu_log_init(log_file, log_level);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
        IDCU_LOG_INFO("Log integration initialized");
    }

    if (g_config_enabled) {
        ret = idcu_config_init(config_file);
        if (ret != IDCU_ERR_OK) {
            if (g_log_enabled) {
                idcu_log_shutdown();
            }
            return ret;
        }
        IDCU_LOG_INFO("Config integration initialized");
    }

    libs->initialized = 1;
    g_basic_libs = libs;
    IDCU_LOG_INFO("Basic libraries integration initialized");
    return IDCU_ERR_OK;
}

void idcu_basic_libs_destroy(idcu_BasicLibs *libs) {
    if (!libs || !libs->initialized) {
        return;
    }

    if (g_config_enabled) {
        idcu_config_shutdown();
        IDCU_LOG_INFO("Config integration destroyed");
    }

    if (g_log_enabled) {
        idcu_log_shutdown();
        IDCU_LOG_INFO("Log integration destroyed");
    }

    libs->initialized = 0;
    g_basic_libs = NULL;
    IDCU_LOG_INFO("Basic libraries integration destroyed");
}

int idcu_basic_libs_enable_config(int enable) {
    g_config_enabled = enable;
    return IDCU_ERR_OK;
}

int idcu_basic_libs_enable_log(int enable) {
    g_log_enabled = enable;
    return IDCU_ERR_OK;
}

int idcu_basic_libs_enable_json(int enable) {
    g_json_enabled = enable;
    return IDCU_ERR_OK;
}

int idcu_basic_libs_enable_metrics(int enable) {
    g_metrics_enabled = enable;
    return IDCU_ERR_OK;
}

int idcu_basic_libs_enable_healthcheck(int enable) {
    g_healthcheck_enabled = enable;
    return IDCU_ERR_OK;
}

int idcu_basic_libs_enable_http_server(int enable) {
    g_http_server_enabled = enable;
    return IDCU_ERR_OK;
}

int idcu_basic_libs_enable_http_client(int enable) {
    g_http_client_enabled = enable;
    return IDCU_ERR_OK;
}

int idcu_basic_libs_enable_network(int enable) {
    g_network_enabled = enable;
    return IDCU_ERR_OK;
}

int idcu_basic_libs_config_reload(void) {
    if (!g_basic_libs || !g_basic_libs->initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    if (!g_config_enabled) {
        return IDCU_ERR_NOT_ENABLED;
    }
    return idcu_config_reload();
}

int idcu_basic_libs_config_save(const char *file_path) {
    if (!g_basic_libs || !g_basic_libs->initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    if (!g_config_enabled) {
        return IDCU_ERR_NOT_ENABLED;
    }
    return idcu_config_save(file_path);
}

int idcu_basic_libs_log_set_level(idcu_LogLevel level) {
    if (!g_basic_libs || !g_basic_libs->initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    if (!g_log_enabled) {
        return IDCU_ERR_NOT_ENABLED;
    }
    idcu_log_set_level(level);
    g_basic_libs->log_level = level;
    return IDCU_ERR_OK;
}

idcu_LogLevel idcu_basic_libs_log_get_level(void) {
    if (!g_basic_libs || !g_basic_libs->initialized) {
        return IDCU_LOG_INFO;
    }
    return g_basic_libs->log_level;
}

static idcu_BasicLibs s_basic_libs;

static int basic_libs_module_init(void) {
    return idcu_basic_libs_init(&s_basic_libs, "config/agent.cfg", NULL, IDCU_LOG_INFO);
}

static int basic_libs_module_run(void) { return 0; }

static int basic_libs_module_stop(void) {
    idcu_basic_libs_destroy(&s_basic_libs);
    return 0;
}

IDCU_REGISTER_MODULE(basic_libs, IDCU_MODULE_VERSION(1, 0, 0), basic_libs_module_init,
                     basic_libs_module_run, basic_libs_module_stop);
