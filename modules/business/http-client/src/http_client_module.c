#include "module_def.h"
#include "http_client.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

static idcu_HttpClient g_http_client;
static int g_initialized = 0;

static int http_client_module_init() {
    IDCU_LOG_INFO("[http_client_module] Initializing HTTP client module");
    
    int ret = idcu_http_client_init(&g_http_client, 30000);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("[http_client_module] Failed to initialize HTTP client: %d", ret);
        return -1;
    }
    
    g_initialized = 1;
    IDCU_LOG_INFO("[http_client_module] HTTP client module initialized successfully");
    return 0;
}

static int http_client_module_run() {
    return 0;
}

static int http_client_module_stop() {
    if (g_initialized) {
        IDCU_LOG_INFO("[http_client_module] Stopping HTTP client module");
        idcu_http_client_destroy(&g_http_client);
        g_initialized = 0;
    }
    return 0;
}

int idcu_http_client_module_get(const char* url, char* response_body, size_t* response_len) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[http_client_module] Module not initialized");
        return -1;
    }
    
    idcu_HttpClientResponse resp;
    int ret = idcu_http_client_response_init(&resp);
    if (ret != IDCU_ERR_OK) {
        return -1;
    }
    
    ret = idcu_http_client_get(&g_http_client, url, &resp);
    if (ret != IDCU_ERR_OK) {
        idcu_http_client_response_destroy(&resp);
        return -1;
    }
    
    if (response_body && response_len && *response_len > 0) {
        size_t copy_len = resp.body_length;
        if (copy_len > *response_len - 1) {
            copy_len = *response_len - 1;
        }
        memcpy(response_body, resp.body, copy_len);
        response_body[copy_len] = '\0';
        *response_len = copy_len;
    }
    
    idcu_http_client_response_destroy(&resp);
    return resp.status_code;
}

int idcu_http_client_module_post(const char* url, const char* body, size_t body_len,
                                 char* response_body, size_t* response_len) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[http_client_module] Module not initialized");
        return -1;
    }
    
    idcu_HttpClientResponse resp;
    int ret = idcu_http_client_response_init(&resp);
    if (ret != IDCU_ERR_OK) {
        return -1;
    }
    
    ret = idcu_http_client_post(&g_http_client, url, body, body_len, &resp);
    if (ret != IDCU_ERR_OK) {
        idcu_http_client_response_destroy(&resp);
        return -1;
    }
    
    if (response_body && response_len && *response_len > 0) {
        size_t copy_len = resp.body_length;
        if (copy_len > *response_len - 1) {
            copy_len = *response_len - 1;
        }
        memcpy(response_body, resp.body, copy_len);
        response_body[copy_len] = '\0';
        *response_len = copy_len;
    }
    
    idcu_http_client_response_destroy(&resp);
    return resp.status_code;
}

IDCU_REGISTER_MODULE(http_client_module, IDCU_MODULE_VERSION(1, 0, 0), 
                     http_client_module_init, http_client_module_run, http_client_module_stop);
