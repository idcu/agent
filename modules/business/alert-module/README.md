# idcu-alert-module

Alert business module for IDCU Agent, providing:
- Alert rule configuration
- Alert triggering and deduplication
- Multi-channel alert notifications (message bus, HTTP, etc.)
- Alert history records
- Alert queries and management

## Usage

```c
#include <idcu/alert_module/alert_module.h>

int main() {
    idcu_AlertModule am;
    int ret = idcu_alert_module_init(&am);
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_alert_module_start(&am);
    if (ret != IDCU_ERR_OK) {
        idcu_alert_module_destroy(&am);
        return 1;
    }
    
    idcu_alert_module_trigger(&am, "high_cpu_usage", NULL);
    
    idcu_alert_module_stop(&am);
    idcu_alert_module_destroy(&am);
    
    return 0;
}
```
