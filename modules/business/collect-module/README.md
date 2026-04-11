# idcu-collect-module

Collect business module for IDCU Agent, providing:
- Data collection from various sources
- Device data collection
- Network data collection
- Data aggregation
- Data filtering

## Usage

```c
#include <idcu/collect_module/collect_module.h>

int main() {
    idcu_CollectModule cm;
    int ret = idcu_collect_module_init(&cm);
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_collect_module_start(&cm);
    if (ret != IDCU_ERR_OK) {
        idcu_collect_module_destroy(&cm);
        return 1;
    }
    
    idcu_collect_module_stop(&cm);
    idcu_collect_module_destroy(&cm);
    
    return 0;
}
```
