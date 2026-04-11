#ifndef IDCU_HTTP_MANAGEMENT_MODULE_HTTP_MANAGEMENT_MODULE_H
#define IDCU_HTTP_MANAGEMENT_MODULE_HTTP_MANAGEMENT_MODULE_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/http_server/http_server.h>
#include <idcu/management/management.h>
#include <idcu/sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    idcu_HttpServer* http_server;
    idcu_Management* management;
    int initialized;
    void* user_data;
} idcu_HttpManagementModule;

int idcu_http_management_module_init(idcu_HttpManagementModule* hmm);
int idcu_http_management_module_start(idcu_HttpManagementModule* hmm);
int idcu_http_management_module_stop(idcu_HttpManagementModule* hmm);
void idcu_http_management_module_destroy(idcu_HttpManagementModule* hmm);

idcu_HttpServer* idcu_http_management_module_get_server(idcu_HttpManagementModule* hmm);
idcu_Management* idcu_http_management_module_get_management(idcu_HttpManagementModule* hmm);

#ifdef __cplusplus
}
#endif

#endif
