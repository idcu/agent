#ifndef IDCU_INTEGRATIONS_HTTP_CLIENT_INTEGRATION_H
#define IDCU_INTEGRATIONS_HTTP_CLIENT_INTEGRATION_H

#include "idcu/common/error_code.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        int initialized;
    } idcu_HttpClientIntegration;

    int  idcu_http_client_integration_init(idcu_HttpClientIntegration* integration);
    void idcu_http_client_integration_destroy(idcu_HttpClientIntegration* integration);

#ifdef __cplusplus
}
#endif

#endif  // IDCU_INTEGRATIONS_HTTP_CLIENT_INTEGRATION_H
