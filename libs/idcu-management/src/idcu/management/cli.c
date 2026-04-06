#include "idcu/management/cli.h"
#include "idcu/management/management.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void idcu_cli_print_help(void) {
    printf("IDCU Agent Management CLI\n");
    printf("Usage: idcu-cli [command] [options]\n\n");
    printf("Commands:\n");
    printf("  status              Show Agent status\n");
    printf("  modules             List all modules\n");
    printf("  module <name>       Show module details\n");
    printf("  load <name>         Load a module\n");
    printf("  unload <name>       Unload a module\n");
    printf("  start <name>        Start a module\n");
    printf("  stop <name>         Stop a module\n");
    printf("  coroutines          Show coroutine statistics\n");
    printf("  metrics             Show metrics data\n");
    printf("  config              Show configuration\n");
    printf("  healthcheck         Trigger health check\n");
    printf("  help                Show this help message\n");
}

static void print_agent_status(void) {
    idcu_AgentStatus status;
    int ret = idcu_management_get_agent_status(&status);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Failed to get agent status: %d\n", ret);
        return;
    }
    
    printf("Agent Status:\n");
    printf("  Version: %s\n", status.version);
    printf("  Uptime: %s\n", status.uptime_str);
    printf("  Running: %s\n", status.running ? "Yes" : "No");
}

static void print_modules(void) {
    idcu_ModuleDetail modules[64];
    size_t actual_count = 0;
    int ret = idcu_management_get_modules(modules, 64, &actual_count);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Failed to get modules: %d\n", ret);
        return;
    }
    
    printf("Modules (%zu):\n", actual_count);
    for (size_t i = 0; i < actual_count; i++) {
        printf("  - %s (v%s)\n", modules[i].info.name, modules[i].info.version);
        printf("    Author: %s\n", modules[i].info.author);
        printf("    Description: %s\n", modules[i].info.description);
        printf("    Priority: %d\n", modules[i].info.priority);
        printf("    Loaded: %s\n", modules[i].loaded ? "Yes" : "No");
        printf("    Running: %s\n", modules[i].running ? "Yes" : "No");
        printf("\n");
    }
}

static void print_module_detail(const char* name) {
    idcu_ModuleDetail detail;
    int ret = idcu_management_get_module_detail(name, &detail);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Failed to get module details: %d\n", ret);
        return;
    }
    
    printf("Module Details:\n");
    printf("  Name: %s\n", detail.info.name);
    printf("  Version: %s\n", detail.info.version);
    printf("  Author: %s\n", detail.info.author);
    printf("  Description: %s\n", detail.info.description);
    printf("  Priority: %d\n", detail.info.priority);
    printf("  Loaded: %s\n", detail.loaded ? "Yes" : "No");
    printf("  Running: %s\n", detail.running ? "Yes" : "No");
}

static void print_coroutines(void) {
    idcu_CoroutineStats stats;
    int ret = idcu_management_get_coroutine_stats(&stats);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Failed to get coroutine stats: %d\n", ret);
        return;
    }
    
    printf("Coroutine Statistics:\n");
    printf("  Total: %u\n", stats.total_count);
    printf("  Ready: %u\n", stats.ready_count);
    printf("  Running: %u\n", stats.running_count);
    printf("  Suspended: %u\n", stats.suspended_count);
    printf("  Finished: %u\n", stats.finished_count);
}

static void print_metrics(void) {
    char buffer[8192];
    int ret = idcu_management_get_metrics(buffer, sizeof(buffer));
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Failed to get metrics: %d\n", ret);
        return;
    }
    
    printf("Metrics:\n");
    printf("%s", buffer);
}

static void print_config(void) {
    char buffer[8192];
    int ret = idcu_management_get_config(buffer, sizeof(buffer));
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Failed to get config: %d\n", ret);
        return;
    }
    
    printf("%s", buffer);
}

static void trigger_health_check(void) {
    int ret = idcu_management_trigger_health_check();
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Failed to trigger health check: %d\n", ret);
        return;
    }
    printf("Health check triggered successfully\n");
}

static void load_module(const char* name) {
    int ret = idcu_management_load_module(name);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Failed to load module '%s': %d\n", name, ret);
        return;
    }
    printf("Module '%s' loaded successfully\n", name);
}

static void unload_module(const char* name) {
    int ret = idcu_management_unload_module(name);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Failed to unload module '%s': %d\n", name, ret);
        return;
    }
    printf("Module '%s' unloaded successfully\n", name);
}

static void start_module(const char* name) {
    int ret = idcu_management_start_module(name);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Failed to start module '%s': %d\n", name, ret);
        return;
    }
    printf("Module '%s' started successfully\n", name);
}

static void stop_module(const char* name) {
    int ret = idcu_management_stop_module(name);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Failed to stop module '%s': %d\n", name, ret);
        return;
    }
    printf("Module '%s' stopped successfully\n", name);
}

int idcu_cli_run(int argc, char* argv[]) {
    if (argc < 2) {
        idcu_cli_print_help();
        return IDCU_ERR_INVALID_PARAM;
    }
    
    const char* command = argv[1];
    
    if (strcmp(command, "help") == 0 || strcmp(command, "--help") == 0 || strcmp(command, "-h") == 0) {
        idcu_cli_print_help();
        return IDCU_ERR_SUCCESS;
    }
    
    if (strcmp(command, "status") == 0) {
        print_agent_status();
        return IDCU_ERR_SUCCESS;
    }
    
    if (strcmp(command, "modules") == 0) {
        print_modules();
        return IDCU_ERR_SUCCESS;
    }
    
    if (strcmp(command, "module") == 0) {
        if (argc < 3) {
            printf("Usage: idcu-cli module <name>\n");
            return IDCU_ERR_INVALID_PARAM;
        }
        print_module_detail(argv[2]);
        return IDCU_ERR_SUCCESS;
    }
    
    if (strcmp(command, "load") == 0) {
        if (argc < 3) {
            printf("Usage: idcu-cli load <name>\n");
            return IDCU_ERR_INVALID_PARAM;
        }
        load_module(argv[2]);
        return IDCU_ERR_SUCCESS;
    }
    
    if (strcmp(command, "unload") == 0) {
        if (argc < 3) {
            printf("Usage: idcu-cli unload <name>\n");
            return IDCU_ERR_INVALID_PARAM;
        }
        unload_module(argv[2]);
        return IDCU_ERR_SUCCESS;
    }
    
    if (strcmp(command, "start") == 0) {
        if (argc < 3) {
            printf("Usage: idcu-cli start <name>\n");
            return IDCU_ERR_INVALID_PARAM;
        }
        start_module(argv[2]);
        return IDCU_ERR_SUCCESS;
    }
    
    if (strcmp(command, "stop") == 0) {
        if (argc < 3) {
            printf("Usage: idcu-cli stop <name>\n");
            return IDCU_ERR_INVALID_PARAM;
        }
        stop_module(argv[2]);
        return IDCU_ERR_SUCCESS;
    }
    
    if (strcmp(command, "coroutines") == 0) {
        print_coroutines();
        return IDCU_ERR_SUCCESS;
    }
    
    if (strcmp(command, "metrics") == 0) {
        print_metrics();
        return IDCU_ERR_SUCCESS;
    }
    
    if (strcmp(command, "config") == 0) {
        print_config();
        return IDCU_ERR_SUCCESS;
    }
    
    if (strcmp(command, "healthcheck") == 0) {
        trigger_health_check();
        return IDCU_ERR_SUCCESS;
    }
    
    printf("Unknown command: %s\n", command);
    idcu_cli_print_help();
    return IDCU_ERR_INVALID_PARAM;
}
