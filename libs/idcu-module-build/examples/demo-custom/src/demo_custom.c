#include "demo_custom/demo_custom.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

static int g_initialized = 0;

int custom_init(void)
{
    if (g_initialized) {
        return 0;
    }
    g_initialized = 1;
    printf("[demo-custom] Initialized\n");
    return 0;
}

int custom_process(CustomData* data)
{
    if (!g_initialized || !data) {
        return -1;
    }
    
    time_t now = time(NULL);
    data->timestamp = (uint64_t)now;
    
    printf("[demo-custom] Processing: %s = %d (ts: %llu)\n", 
           data->name, data->value, (unsigned long long)data->timestamp);
    
    return 0;
}

int custom_cleanup(void)
{
    if (!g_initialized) {
        return 0;
    }
    g_initialized = 0;
    printf("[demo-custom] Cleaned up\n");
    return 0;
}
