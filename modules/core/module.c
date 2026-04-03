#include "module/module_def.h"
#include <stdio.h>

static int core_init()  { printf("[core_module] init\n"); return 0; }
static int core_run()   { return 0; }
static int core_stop()  { return 0; }

IDCU_REGISTER_MODULE(core_module, core_init, core_run, core_stop);
