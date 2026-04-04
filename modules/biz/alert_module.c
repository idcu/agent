#include "module/module_def.h"
#include "monitor/alert_manager.h"
#include "monitor/metrics.h"
#include "monitor/notifier.h"
#include "utils/log.h"
#include "kernel/micro_kernel.h"
#include <stdio.h>

static idcu_AlertManager g_alert_manager;
static idcu_Notifier g_notifier;
static idcu_MetricsCollector* g_metrics_collector = NULL;
static idcu_MicroKernel* g_kernel = NULL;
static int g_initialized = 0;

static int alert_module_init(void)
{
    IDCU_LOG_INFO("Alert Module: Initializing...");
    
    extern idcu_MicroKernel* idcu_get_kernel(void);
    g_kernel = idcu_get_kernel();
    
    if (!g_kernel) {
        IDCU_LOG_ERROR("Alert Module: Failed to get kernel");
        return IDCU_ERR_NOT_INITIALIZED;
    }
    
    int ret = idcu_alert_manager_init(&g_alert_manager);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("Alert Module: Failed to initialize alert manager");
        return ret;
    }
    
    ret = idcu_notifier_init(&g_notifier);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("Alert Module: Failed to initialize notifier");
        idcu_alert_manager_destroy(&g_alert_manager);
        return ret;
    }
    
    ret = idcu_notifier_add_log_channel(&g_notifier);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("Alert Module: Failed to add log channel");
    }
    
    ret = idcu_notifier_add_file_channel(&g_notifier, "alerts.log");
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("Alert Module: Failed to add file channel");
    }
    
    ret = idcu_alert_manager_set_callback(&g_alert_manager, 
                                            idcu_notifier_alert_callback, 
                                            &g_notifier);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("Alert Module: Failed to set alert callback");
    }
    
    idcu_AlertRule cpu_rule;
    idcu_alert_rule_init(&cpu_rule, "high_cpu_usage", 
                           IDCU_ALERT_LEVEL_WARNING, 
                           "cpu_usage_percent", 
                           IDCU_ALERT_COND_GREATER, 80);
    idcu_alert_rule_set_message(&cpu_rule, "CPU usage is above 80%");
    idcu_alert_rule_set_pending_duration(&cpu_rule, 5000);
    idcu_alert_manager_add_rule(&g_alert_manager, &cpu_rule);
    
    idcu_AlertRule mem_rule;
    idcu_alert_rule_init(&mem_rule, "high_memory_usage", 
                           IDCU_ALERT_LEVEL_ERROR, 
                           "memory_usage_percent", 
                           IDCU_ALERT_COND_GREATER, 90);
    idcu_alert_rule_set_message(&mem_rule, "Memory usage is above 90%");
    idcu_alert_rule_set_pending_duration(&mem_rule, 3000);
    idcu_alert_manager_add_rule(&g_alert_manager, &mem_rule);
    
    g_initialized = 1;
    IDCU_LOG_INFO("Alert Module: Initialized successfully");
    return IDCU_ERR_SUCCESS;
}

static int alert_module_run(void)
{
    if (!g_initialized) {
        IDCU_LOG_ERROR("Alert Module: Not initialized");
        return IDCU_ERR_NOT_INITIALIZED;
    }
    
    extern idcu_MetricsCollector* idcu_get_metrics_collector(void);
    g_metrics_collector = idcu_get_metrics_collector();
    
    if (g_metrics_collector) {
        idcu_alert_manager_evaluate(&g_alert_manager, g_metrics_collector);
    }
    
    return IDCU_ERR_SUCCESS;
}

static int alert_module_stop(void)
{
    IDCU_LOG_INFO("Alert Module: Stopping...");
    
    if (g_initialized) {
        idcu_notifier_destroy(&g_notifier);
        idcu_alert_manager_destroy(&g_alert_manager);
        g_initialized = 0;
        g_metrics_collector = NULL;
        g_kernel = NULL;
    }
    
    IDCU_LOG_INFO("Alert Module: Stopped successfully");
    return IDCU_ERR_SUCCESS;
}

IDCU_REGISTER_MODULE(alert, IDCU_MODULE_VERSION(1, 0, 0), 
                    alert_module_init, alert_module_run, alert_module_stop);
