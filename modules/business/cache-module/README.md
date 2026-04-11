# idcu-cache-module

Cache business module for IDCU Agent, providing:
- In-memory cache
- Cache expiration (TTL)
- Cache eviction policies
- Cache statistics
- Cache persistence

## Usage

```c
#include <idcu/cache_module/cache_module.h>

int main() {
    idcu_CacheModule cm;
    int ret = idcu_cache_module_init(&cm);
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_cache_module_start(&cm);
    if (ret != IDCU_ERR_OK) {
        idcu_cache_module_destroy(&cm);
        return 1;
    }
    
    const char* value = "test data";
    idcu_cache_module_set(&cm, "key1", value, strlen(value), 3600000);
    
    void* out_value;
    size_t out_size;
    idcu_cache_module_get(&cm, "key1", &out_value, &out_size);
    
    idcu_cache_module_delete(&cm, "key1");
    
    idcu_cache_module_stop(&cm);
    idcu_cache_module_destroy(&cm);
    
    return 0;
}
```
