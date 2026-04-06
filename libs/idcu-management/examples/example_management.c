#include "idcu/management/management.h"
#include "idcu/management/http_api.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    // 初始化日志系统
    idcu_LogConfig log_config;
    idcu_log_get_default_config(&log_config);
    log_config.level = IDCU_LOG_INFO;
    log_config.output = IDCU_LOG_OUTPUT_CONSOLE | IDCU_LOG_OUTPUT_FILE;
    log_config.rotate_policy = IDCU_LOG_ROTATE_SIZE;
    log_config.max_file_size = 10 * 1024 * 1024; // 10MB
    log_config.max_backup_files = 5;
    strncpy(log_config.filename, "agent.log", sizeof(log_config.filename) - 1);
    
    if (idcu_log_init_with_config(&log_config) != IDCU_ERR_SUCCESS) {
        fprintf(stderr, "Failed to initialize log system\n");
        return 1;
    }
    
    // 初始化运维管理模块
    if (idcu_management_init() != IDCU_ERR_SUCCESS) {
        fprintf(stderr, "Failed to initialize management module\n");
        idcu_log_shutdown();
        return 1;
    }
    
    IDCU_LOG_INFO("Agent starting...");
    
    // 如果是命令行模式，运行 CLI
    if (argc > 1) {
        idcu_cli_run(argc, argv);
    } else {
        // 否则启动 HTTP 服务器
        idcu_HttpManagementServer http_server;
        
        if (idcu_http_management_init(&http_server, "0.0.0.0", 8080) != IDCU_ERR_SUCCESS) {
            fprintf(stderr, "Failed to initialize HTTP server\n");
            idcu_management_shutdown();
            idcu_log_shutdown();
            return 1;
        }
        
        if (idcu_http_management_start(&http_server) != IDCU_ERR_SUCCESS) {
            fprintf(stderr, "Failed to start HTTP server\n");
            idcu_http_management_destroy(&http_server);
            idcu_management_shutdown();
            idcu_log_shutdown();
            return 1;
        }
        
        printf("HTTP management server running on http://0.0.0.0:8080\n");
        printf("Available endpoints:\n");
        printf("  GET  /api/status    - Get agent status\n");
        printf("  GET  /api/modules   - List all modules\n");
        printf("  POST /api/modules/load   - Load a module\n");
        printf("  POST /api/modules/unload - Unload a module\n");
        printf("  POST /api/healthcheck    - Trigger health check\n");
        printf("  GET  /api/metrics  - Get metrics data\n");
        printf("  GET  /api/config   - Get configuration\n");
        printf("\nPress Ctrl+C to stop...\n");
        
        // 简单的轮询循环
        while (1) {
            idcu_http_management_poll(&http_server, 100);
        }
        
        idcu_http_management_stop(&http_server);
        idcu_http_management_destroy(&http_server);
    }
    
    // 清理
    idcu_management_shutdown();
    idcu_log_shutdown();
    
    return 0;
}
