#include "demo_idcu_agent/demo_idcu_agent.h"
#include "idcu/log/log.h"
#include "module_def.h"
#include <stdio.h>
#include <string.h>

static int g_module_enabled = 1;
static int g_run_count = 0;

int demo_idcu_agent_init(void) {
    IDCU_LOG_INFO("demo_idcu_agent initialized");
    g_run_count = 0;
    return IDCU_ERR_SUCCESS;
}

int demo_idcu_agent_run(void) {
    if (!g_module_enabled) {
        return IDCU_ERR_SUCCESS;
    }

    g_run_count++;
    if (g_run_count % 100 == 0) {
        IDCU_LOG_INFO("demo_idcu_agent running (count: %d)", g_run_count);
    }

    return IDCU_ERR_SUCCESS;
}

int demo_idcu_agent_stop(void) {
    IDCU_LOG_INFO("demo_idcu_agent stopped (total runs: %d)", g_run_count);
    return IDCU_ERR_SUCCESS;
}

IDCU_REGISTER_MODULE(demo_idcu_agent, IDCU_MODULE_VERSION(1, 0, 0), demo_idcu_agent_init,
                     demo_idcu_agent_run, demo_idcu_agent_stop);
