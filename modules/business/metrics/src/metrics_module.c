#include "module_def.h"
#include "config_manager.h"
#include "log.h"
#include <stdio.h>

static int g_collection_interval_ms = 5000;
static int g_export_enabled = 1;
static uint64_t g_last_collection_ms = 0;
static int g_collection_count = 0;

static int metrics_module_init()
{
    const char* config_prefix = "module.metrics_module";
    g_collection_interval_ms = idcu_config_get_int(config_prefix, "collection_interval_ms", 5000);
    g_export_enabled = idcu_config_get_bool(config_prefix, "export_enabled", 1);
    
    IDCU_LOG_INFO("metrics_module initialized (interval: %d ms, export: %s)", 
                  g_collection_interval_ms, g_export_enabled ? "enabled" : "disabled");
    return IDCU_ERR_SUCCESS;
}

static int metrics_module_run()
{
    extern uint64_t idcu_health_get_uptime_ms(void);
    uint64_t now = idcu_health_get_uptime_ms();
    
    if (now - g_last_collection_ms >= (uint64_t)g_collection_interval_ms) {
        g_collection_count++;
        IDCU_LOG_INFO("metrics_module: collecting metrics #%d", g_collection_count);
        g_last_collection_ms = now;
    }
    
    return IDCU_ERR_SUCCESS;
}

static int metrics_module_stop()
{
    IDCU_LOG_INFO("metrics_module stopped (total collections: %d)", g_collection_count);
    return IDCU_ERR_SUCCESS;
}

IDCU_REGISTER_MODULE(metrics_module, IDCU_MODULE_VERSION(1, 0, 0), metrics_module_init, metrics_module_run, metrics_module_stop);
