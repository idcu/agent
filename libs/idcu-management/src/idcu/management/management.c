#include "idcu/management/management.h"
#include "idcu/log/log.h"
#include "idcu/common/string_buf.h"
#include <string.h>
#include <time.h>

static time_t start_time = 0;
static int initialized = 0;

int idcu_management_init(void) {
    if (initialized) {
        return IDCU_ERR_SUCCESS;
    }
    start_time = time(NULL);
    initialized = 1;
    IDCU_LOG_INFO("Management module initialized");
    return IDCU_ERR_SUCCESS;
}

void idcu_management_shutdown(void) {
    if (!initialized) {
        return;
    }
    initialized = 0;
    IDCU_LOG_INFO("Management module shutdown");
}

int idcu_management_get_agent_status(idcu_AgentStatus* status) {
    if (!status || !initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    time_t now = time(NULL);
    uint64_t uptime = (uint64_t)difftime(now, start_time);
    
    strncpy(status->version, "1.0.0", sizeof(status->version) - 1);
    status->uptime_seconds = uptime;
    
    uint64_t days = uptime / 86400;
    uint64_t hours = (uptime % 86400) / 3600;
    uint64_t minutes = (uptime % 3600) / 60;
    uint64_t seconds = uptime % 60;
    
    snprintf(status->uptime_str, sizeof(status->uptime_str), 
             "%llud %lluh %llum %llus", 
             (unsigned long long)days, (unsigned long long)hours, 
             (unsigned long long)minutes, (unsigned long long)seconds);
    
    status->running = 1;
    
    return IDCU_ERR_SUCCESS;
}

int idcu_management_get_modules(idcu_ModuleDetail* modules, size_t max_modules, size_t* actual_count) {
    if (!modules || !actual_count || !initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    char** names = (char**)malloc(max_modules * sizeof(char*));
    if (!names) {
        return IDCU_ERR_MEMORY;
    }
    
    for (size_t i = 0; i < max_modules; i++) {
        names[i] = (char*)malloc(IDCU_MODULE_NAME_MAX);
        if (!names[i]) {
            for (size_t j = 0; j < i; j++) {
                free(names[j]);
            }
            free(names);
            return IDCU_ERR_MEMORY;
        }
    }
    
    int count = idcu_module_get_all_names(names, max_modules, actual_count);
    if (count != IDCU_ERR_SUCCESS) {
        for (size_t i = 0; i < max_modules; i++) {
            free(names[i]);
        }
        free(names);
        return count;
    }
    
    for (size_t i = 0; i < *actual_count && i < max_modules; i++) {
        idcu_module_get_info(names[i], &modules[i].info);
        modules[i].loaded = idcu_module_is_loaded(names[i]);
        modules[i].running = idcu_module_is_running(names[i]);
    }
    
    for (size_t i = 0; i < max_modules; i++) {
        free(names[i]);
    }
    free(names);
    
    return IDCU_ERR_SUCCESS;
}

int idcu_management_get_module_detail(const char* name, idcu_ModuleDetail* detail) {
    if (!name || !detail || !initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_module_get_info(name, &detail->info);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    detail->loaded = idcu_module_is_loaded(name);
    detail->running = idcu_module_is_running(name);
    
    return IDCU_ERR_SUCCESS;
}

int idcu_management_load_module(const char* name) {
    if (!name || !initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    idcu_ModuleHandle* handle = NULL;
    return idcu_module_load(name, &handle);
}

int idcu_management_unload_module(const char* name) {
    if (!name || !initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    return idcu_module_unregister(name);
}

int idcu_management_start_module(const char* name) {
    if (!name || !initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    idcu_ModuleHandle* handle = NULL;
    int ret = idcu_module_load(name, &handle);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    return idcu_module_start(handle);
}

int idcu_management_stop_module(const char* name) {
    if (!name || !initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    idcu_ModuleHandle* handle = NULL;
    int ret = idcu_module_load(name, &handle);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    return idcu_module_stop(handle);
}

int idcu_management_get_coroutine_stats(idcu_CoroutineStats* stats) {
    if (!stats || !initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    memset(stats, 0, sizeof(idcu_CoroutineStats));
    stats->ready_count = idcu_coro_get_ready_count(NULL);
    // Note: This is a placeholder - would need access to actual scheduler
    // For now, we'll return basic info
    
    return IDCU_ERR_SUCCESS;
}

int idcu_management_get_metrics(char* buffer, size_t buffer_size) {
    if (!buffer || !initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    idcu_StringBuf buf;
    idcu_string_buf_init(&buf, buffer, buffer_size);
    
    idcu_MetricsCollector* collector = idcu_global_metrics_collector();
    if (collector) {
        idcu_metrics_export_text(collector, &buf);
    } else {
        idcu_string_buf_append(&buf, "No metrics available\n");
    }
    
    return IDCU_ERR_SUCCESS;
}

int idcu_management_get_config(char* buffer, size_t buffer_size) {
    if (!buffer || !initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    idcu_StringBuf buf;
    idcu_string_buf_init(&buf, buffer, buffer_size);
    
    // This is a placeholder - would need to iterate through config sections
    idcu_string_buf_append(&buf, "Configuration:\n");
    idcu_string_buf_append(&buf, "  (Config export not fully implemented yet)\n");
    
    return IDCU_ERR_SUCCESS;
}

int idcu_management_trigger_health_check(void) {
    if (!initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    IDCU_LOG_INFO("Health check triggered");
    // This is a placeholder - would integrate with actual health check module
    return IDCU_ERR_SUCCESS;
}
