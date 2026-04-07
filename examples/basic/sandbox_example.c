/**
 * @file sandbox_example.c
 * @brief 沙箱安全机制示例
 * 
 * 演示如何使用沙箱服务进行模块安全隔离
 */

#include "../modules/services/security/include/sandbox.h"
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

void print_sandbox_info(const idcu_Sandbox* sandbox) {
    if (!sandbox) return;
    
    printf("Sandbox Info:\n");
    printf("  ID: %u\n", sandbox->id);
    printf("  Name: %s\n", sandbox->name);
    printf("  Permissions: 0x%08x\n", sandbox->permissions);
    printf("  State: %d\n", sandbox->state);
    printf("  Memory Limit: %u bytes\n", sandbox->memory_limit);
    printf("  CPU Time Limit: %u ms\n", sandbox->cpu_time_limit);
    printf("\n");
}

int main(void) {
    printf("====================================\n");
    printf("IDCU Sandbox Service Example\n");
    printf("====================================\n\n");

    signal(SIGINT, signal_handler);
    
    #ifdef _WIN32
    signal(SIGBREAK, signal_handler);
    #endif

    printf("Step 1: Creating sandbox...\n");
    idcu_Sandbox sandbox;
    
    uint32_t permissions = IDCU_SANDBOX_PERM_SEND_MSG | 
                           IDCU_SANDBOX_PERM_RECV_MSG | 
                           IDCU_SANDBOX_PERM_RUN;
    
    int ret = idcu_sandbox_create(&sandbox, "example_sandbox", permissions);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to create sandbox: %d\n", ret);
        return 1;
    }
    printf("Sandbox created\n\n");

    print_sandbox_info(&sandbox);

    printf("Step 2: Setting resource limits...\n");
    idcu_sandbox_set_resource_limit(&sandbox, IDCU_SANDBOX_RES_MEMORY, 1024 * 1024);  // 1MB
    idcu_sandbox_set_resource_limit(&sandbox, IDCU_SANDBOX_RES_CPU_TIME, 1000);  // 1 second
    idcu_sandbox_set_resource_limit(&sandbox, IDCU_SANDBOX_RES_FDS, 10);  // 10 file descriptors
    printf("Resource limits set\n\n");

    printf("Step 3: Checking permissions...\n");
    if (idcu_sandbox_check_permission(&sandbox, IDCU_SANDBOX_PERM_SEND_MSG)) {
        printf("  SEND_MSG permission: ALLOWED\n");
    } else {
        printf("  SEND_MSG permission: DENIED\n");
    }
    
    if (idcu_sandbox_check_permission(&sandbox, IDCU_SANDBOX_PERM_HW_ACCESS)) {
        printf("  HW_ACCESS permission: ALLOWED\n");
    } else {
        printf("  HW_ACCESS permission: DENIED\n");
    }
    printf("\n");

    printf("Sandbox service is available\n");
    printf("Note: This example demonstrates sandbox creation and permission checking\n");
    printf("For full sandbox functionality, please refer to the security service documentation\n\n");

    printf("Example is running! Press Ctrl+C to stop\n");

    while (g_running) {
        SLEEP_MS(100);
    }

    printf("\nCleaning up...\n");
    idcu_sandbox_destroy(&sandbox);
    
    printf("Example completed\n");
    return 0;
}
