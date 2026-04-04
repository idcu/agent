#include "module/module_def.h"
#include <stdio.h>

static int g_core_counter = 0;

static int healthcheck_module_init()  { printf("[healthcheck_module] init\n"); return 0; }
static int healthcheck_module_run()   { 
    if (g_core_counter % 2000000 == 0) {
        printf("[healthcheck_module] run (counter: %d)\n", g_core_counter);
    }
    g_core_counter++;
    return 0; 
}
static int healthcheck_module_stop()  { printf("[healthcheck_module] stop\n"); return 0; }

IDCU_REGISTER_MODULE(healthcheck_module, healthcheck_module_init, healthcheck_module_run, healthcheck_module_stop);
