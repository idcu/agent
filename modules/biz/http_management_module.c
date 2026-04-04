#include "module/module_def.h"
#include "network/http_server.h"
#include "network/management_api.h"
#include "kernel/micro_kernel.h"
#include "utils/log.h"
#include <stdio.h>

static idcu_HttpServer g_http_server;
static int g_initialized = 0;
static idcu_MicroKernel* g_kernel = NULL;

static int http_management_module_init(void)
{
    IDCU_LOG_INFO("HTTP Management Module: Initializing...");
    
    extern idcu_MicroKernel* idcu_get_kernel(void);
    g_kernel = idcu_get_kernel();
    
    if (!g_kernel) {
        IDCU_LOG_ERROR("HTTP Management Module: Failed to get kernel");
        return IDCU_ERR_NOT_INITIALIZED;
    }
    
    const char* bind_addr = "0.0.0.0";
    uint16_t bind_port = 8080;
    
    int ret = idcu_http_server_init(&g_http_server, g_kernel, bind_addr, bind_port);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("HTTP Management Module: Failed to initialize HTTP server");
        return ret;
    }
    
    ret = idcu_management_api_register_routes(&g_http_server);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("HTTP Management Module: Failed to register API routes");
        idcu_http_server_destroy(&g_http_server);
        return ret;
    }
    
    ret = idcu_http_server_start(&g_http_server);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("HTTP Management Module: Failed to start HTTP server");
        idcu_http_server_destroy(&g_http_server);
        return ret;
    }
    
    g_initialized = 1;
    IDCU_LOG_INFO("HTTP Management Module: Initialized and started on %s:%d", bind_addr, bind_port);
    return IDCU_ERR_OK;
}

static int http_management_module_run(void)
{
    if (!g_initialized) {
        IDCU_LOG_ERROR("HTTP Management Module: Not initialized");
        return IDCU_ERR_NOT_INITIALIZED;
    }
    
    idcu_http_server_poll(&g_http_server, 10);
    return IDCU_ERR_OK;
}

static int http_management_module_stop(void)
{
    IDCU_LOG_INFO("HTTP Management Module: Stopping...");
    
    if (g_initialized) {
        idcu_http_server_destroy(&g_http_server);
        g_initialized = 0;
        g_kernel = NULL;
    }
    
    IDCU_LOG_INFO("HTTP Management Module: Stopped successfully");
    return IDCU_ERR_OK;
}

IDCU_REGISTER_MODULE(http_management, IDCU_MODULE_VERSION(1, 0, 0), 
                    http_management_module_init, http_management_module_run, http_management_module_stop);
