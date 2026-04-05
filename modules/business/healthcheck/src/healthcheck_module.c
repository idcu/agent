#include "module_def.h"
#include "config_manager.h"
#include "system_monitor.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>

static int g_check_interval_ms = 10000;
static int g_alert_threshold = 3;
static uint64_t g_last_check_ms = 0;
static int g_check_count = 0;
static idcu_SystemMonitor* g_system_monitor = NULL;

static int healthcheck_module_init()
{
    const char* config_prefix = "module.healthcheck_module";
    g_check_interval_ms = idcu_config_get_int(config_prefix, "check_interval_ms", 10000);
    g_alert_threshold = idcu_config_get_int(config_prefix, "alert_threshold", 3);
    
    int ret = idcu_system_monitor_init(&g_system_monitor);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_WARN("healthcheck_module: failed to initialize system monitor");
    }
    
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
        
        if (g_system_monitor) {
            idcu_CPUStats cpu_stats;
            idcu_MemoryStats mem_stats;
            idcu_DiskStats disk_stats;
            size_t disk_count = 1;
            
            idcu_system_monitor_get_cpu_stats(g_system_monitor, &cpu_stats);
            idcu_system_monitor_get_memory_stats(g_system_monitor, &mem_stats);
            idcu_system_monitor_get_disk_stats(g_system_monitor, &disk_stats, &disk_count);
            
            IDCU_LOG_INFO("healthcheck_module: CPU=%.1f%%, Memory=%.1f%%, Disk=%.1f%%",
                          cpu_stats.usage_percent,
                          mem_stats.usage_percent,
                          disk_count > 0 ? disk_stats.usage_percent : 0.0);
        }
        
        g_last_check_ms = now;
    }
    
    return IDCU_ERR_SUCCESS;
}

static int healthcheck_module_stop()
{
    if (g_system_monitor) {
        idcu_system_monitor_destroy(g_system_monitor);
        g_system_monitor = NULL;
    }
    
    IDCU_LOG_INFO("healthcheck_module stopped (total checks: %d)", g_check_count);
    return IDCU_ERR_SUCCESS;
}

IDCU_REGISTER_MODULE(healthcheck_module, IDCU_MODULE_VERSION(1, 0, 0), healthcheck_module_init, healthcheck_module_run, healthcheck_module_stop);
