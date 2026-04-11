# idcu-healthcheck-module

Healthcheck business module for IDCU Agent, providing:
- Periodic health checks
- Custom health checks
- Health status aggregation
- Health status notifications
- HTTP API queries

## Usage

```c
#include <idcu/healthcheck_module/healthcheck_module.h>

int main() {
    idcu_HealthCheckModule hcm;
    int ret = idcu_healthcheck_module_init(&hcm);
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_healthcheck_module_start(&hcm);
    if (ret != IDCU_ERR_OK) {
        idcu_healthcheck_module_destroy(&hcm);
        return 1;
    }
    
    idcu_HealthCheckResult result;
    idcu_healthcheck_module_get_status(&hcm, &result);
    printf("Health status: %d\n", result.status);
    
    idcu_healthcheck_module_stop(&hcm);
    idcu_healthcheck_module_destroy(&hcm);
    
    return 0;
}
```
