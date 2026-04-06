#ifndef IDCU_INTEGRATIONS_CONFIG_INTEGRATION_H
#define IDCU_INTEGRATIONS_CONFIG_INTEGRATION_H

#include "idcu/common/error_code.h"
#include "idcu/config/config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int initialized;
    char config_file[1024];
} idcu_ConfigIntegration;

int idcu_config_integration_init(idcu_ConfigIntegration* integration, const char* config_file);
void idcu_config_integration_destroy(idcu_ConfigIntegration* integration);

int idcu_config_integration_reload(idcu_ConfigIntegration* integration);
int idcu_config_integration_save(idcu_ConfigIntegration* integration, const char* file_path);

#ifdef __cplusplus
}
#endif

#endif // IDCU_INTEGRATIONS_CONFIG_INTEGRATION_H
