#include "module/module_def.h"
#include <stdio.h>

static int g_core_counter = 0;

static int core_module_init()  { printf("[core_module] init\n"); return 0; }
static int core_module_run()   { 
    if (g_core_counter % 2000000 == 0) {
        printf("[core_module] run (counter: %d)\n", g_core_counter);
    }
    g_core_counter++;
    return 0; 
}
static int core_module_stop()  { printf("[core_module] stop\n"); return 0; }

IDCU_REGISTER_MODULE(core_module, core_module_init, core_module_run, core_module_stop);
