#include "module/module_def.h"
#include "utils/config_manager.h"
#include "utils/log.h"
#include <stdio.h>

static int g_check_interval_ms = 10000;
static int g_alert_threshold = 3;
static uint64_t g_last_check_ms = 0;
static int g_check_count = 0;

static int healthcheck_module_init()
{
    const char* config_prefix = "module.healthcheck_module";
    g_check_interval_ms = idcu_config_get_int(config_prefix, "check_interval_ms", 10000);
    g_alert_threshold = idcu_config_get_int(config_prefix, "alert_threshold", 3);
    
    IDCU_LOG_INFO("healthcheck_module initialized (interval: %d ms, threshold: %d)", 
                  g_check_interval_ms, g_alert_threshold);
    return IDCU_ERR_SUCCESS;
}

static int healthcheck_module_run()
{
    extern uint64_t idcu_health_get_uptime_ms(void);
    uint64_t now = idcu_health_get_uptime_ms();
    
    if (now - g_last_check_ms >= (uint64_t)g_check_interval_ms) {
        g_check_count++;
        IDCU_LOG_INFO("healthcheck_module: performing health check #%d", g_check_count);
        g_last_check_ms = now;
    }
    
    return IDCU_ERR_SUCCESS;
}

static int healthcheck_module_stop()
{
    IDCU_LOG_INFO("healthcheck_module stopped (total checks: %d)", g_check_count);
    return IDCU_ERR_SUCCESS;
}

IDCU_REGISTER_MODULE(healthcheck_module, IDCU_MODULE_VERSION(1, 0, 0), healthcheck_module_init, healthcheck_module_run, healthcheck_module_stop);
