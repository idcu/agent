#include "idcu/config/config.h"
#include "idcu/log/log.h"
#include "module_def.h"
#include <stdio.h>

static int g_heartbeat_interval_ms = 5000;
static uint64_t g_last_heartbeat_ms = 0;
static int g_counter = 0;

static int heartbeat_module_init() {
    const char *config_prefix = "module.heartbeat_module";
    g_heartbeat_interval_ms = idcu_config_get_int(config_prefix, "interval_ms", 5000);

    IDCU_LOG_INFO("heartbeat_module initialized (interval: %d ms)", g_heartbeat_interval_ms);
    return IDCU_ERR_SUCCESS;
}

static int heartbeat_module_run() {
    extern uint64_t idcu_health_get_uptime_ms(void);
    uint64_t now = idcu_health_get_uptime_ms();

    if (now - g_last_heartbeat_ms >= (uint64_t)g_heartbeat_interval_ms) {
        g_counter++;
        IDCU_LOG_INFO("heartbeat_module: heartbeat #%d", g_counter);
        g_last_heartbeat_ms = now;
    }

    return IDCU_ERR_SUCCESS;
}

static int heartbeat_module_stop() {
    IDCU_LOG_INFO("heartbeat_module stopped (total heartbeats: %d)", g_counter);
    return IDCU_ERR_SUCCESS;
}

IDCU_REGISTER_MODULE(heartbeat_module, IDCU_MODULE_VERSION(1, 0, 0), heartbeat_module_init,
                     heartbeat_module_run, heartbeat_module_stop);
