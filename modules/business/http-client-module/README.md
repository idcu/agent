# idcu-http-client-module

HTTP client business module for IDCU Agent, providing:
- HTTP GET requests
- HTTP POST requests
- Connection pooling
- Timeout handling
- SSL/TLS support

## Usage

```c
#include <idcu/http_client_module/http_client_module.h>

int main() {
    idcu_HttpClientModule hcm;
    int ret = idcu_http_client_module_init(&hcm);
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_http_client_module_start(&hcm);
    if (ret != IDCU_ERR_OK) {
        idcu_http_client_module_destroy(&hcm);
        return 1;
    }
    
    char response[4096];
    size_t response_size;
    idcu_http_client_module_get(&hcm, "https://example.com", response, &response_size);
    
    idcu_http_client_module_stop(&hcm);
    idcu_http_client_module_destroy(&hcm);
    
    return 0;
}
```
