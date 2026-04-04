#include "module/module_def.h"
#include <stdio.h>

static int g_core_counter = 0;

static int config_module_init()  { printf("[config_module] init\n"); return 0; }
static int config_module_run()   { 
    if (g_core_counter % 2000000 == 0) {
        printf("[config_module] run (counter: %d)\n", g_core_counter);
    }
    g_core_counter++;
    return 0; 
}
static int config_module_stop()  { printf("[config_module] stop\n"); return 0; }

IDCU_REGISTER_MODULE(config_module, config_module_init, config_module_run, config_module_stop);
