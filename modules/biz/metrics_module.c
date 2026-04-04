#include "module/module_def.h"
#include <stdio.h>

static int g_core_counter = 0;

static int metrics_module_init()  { printf("[metrics_module] init\n"); return 0; }
static int metrics_module_run()   { 
    if (g_core_counter % 2000000 == 0) {
        printf("[metrics_module] run (counter: %d)\n", g_core_counter);
    }
    g_core_counter++;
    return 0; 
}
static int metrics_module_stop()  { printf("[metrics_module] stop\n"); return 0; }

IDCU_REGISTER_MODULE(metrics_module, metrics_module_init, metrics_module_run, metrics_module_stop);
