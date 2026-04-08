#include "idcu/management/http_api.h"
#include "idcu/json/json.h"
#include "idcu/log/log.h"
#include "idcu/management/management.h"
#include <stdio.h>
#include <string.h>

static int handle_status(idcu_HttpRequest *request, idcu_HttpResponse *response, void *user_data) {
    (void)request;
    (void)user_data;

    idcu_AgentStatus status;
    idcu_management_get_agent_status(&status);

    char json[512];
    snprintf(json, sizeof(json),
             "{\"version\":\"%s\",\"uptime_seconds\":%llu,\"uptime\":\"%s\",\"running\":%s}",
             status.version, (unsigned long long)status.uptime_seconds, status.uptime_str,
             status.running ? "true" : "false");

    idcu_http_response_set_status(response, 200);
    idcu_http_response_add_header(response, "Content-Type", "application/json");
    idcu_http_response_set_json_body(response, json);

    return IDCU_ERR_SUCCESS;
}

static int handle_modules(idcu_HttpRequest *request, idcu_HttpResponse *response, void *user_data) {
    (void)request;
    (void)user_data;

    idcu_ModuleDetail modules[64];
    size_t actual_count = 0;
    idcu_management_get_modules(modules, 64, &actual_count);

    char json[4096];
    int offset = 0;
    offset += snprintf(json + offset, sizeof(json) - offset, "[");

    for (size_t i = 0; i < actual_count; i++) {
        if (i > 0) {
            offset += snprintf(json + offset, sizeof(json) - offset, ",");
        }
        offset +=
            snprintf(json + offset, sizeof(json) - offset,
                     "{\"name\":\"%s\",\"version\":\"%s\",\"author\":\"%s\","
                     "\"description\":\"%s\",\"priority\":%d,\"loaded\":%s,\"running\":%s}",
                     modules[i].info.name, modules[i].info.version, modules[i].info.author,
                     modules[i].info.description, modules[i].info.priority,
                     modules[i].loaded ? "true" : "false", modules[i].running ? "true" : "false");
    }

    offset += snprintf(json + offset, sizeof(json) - offset, "]");

    idcu_http_response_set_status(response, 200);
    idcu_http_response_add_header(response, "Content-Type", "application/json");
    idcu_http_response_set_json_body(response, json);

    return IDCU_ERR_SUCCESS;
}

static int handle_module_load(idcu_HttpRequest *request, idcu_HttpResponse *response,
                              void *user_data) {
    (void)user_data;

    char module_name[128];
    const char *body = request->body;

    if (sscanf(body, "{\"name\":\"%127[^\"]\"}", module_name) != 1) {
        idcu_http_response_set_status(response, 400);
        idcu_http_response_set_body(response, "{\"error\":\"Invalid request\"}", 23);
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_management_load_module(module_name);
    if (ret != IDCU_ERR_SUCCESS) {
        idcu_http_response_set_status(response, 500);
        char error[256];
        snprintf(error, sizeof(error), "{\"error\":\"Failed to load module: %d\"}", ret);
        idcu_http_response_set_body(response, error, strlen(error));
        return ret;
    }

    idcu_http_response_set_status(response, 200);
    idcu_http_response_set_body(response, "{\"success\":true}", 16);

    return IDCU_ERR_SUCCESS;
}

static int handle_module_unload(idcu_HttpRequest *request, idcu_HttpResponse *response,
                                void *user_data) {
    (void)user_data;

    char module_name[128];
    const char *body = request->body;

    if (sscanf(body, "{\"name\":\"%127[^\"]\"}", module_name) != 1) {
        idcu_http_response_set_status(response, 400);
        idcu_http_response_set_body(response, "{\"error\":\"Invalid request\"}", 23);
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_management_unload_module(module_name);
    if (ret != IDCU_ERR_SUCCESS) {
        idcu_http_response_set_status(response, 500);
        char error[256];
        snprintf(error, sizeof(error), "{\"error\":\"Failed to unload module: %d\"}", ret);
        idcu_http_response_set_body(response, error, strlen(error));
        return ret;
    }

    idcu_http_response_set_status(response, 200);
    idcu_http_response_set_body(response, "{\"success\":true}", 16);

    return IDCU_ERR_SUCCESS;
}

static int handle_healthcheck(idcu_HttpRequest *request, idcu_HttpResponse *response,
                              void *user_data) {
    (void)request;
    (void)user_data;

    int ret = idcu_management_trigger_health_check();
    if (ret != IDCU_ERR_SUCCESS) {
        idcu_http_response_set_status(response, 500);
        char error[256];
        snprintf(error, sizeof(error), "{\"error\":\"Failed to trigger health check: %d\"}", ret);
        idcu_http_response_set_body(response, error, strlen(error));
        return ret;
    }

    idcu_http_response_set_status(response, 200);
    idcu_http_response_set_body(response, "{\"success\":true}", 16);

    return IDCU_ERR_SUCCESS;
}

static int handle_metrics(idcu_HttpRequest *request, idcu_HttpResponse *response, void *user_data) {
    (void)request;
    (void)user_data;

    char buffer[8192];
    idcu_management_get_metrics(buffer, sizeof(buffer));

    idcu_http_response_set_status(response, 200);
    idcu_http_response_add_header(response, "Content-Type", "text/plain");
    idcu_http_response_set_body(response, buffer, strlen(buffer));

    return IDCU_ERR_SUCCESS;
}

static int handle_config(idcu_HttpRequest *request, idcu_HttpResponse *response, void *user_data) {
    (void)request;
    (void)user_data;

    char buffer[8192];
    idcu_management_get_config(buffer, sizeof(buffer));

    idcu_http_response_set_status(response, 200);
    idcu_http_response_add_header(response, "Content-Type", "text/plain");
    idcu_http_response_set_body(response, buffer, strlen(buffer));

    return IDCU_ERR_SUCCESS;
}

int idcu_http_management_init(idcu_HttpManagementServer *server, const char *address,
                              uint16_t port) {
    if (!server || !address) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_http_server_init(&server->server, address, port);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    strncpy(server->address, address, sizeof(server->address) - 1);
    server->port = port;
    server->running = 0;

    idcu_http_server_register_route(&server->server, IDCU_HTTP_METHOD_GET, "/api/status",
                                    handle_status, NULL);
    idcu_http_server_register_route(&server->server, IDCU_HTTP_METHOD_GET, "/api/modules",
                                    handle_modules, NULL);
    idcu_http_server_register_route(&server->server, IDCU_HTTP_METHOD_POST, "/api/modules/load",
                                    handle_module_load, NULL);
    idcu_http_server_register_route(&server->server, IDCU_HTTP_METHOD_POST, "/api/modules/unload",
                                    handle_module_unload, NULL);
    idcu_http_server_register_route(&server->server, IDCU_HTTP_METHOD_POST, "/api/healthcheck",
                                    handle_healthcheck, NULL);
    idcu_http_server_register_route(&server->server, IDCU_HTTP_METHOD_GET, "/api/metrics",
                                    handle_metrics, NULL);
    idcu_http_server_register_route(&server->server, IDCU_HTTP_METHOD_GET, "/api/config",
                                    handle_config, NULL);

    IDCU_LOG_INFO("HTTP management server initialized on %s:%d", address, port);

    return IDCU_ERR_SUCCESS;
}

int idcu_http_management_start(idcu_HttpManagementServer *server) {
    if (!server || server->running) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_http_server_start(&server->server);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    server->running = 1;
    IDCU_LOG_INFO("HTTP management server started");

    return IDCU_ERR_SUCCESS;
}

int idcu_http_management_stop(idcu_HttpManagementServer *server) {
    if (!server || !server->running) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_http_server_stop(&server->server);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    server->running = 0;
    IDCU_LOG_INFO("HTTP management server stopped");

    return IDCU_ERR_SUCCESS;
}

void idcu_http_management_destroy(idcu_HttpManagementServer *server) {
    if (!server) {
        return;
    }

    if (server->running) {
        idcu_http_management_stop(server);
    }

    idcu_http_server_destroy(&server->server);
    IDCU_LOG_INFO("HTTP management server destroyed");
}

int idcu_http_management_poll(idcu_HttpManagementServer *server, int timeout_ms) {
    if (!server || !server->running) {
        return IDCU_ERR_INVALID_PARAM;
    }

    return idcu_http_server_poll(&server->server, timeout_ms);
}
