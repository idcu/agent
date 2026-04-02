#include "include/module_def.h"
#include <stdio.h>

static int log_init()  { printf("[base_log] init\n"); return 0; }
static int log_run()   { return 0; }
static int log_stop()  { return 0; }

REGISTER_MODULE(base_log, log_init, log_run, log_stop);