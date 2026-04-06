#include "demo/demo.h"

#define DEMO_VERSION "1.0.0"

int demo_add(int a, int b) {
    return a + b;
}

int demo_multiply(int a, int b) {
    return a * b;
}

const char* demo_get_version(void) {
    return DEMO_VERSION;
}
