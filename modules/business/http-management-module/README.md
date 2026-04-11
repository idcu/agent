# idcu-http-management-module

HTTP management business module for IDCU Agent, providing:
- HTTP server for management APIs
- RESTful API endpoints
- System health monitoring API
- Configuration management API
- Module management API

## Usage

```c
#include <idcu/http_management_module/http_management_module.h>

int main() {
    idcu_HttpManagementModule hmm;
    int ret = idcu_http_management_module_init(&hmm);
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_http_management_module_start(&hmm);
    if (ret != IDCU_ERR_OK) {
        idcu_http_management_module_destroy(&hmm);
        return 1;
    }
    
    idcu_http_management_module_stop(&hmm);
    idcu_http_management_module_destroy(&hmm);
    
    return 0;
}
```
