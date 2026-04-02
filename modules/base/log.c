#include "include/module_def.h"
#include <stdio.h>

static int base_log_init()  { printf("[base_log] init\n"); return 0; }
static int base_log_run()   { return 0; }
static int base_log_stop()  { return 0; }

REGISTER_MODULE(base_log, base_log_init, base_log_run, base_log_stop);