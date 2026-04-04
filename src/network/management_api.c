#include "network/management_api.h"
#include "module/module_registry.h"
#include "module/module_version.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static const char* module_state_to_string(idcu_ModuleState state) {
    switch (state) {
        case IDCU_MOD_STATE_UNINIT: return "uninitialized";
        case IDCU_MOD_STATE_INITED: return "initialized";
        case IDCU_MOD_STATE_RUNNING: return "running";
        case IDCU_MOD_STATE_STOPPED: return "stopped";
        case IDCU_MOD_STATE_ERROR: return "error";
        default: return "unknown";
    }
}

static int handle_get_modules(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    
    char json[IDCU_HTTP_MAX_BODY];
    int offset = 0;
    
    offset += snprintf(json + offset, sizeof(json) - offset, "[");
    
    idcu_MicroKernel* kernel = (idcu_MicroKernel*)user_data;
    if (kernel && kernel->registry) {
        int count = idcu_module_registry_get_count(kernel->registry);
        for (int i = 0; i < count; i++) {
            const idcu_RegisteredModule* mod = idcu_module_registry_get_at(kernel->registry, i);
            if (mod) {
                char ver_str[IDCU_VERSION_STR_MAX];
                idcu_version_format(&mod->iface->version, ver_str, sizeof(ver_str));
                
                if (i > 0) {
                    offset += snprintf(json + offset, sizeof(json) - offset, ",");
                }
                
                offset += snprintf(json + offset, sizeof(json) - offset,
                    "{\"id\":%u,\"name\":\"%s\",\"version\":\"%s\",\"state\":\"%s\"}",
                    mod->module_id, mod->iface->name, ver_str,
                    module_state_to_string(mod->state));
            }
        }
    }
    
    offset += snprintf(json + offset, sizeof(json) - offset, "]");
    
    idcu_http_response_set_json_body(response, json);
    return IDCU_ERR_OK;
}

static int handle_get_module(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    
    idcu_MicroKernel* kernel = (idcu_MicroKernel*)user_data;
    if (!kernel || !kernel->registry) {
        idcu_http_response_set_status(response, 500);
        idcu_http_response_set_json_body(response, "{\"error\":\"Internal server error\"}");
        return IDCU_ERR_OK;
    }
    
    const char* path = request->path;
    const char* module_name = strrchr(path, '/');
    if (!module_name) {
        idcu_http_response_set_status(response, 400);
        idcu_http_response_set_json_body(response, "{\"error\":\"Invalid path\"}");
        return IDCU_ERR_OK;
    }
    module_name++;
    
    const idcu_RegisteredModule* mod = idcu_module_registry_find_by_name(kernel->registry, module_name);
    if (!mod) {
        idcu_http_response_set_status(response, 404);
        idcu_http_response_set_json_body(response, "{\"error\":\"Module not found\"}");
        return IDCU_ERR_OK;
    }
    
    char json[IDCU_HTTP_MAX_BODY];
    char ver_str[IDCU_VERSION_STR_MAX];
    idcu_version_format(&mod->iface->version, ver_str, sizeof(ver_str));
    
    snprintf(json, sizeof(json),
        "{\"id\":%u,\"name\":\"%s\",\"version\":\"%s\",\"state\":\"%s\"}",
        mod->module_id, mod->iface->name, ver_str,
        module_state_to_string(mod->state));
    
    idcu_http_response_set_json_body(response, json);
    return IDCU_ERR_OK;
}

static int handle_start_module(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    
    idcu_MicroKernel* kernel = (idcu_MicroKernel*)user_data;
    if (!kernel || !kernel->registry) {
        idcu_http_response_set_status(response, 500);
        idcu_http_response_set_json_body(response, "{\"error\":\"Internal server error\"}");
        return IDCU_ERR_OK;
    }
    
    const char* path = request->path;
    const char* module_name = strrchr(path, '/');
    if (!module_name) {
        idcu_http_response_set_status(response, 400);
        idcu_http_response_set_json_body(response, "{\"error\":\"Invalid path\"}");
        return IDCU_ERR_OK;
    }
    module_name++;
    
    const idcu_RegisteredModule* mod = idcu_module_registry_find_by_name(kernel->registry, module_name);
    if (!mod) {
        idcu_http_response_set_status(response, 404);
        idcu_http_response_set_json_body(response, "{\"error\":\"Module not found\"}");
        return IDCU_ERR_OK;
    }
    
    int ret = idcu_module_registry_init_module(kernel->registry, mod->module_id);
    if (ret != IDCU_ERR_OK) {
        idcu_http_response_set_status(response, 500);
        idcu_http_response_set_json_body(response, "{\"error\":\"Failed to initialize module\"}");
        return IDCU_ERR_OK;
    }
    
    ret = idcu_module_registry_run_module(kernel->registry, mod->module_id);
    if (ret != IDCU_ERR_OK) {
        idcu_http_response_set_status(response, 500);
        idcu_http_response_set_json_body(response, "{\"error\":\"Failed to start module\"}");
        return IDCU_ERR_OK;
    }
    
    idcu_http_response_set_status(response, 200);
    idcu_http_response_set_json_body(response, "{\"success\":true,\"message\":\"Module started\"}");
    return IDCU_ERR_OK;
}

static int handle_stop_module(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    
    idcu_MicroKernel* kernel = (idcu_MicroKernel*)user_data;
    if (!kernel || !kernel->registry) {
        idcu_http_response_set_status(response, 500);
        idcu_http_response_set_json_body(response, "{\"error\":\"Internal server error\"}");
        return IDCU_ERR_OK;
    }
    
    const char* path = request->path;
    const char* module_name = strrchr(path, '/');
    if (!module_name) {
        idcu_http_response_set_status(response, 400);
        idcu_http_response_set_json_body(response, "{\"error\":\"Invalid path\"}");
        return IDCU_ERR_OK;
    }
    module_name++;
    
    const idcu_RegisteredModule* mod = idcu_module_registry_find_by_name(kernel->registry, module_name);
    if (!mod) {
        idcu_http_response_set_status(response, 404);
        idcu_http_response_set_json_body(response, "{\"error\":\"Module not found\"}");
        return IDCU_ERR_OK;
    }
    
    int ret = idcu_module_registry_stop_module(kernel->registry, mod->module_id);
    if (ret != IDCU_ERR_OK) {
        idcu_http_response_set_status(response, 500);
        idcu_http_response_set_json_body(response, "{\"error\":\"Failed to stop module\"}");
        return IDCU_ERR_OK;
    }
    
    idcu_http_response_set_status(response, 200);
    idcu_http_response_set_json_body(response, "{\"success\":true,\"message\":\"Module stopped\"}");
    return IDCU_ERR_OK;
}

static int handle_get_health(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    (void)user_data;
    
    idcu_http_response_set_json_body(response, "{\"status\":\"healthy\"}");
    return IDCU_ERR_OK;
}

int idcu_management_api_register_routes(idcu_HttpServer* server) {
    if (!server) return IDCU_ERR_INVALID_PARAM;
    
    int ret;
    
    ret = idcu_http_server_register_route(server, IDCU_HTTP_METHOD_GET, "/health",
                                          handle_get_health, server->kernel);
    if (ret != IDCU_ERR_OK) return ret;
    
    ret = idcu_http_server_register_route(server, IDCU_HTTP_METHOD_GET, "/api/v1/modules",
                                          handle_get_modules, server->kernel);
    if (ret != IDCU_ERR_OK) return ret;
    
    IDCU_LOG_INFO("Management API routes registered successfully");
    return IDCU_ERR_OK;
}
