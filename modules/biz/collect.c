#include "module/module_def.h"
#include <stdio.h>

static int coll_init()  { printf("[biz_collect] init\n"); return 0; }
static int coll_run()   { return 0; }
static int coll_stop()  { return 0; }

IDCU_REGISTER_MODULE(biz_collect, coll_init, coll_run, coll_stop);
