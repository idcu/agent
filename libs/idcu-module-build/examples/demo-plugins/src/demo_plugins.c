#include "demo_plugins/demo_plugins.h"

#include "demo_plugins_version.h"

int demo_plugins_add(int a, int b) { return a + b; }

int demo_plugins_multiply(int a, int b) { return a * b; }

const char *demo_plugins_get_version(void) { return DEMO_PLUGINS_VERSION; }
