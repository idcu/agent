/**
 * @file network_layer_example.c
 * @brief 网络层使用示�? * 
 * 演示如何使用网络层服务进行基本的网络通信
 */

#include "../modules/services/network/include/network_layer.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>
#define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

static volatile int g_running = 1;

void signal_handler(int sig) {
    (void)sig;
    g_running = 0;
    printf("\nShutting down...\n");
}

int main(void) {
    printf("====================================\n");
    printf("IDCU Network Layer Example\n");
    printf("====================================\n\n");

    signal(SIGINT, signal_handler);
    
    #ifdef _WIN32
    signal(SIGBREAK, signal_handler);
    #endif

    printf("Step 1: Initializing network layer...\n");
    int ret = idcu_network_init();
    if (ret != IDCU_ERR_OK) {
        printf("Failed to initialize network layer: %d\n", ret);
        return 1;
    }
    printf("Network layer initialized\n\n");

    printf("Network layer is available\n");
    printf("Note: This example demonstrates network layer initialization\n");
    printf("For full network functionality, please refer to the network service documentation\n\n");

    printf("Example is running! Press Ctrl+C to stop\n");

    while (g_running) {
        SLEEP_MS(100);
    }

    printf("\nCleaning up...\n");
    idcu_network_cleanup();
    
    printf("Example completed\n");
    return 0;
}
