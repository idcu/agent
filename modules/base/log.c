#include "module/module_def.h"
#include <stdio.h>

static int g_log_counter = 0;

static int base_log_init()  { printf("[base_log] init\n"); return 0; }
static int base_log_run()   { 
    if (g_log_counter % 1000000 == 0) {
        printf("[base_log] run (counter: %d)\n", g_log_counter);
    }
    g_log_counter++;
    return 0; 
}
static int base_log_stop()  { printf("[base_log] stop\n"); return 0; }

IDCU_REGISTER_MODULE(base_log, IDCU_MODULE_VERSION(1, 0, 0), base_log_init, base_log_run, base_log_stop);
