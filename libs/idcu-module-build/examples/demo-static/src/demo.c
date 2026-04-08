#include "demo/demo.h"

#define DEMO_VERSION "1.0.0"

int demo_subtract(int a, int b) { return a - b; }

int demo_divide(int a, int b) {
    if (b == 0) {
        return 0;
    }
    return a / b;
}

const char *demo_get_version(void) { return DEMO_VERSION; }
