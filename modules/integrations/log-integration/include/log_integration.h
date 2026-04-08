#ifndef IDCU_INTEGRATIONS_LOG_INTEGRATION_H
#define IDCU_INTEGRATIONS_LOG_INTEGRATION_H

#include "idcu/common/error_code.h"
#include "idcu/log/log.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        int           initialized;
        char          log_file[256];
        idcu_LogLevel log_level;
    } idcu_LogIntegration;

    int  idcu_log_integration_init(idcu_LogIntegration* integration, const char* log_file,
                                   idcu_LogLevel level);
    void idcu_log_integration_destroy(idcu_LogIntegration* integration);

    int idcu_log_integration_set_level(idcu_LogIntegration* integration, idcu_LogLevel level);
    idcu_LogLevel idcu_log_integration_get_level(idcu_LogIntegration* integration);

#ifdef __cplusplus
}
#endif

#endif  // IDCU_INTEGRATIONS_LOG_INTEGRATION_H
