#include "module/module_def.h"
#include <stdio.h>

static int core_module_init()  { printf("[core_module] init\n"); return 0; }
static int core_module_run()   { return 0; }
static int core_module_stop()  { return 0; }

IDCU_REGISTER_MODULE(core_module, core_module_init, core_module_run, core_module_stop);
