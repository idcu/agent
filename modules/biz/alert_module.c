#include "module/module_def.h"
#include "utils/config_manager.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

#define MAX_CHANNELS 16
#define MAX_ALERT_LEVELS 4
#define MAX_RULES 16

typedef struct {
    char name[64];
    int enabled;
    char config[512];
} AlertChannel;

typedef struct {
    char name[64];
    int enabled;
    int threshold;
    int duration_ms;
    char channels[256];
} AlertRule;

static char g_notification_url[256] = "http://localhost:8080/alert";
static int g_retry_count = 3;
static int g_alert_count = 0;
static int g_module_enabled = 1;

static AlertChannel g_channels[MAX_CHANNELS];
static int g_channel_count = 0;
static AlertRule g_rules[MAX_RULES];
static int g_rule_count = 0;
static int g_alert_levels[MAX_ALERT_LEVELS] = {1, 1, 1, 1};

static const char* s_alert_level_names[] = {"info", "warning", "error", "critical"};

static int load_channel_config(const char* channel_name)
{
    if (g_channel_count >= MAX_CHANNELS) {
        return IDCU_ERR_QUEUE_FULL;
    }
    
    const char* config_prefix = "module.alert_module";
    char prefix[128];
    snprintf(prefix, sizeof(prefix), "channels.%s", channel_name);
    
    AlertChannel* channel = &g_channels[g_channel_count];
    strncpy(channel->name, channel_name, sizeof(channel->name) - 1);
    channel->enabled = idcu_config_get_nested_bool(config_prefix, prefix, "enabled", 0);
    
    if (channel->enabled) {
        IDCU_LOG_INFO("Alert channel '%s' enabled", channel_name);
    }
    
    g_channel_count++;
    return IDCU_ERR_SUCCESS;
}

static int load_rule_config(const char* rule_name)
{
    if (g_rule_count >= MAX_RULES) {
        return IDCU_ERR_QUEUE_FULL;
    }
    
    const char* config_prefix = "module.alert_module";
    char prefix[128];
    snprintf(prefix, sizeof(prefix), "rules.%s", rule_name);
    
    AlertRule* rule = &g_rules[g_rule_count];
    strncpy(rule->name, rule_name, sizeof(rule->name) - 1);
    rule->enabled = idcu_config_get_nested_bool(config_prefix, prefix, "enabled", 0);
    rule->threshold = idcu_config_get_nested_int(config_prefix, prefix, "threshold", 90);
    rule->duration_ms = idcu_config_get_nested_int(config_prefix, prefix, "duration_ms", 60000);
    
    const char* channels_str = idcu_config_get_nested_string(config_prefix, prefix, "channels", "");
    strncpy(rule->channels, channels_str, sizeof(rule->channels) - 1);
    
    if (rule->enabled) {
        IDCU_LOG_INFO("Alert rule '%s' enabled (threshold: %d, duration: %dms, channels: %s)", 
                     rule_name, rule->threshold, rule->duration_ms, rule->channels);
    }
    
    g_rule_count++;
    return IDCU_ERR_SUCCESS;
}

static int alert_module_init()
{
    const char* config_prefix = "module.alert_module";
    
    g_module_enabled = idcu_config_get_bool(config_prefix, "enabled", 1);
    if (!g_module_enabled) {
        IDCU_LOG_INFO("alert_module disabled by configuration");
        return IDCU_ERR_SUCCESS;
    }
    
    const char* url = idcu_config_get_string(config_prefix, "notification_url", "http://localhost:8080/alert");
    strncpy(g_notification_url, url, sizeof(g_notification_url) - 1);
    g_retry_count = idcu_config_get_int(config_prefix, "retry_count", 3);
    
    for (int i = 0; i < MAX_ALERT_LEVELS; i++) {
        char level_key[64];
        snprintf(level_key, sizeof(level_key), "alert_levels.%s", s_alert_level_names[i]);
        g_alert_levels[i] = idcu_config_get_bool(config_prefix, level_key, 1);
    }
    
    load_channel_config("email");
    load_channel_config("webhook");
    load_channel_config("sms");
    load_channel_config("dingtalk");
    
    load_rule_config("cpu_high");
    load_rule_config("memory_high");
    load_rule_config("disk_full");
    
    IDCU_LOG_INFO("alert_module initialized (url: %s, retries: %d, channels: %d, rules: %d)", 
                 g_notification_url, g_retry_count, g_channel_count, g_rule_count);
    return IDCU_ERR_SUCCESS;
}

static int alert_module_run()
{
    if (!g_module_enabled) {
        return IDCU_ERR_SUCCESS;
    }
    return IDCU_ERR_SUCCESS;
}

static int alert_module_stop()
{
    IDCU_LOG_INFO("alert_module stopped (total alerts: %d)", g_alert_count);
    return IDCU_ERR_SUCCESS;
}

IDCU_REGISTER_MODULE(alert_module, IDCU_MODULE_VERSION(2, 0, 0), alert_module_init, alert_module_run, alert_module_stop);
