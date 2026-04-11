#include <idcu/http_management_module/http_management_module.h>
#include <string.h>

int idcu_http_management_module_init(idcu_HttpManagementModule* hmm) {
    if (!hmm) {
        return IDCU_ERR_INVALID_ARG;
    }

    memset(hmm, 0, sizeof(idcu_HttpManagementModule));
    hmm->initialized = 0;

    idcu_HttpServerConfig server_config = {
        .port = 8080,
        .max_connections = 100
    };

    int ret = idcu_http_server_create(&server_config, &hmm->http_server);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    ret = idcu_management_create(&hmm->management);
    if (ret != IDCU_ERR_OK) {
        idcu_http_server_destroy(hmm->http_server);
        return ret;
    }

    hmm->initialized = 1;
    return IDCU_ERR_OK;
}

int idcu_http_management_module_start(idcu_HttpManagementModule* hmm) {
    if (!hmm || !hmm->initialized) {
        return IDCU_ERR_INVALID_STATE;
    }
    return IDCU_ERR_OK;
}

int idcu_http_management_module_stop(idcu_HttpManagementModule* hmm) {
    if (!hmm || !hmm->initialized) {
        return IDCU_ERR_INVALID_STATE;
    }
    return IDCU_ERR_OK;
}

void idcu_http_management_module_destroy(idcu_HttpManagementModule* hmm) {
    if (!hmm) {
        return;
    }

    if (hmm->management) {
        idcu_management_destroy(hmm->management);
        hmm->management = NULL;
    }

    if (hmm->http_server) {
        idcu_http_server_destroy(hmm->http_server);
        hmm->http_server = NULL;
    }

    hmm->initialized = 0;
    memset(hmm, 0, sizeof(idcu_HttpManagementModule));
}

idcu_HttpServer* idcu_http_management_module_get_server(idcu_HttpManagementModule* hmm) {
    if (!hmm) {
        return NULL;
    }
    return hmm->http_server;
}

idcu_Management* idcu_http_management_module_get_management(idcu_HttpManagementModule* hmm) {
    if (!hmm) {
        return NULL;
    }
    return hmm->management;
}
