#include <idcu/http_client_module/http_client_module.h>
#include <string.h>

int idcu_http_client_module_init(idcu_HttpClientModule* hcm) {
    if (!hcm) {
        return IDCU_ERR_INVALID_ARG;
    }

    memset(hcm, 0, sizeof(idcu_HttpClientModule));
    hcm->initialized = 0;

    int ret = idcu_http_client_create(&hcm->http_client);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    hcm->initialized = 1;
    return IDCU_ERR_OK;
}

int idcu_http_client_module_start(idcu_HttpClientModule* hcm) {
    if (!hcm || !hcm->initialized) {
        return IDCU_ERR_INVALID_STATE;
    }
    return IDCU_ERR_OK;
}

int idcu_http_client_module_stop(idcu_HttpClientModule* hcm) {
    if (!hcm || !hcm->initialized) {
        return IDCU_ERR_INVALID_STATE;
    }
    return IDCU_ERR_OK;
}

void idcu_http_client_module_destroy(idcu_HttpClientModule* hcm) {
    if (!hcm) {
        return;
    }

    if (hcm->http_client) {
        idcu_http_client_destroy(hcm->http_client);
        hcm->http_client = NULL;
    }

    hcm->initialized = 0;
    memset(hcm, 0, sizeof(idcu_HttpClientModule));
}

int idcu_http_client_module_get(idcu_HttpClientModule* hcm, const char* url, char* response, size_t* response_size) {
    if (!hcm || !hcm->http_client || !url || !response || !response_size) {
        return IDCU_ERR_INVALID_ARG;
    }
    *response_size = 0;
    if (*response_size > 0) {
        response[0] = '\0';
    }
    return IDCU_ERR_OK;
}

int idcu_http_client_module_post(idcu_HttpClientModule* hcm, const char* url, const char* data, size_t data_size, char* response, size_t* response_size) {
    if (!hcm || !hcm->http_client || !url || !response || !response_size) {
        return IDCU_ERR_INVALID_ARG;
    }
    (void)data;
    (void)data_size;
    *response_size = 0;
    if (*response_size > 0) {
        response[0] = '\0';
    }
    return IDCU_ERR_OK;
}

idcu_HttpClient* idcu_http_client_module_get_client(idcu_HttpClientModule* hcm) {
    if (!hcm) {
        return NULL;
    }
    return hcm->http_client;
}
