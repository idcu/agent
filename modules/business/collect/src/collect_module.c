#include "module_def.h"
#include <stdio.h>

static int g_collect_counter = 0;

static int collect_module_init() {
    printf("[collect_module] init\n");
    return 0;
}
static int collect_module_run() {
    if (g_collect_counter % 5000000 == 0) {
        printf("[collect_module] run (counter: %d)\n", g_collect_counter);
    }
    g_collect_counter++;
    return 0;
}
static int collect_module_stop() {
    printf("[collect_module] stop\n");
    return 0;
}

IDCU_REGISTER_MODULE(collect_module, IDCU_MODULE_VERSION(1, 0, 0), collect_module_init,
                     collect_module_run, collect_module_stop);
