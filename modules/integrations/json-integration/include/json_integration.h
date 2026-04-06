#ifndef IDCU_INTEGRATIONS_JSON_INTEGRATION_H
#define IDCU_INTEGRATIONS_JSON_INTEGRATION_H

#include "idcu/common/error_code.h"
#include "idcu/json/json.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int initialized;
} idcu_JsonIntegration;

int idcu_json_integration_init(idcu_JsonIntegration* integration);
void idcu_json_integration_destroy(idcu_JsonIntegration* integration);

#ifdef __cplusplus
}
#endif

#endif // IDCU_INTEGRATIONS_JSON_INTEGRATION_H
