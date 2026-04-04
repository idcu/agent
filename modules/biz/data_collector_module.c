#include "module/module_def.h"
#include <stdio.h>

static int g_core_counter = 0;

static int data_collector_module_init()  { printf("[data_collector_module] init\n"); return 0; }
static int data_collector_module_run()   { 
    if (g_core_counter % 2000000 == 0) {
        printf("[data_collector_module] run (counter: %d)\n", g_core_counter);
    }
    g_core_counter++;
    return 0; 
}
static int data_collector_module_stop()  { printf("[data_collector_module] stop\n"); return 0; }

IDCU_REGISTER_MODULE(data_collector_module, data_collector_module_init, data_collector_module_run, data_collector_module_stop);
