#include "module_def.h"
#include <stdio.h>

static int g_core_counter = 0;

static int log_module_init()  { printf("[log_module] init\n"); return 0; }
static int log_module_run()   { 
    if (g_core_counter % 2000000 == 0) {
        printf("[log_module] run (counter: %d)\n", g_core_counter);
    }
    g_core_counter++;
    return 0; 
}
static int log_module_stop()  { printf("[log_module] stop\n"); return 0; }

IDCU_REGISTER_MODULE(log_module, IDCU_MODULE_VERSION(1, 0, 0), log_module_init, log_module_run, log_module_stop);
