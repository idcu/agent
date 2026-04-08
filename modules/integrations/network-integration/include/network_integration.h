#ifndef IDCU_INTEGRATIONS_NETWORK_INTEGRATION_H
#define IDCU_INTEGRATIONS_NETWORK_INTEGRATION_H

#include "idcu/common/error_code.h"
#include "idcu/network/network_layer.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        int initialized;
    } idcu_NetworkIntegration;

    int  idcu_network_integration_init(idcu_NetworkIntegration* integration);
    void idcu_network_integration_destroy(idcu_NetworkIntegration* integration);

#ifdef __cplusplus
}
#endif

#endif  // IDCU_INTEGRATIONS_NETWORK_INTEGRATION_H
