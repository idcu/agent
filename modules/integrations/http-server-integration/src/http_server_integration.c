#include "http_server_integration.h"
#include "idcu/log/log.h"
#include "module_def.h"
#include <string.h>

static idcu_HttpServerIntegration *g_http_server_integration = NULL;

int idcu_http_server_integration_init(idcu_HttpServerIntegration *integration,
                                      const char *bind_address, uint16_t port) {
    if (!integration) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memset(integration, 0, sizeof(idcu_HttpServerIntegration));

    if (bind_address) {
        strncpy(integration->bind_address, bind_address, sizeof(integration->bind_address) - 1);
    } else {
        strcpy(integration->bind_address, "0.0.0.0");
    }
    integration->port = port;

    int ret = idcu_http_server_init(&integration->server, integration->bind_address, port);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to initialize HTTP server");
        return ret;
    }

    integration->initialized = 1;
    g_http_server_integration = integration;
    IDCU_LOG_INFO("HTTP server integration initialized on %s:%d", integration->bind_address,
                  integration->port);
    return IDCU_ERR_OK;
}

void idcu_http_server_integration_destroy(idcu_HttpServerIntegration *integration) {
    if (!integration || !integration->initialized) {
        return;
    }

    idcu_http_server_integration_stop(integration);
    idcu_http_server_destroy(&integration->server);
    integration->initialized = 0;
    g_http_server_integration = NULL;
    IDCU_LOG_INFO("HTTP server integration destroyed");
}

int idcu_http_server_integration_start(idcu_HttpServerIntegration *integration) {
    if (!integration || !integration->initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

    int ret = idcu_http_server_start(&integration->server);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to start HTTP server");
        return ret;
    }

    IDCU_LOG_INFO("HTTP server integration started");
    return IDCU_ERR_OK;
}

int idcu_http_server_integration_stop(idcu_HttpServerIntegration *integration) {
    if (!integration || !integration->initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

    idcu_http_server_stop(&integration->server);
    IDCU_LOG_INFO("HTTP server integration stopped");
    return IDCU_ERR_OK;
}

int idcu_http_server_integration_poll(idcu_HttpServerIntegration *integration, int timeout_ms) {
    if (!integration || !integration->initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

    return idcu_http_server_poll(&integration->server, timeout_ms);
}

idcu_HttpServer *idcu_http_server_integration_get_server(idcu_HttpServerIntegration *integration) {
    if (!integration || !integration->initialized) {
        return NULL;
    }
    return &integration->server;
}

static idcu_HttpServerIntegration s_http_server_integration;

static int http_server_integration_module_init(void) {
    return idcu_http_server_integration_init(&s_http_server_integration, "0.0.0.0", 8080);
}

static int http_server_integration_module_run(void) {
    return idcu_http_server_integration_poll(&s_http_server_integration, 10);
}

static int http_server_integration_module_stop(void) {
    idcu_http_server_integration_destroy(&s_http_server_integration);
    return 0;
}

IDCU_REGISTER_MODULE(http_server_integration, IDCU_MODULE_VERSION(1, 0, 0),
                     http_server_integration_module_init, http_server_integration_module_run,
                     http_server_integration_module_stop);
