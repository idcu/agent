#ifndef IDCU_HTTP_CLIENT_MODULE_HTTP_CLIENT_MODULE_H
#define IDCU_HTTP_CLIENT_MODULE_HTTP_CLIENT_MODULE_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/http_client/http_client.h>
#include <idcu/sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    idcu_HttpClient* http_client;
    int initialized;
    void* user_data;
} idcu_HttpClientModule;

int idcu_http_client_module_init(idcu_HttpClientModule* hcm);
int idcu_http_client_module_start(idcu_HttpClientModule* hcm);
int idcu_http_client_module_stop(idcu_HttpClientModule* hcm);
void idcu_http_client_module_destroy(idcu_HttpClientModule* hcm);

int idcu_http_client_module_get(idcu_HttpClientModule* hcm, const char* url, char* response, size_t* response_size);
int idcu_http_client_module_post(idcu_HttpClientModule* hcm, const char* url, const char* data, size_t data_size, char* response, size_t* response_size);

idcu_HttpClient* idcu_http_client_module_get_client(idcu_HttpClientModule* hcm);

#ifdef __cplusplus
}
#endif

#endif
