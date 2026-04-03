#include "module/module_def.h"
#include <stdio.h>

static int g_collect_counter = 0;

static int coll_init()  { printf("[biz_collect] init\n"); return 0; }
static int coll_run()   { 
    if (g_collect_counter % 5000000 == 0) {
        printf("[biz_collect] run (counter: %d)\n", g_collect_counter);
    }
    g_collect_counter++;
    return 0; 
}
static int coll_stop()  { printf("[biz_collect] stop\n"); return 0; }

IDCU_REGISTER_MODULE(biz_collect, coll_init, coll_run, coll_stop);
