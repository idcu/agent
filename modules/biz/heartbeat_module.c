#include "module/module_def.h"
#include <stdio.h>

static int g_core_counter = 0;

static int heartbeat_module_init()  { printf("[heartbeat_module] init\n"); return 0; }
static int heartbeat_module_run()   { 
    if (g_core_counter % 2000000 == 0) {
        printf("[heartbeat_module] run (counter: %d)\n", g_core_counter);
    }
    g_core_counter++;
    return 0; 
}
static int heartbeat_module_stop()  { printf("[heartbeat_module] stop\n"); return 0; }

IDCU_REGISTER_MODULE(heartbeat_module, IDCU_MODULE_VERSION(1, 0, 0), heartbeat_module_init, heartbeat_module_run, heartbeat_module_stop);
