#include <idcu/microkernel/kernel.h>
#include <idcu/log/log.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("=== IDCU Microkernel Example ===\n\n");
    
    idcu_MicroKernel* kernel;
    idcu_KernelConfig config;
    int ret;
    
    // Step 1: Configure the kernel
    printf("Step 1: Configuring microkernel...\n");
    config.config_path = "config/default/basic_libs.yaml";
    config.enable_logging = true;
    config.log_level = IDCU_LOG_INFO;
    config.max_modules = 100;
    printf("Kernel configuration:\n");
    printf("  - Config path: %s\n", config.config_path);
    printf("  - Logging: %s\n", config.enable_logging ? "enabled" : "disabled");
    printf("  - Log level: %d\n", config.log_level);
    printf("  - Max modules: %zu\n\n", config.max_modules);
    
    // Step 2: Initialize the kernel
    printf("Step 2: Initializing microkernel...\n");
    ret = idcu_kernel_init(&kernel, &config);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to initialize kernel: %s\n", idcu_error_message(ret));
        return 1;
    }
    printf("Microkernel initialized successfully.\n\n");
    
    // Step 3: Check kernel state
    printf("Step 3: Checking kernel state...\n");
    idcu_KernelState state = idcu_kernel_get_state(kernel);
    const char* state_str;
    switch (state) {
        case IDCU_KERNEL_STOPPED: state_str = "STOPPED"; break;
        case IDCU_KERNEL_STARTING: state_str = "STARTING"; break;
        case IDCU_KERNEL_RUNNING: state_str = "RUNNING"; break;
        case IDCU_KERNEL_STOPPING: state_str = "STOPPING"; break;
        default: state_str = "UNKNOWN"; break;
    }
    printf("Current kernel state: %s\n\n", state_str);
    
    // Step 4: Start the kernel
    printf("Step 4: Starting microkernel...\n");
    ret = idcu_kernel_start(kernel);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to start kernel: %s\n", idcu_error_message(ret));
        idcu_kernel_destroy(kernel);
        return 1;
    }
    printf("Microkernel started successfully.\n\n");
    
    // Check state after starting
    state = idcu_kernel_get_state(kernel);
    switch (state) {
        case IDCU_KERNEL_STOPPED: state_str = "STOPPED"; break;
        case IDCU_KERNEL_STARTING: state_str = "STARTING"; break;
        case IDCU_KERNEL_RUNNING: state_str = "RUNNING"; break;
        case IDCU_KERNEL_STOPPING: state_str = "STOPPING"; break;
        default: state_str = "UNKNOWN"; break;
    }
    printf("Kernel state after start: %s\n\n", state_str);
    
    // Step 5: Run the kernel (would block in real use)
    printf("Step 5: Kernel would run here (demo skips blocking run)...\n");
    printf("Note: In a real application, idcu_kernel_run() would block until stop is requested.\n\n");
    
    // Step 6: Stop the kernel
    printf("Step 6: Stopping microkernel...\n");
    ret = idcu_kernel_stop(kernel);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to stop kernel: %s\n", idcu_error_message(ret));
        idcu_kernel_destroy(kernel);
        return 1;
    }
    printf("Microkernel stopped successfully.\n\n");
    
    // Check state after stopping
    state = idcu_kernel_get_state(kernel);
    switch (state) {
        case IDCU_KERNEL_STOPPED: state_str = "STOPPED"; break;
        case IDCU_KERNEL_STARTING: state_str = "STARTING"; break;
        case IDCU_KERNEL_RUNNING: state_str = "RUNNING"; break;
        case IDCU_KERNEL_STOPPING: state_str = "STOPPING"; break;
        default: state_str = "UNKNOWN"; break;
    }
    printf("Kernel state after stop: %s\n\n", state_str);
    
    // Step 7: Cleanup
    printf("Step 7: Destroying microkernel...\n");
    idcu_kernel_destroy(kernel);
    printf("Microkernel destroyed successfully.\n\n");
    
    printf("=== Microkernel Example Complete ===\n");
    printf("\nSummary:\n");
    printf("  - The microkernel provides a minimal execution environment\n");
    printf("  - It manages module lifecycle (load, init, start, stop, destroy)\n");
    printf("  - It handles configuration and logging\n");
    printf("  - It provides graceful startup and shutdown\n");
    printf("  - Modules can communicate via message bus (not shown in this demo)\n");
    
    return 0;
}
