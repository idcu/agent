# idcu-storage-module

Storage business module for IDCU Agent, providing:
- Persistent storage
- Key-value storage
- Data serialization
- Data backup and restore
- Storage encryption

## Usage

```c
#include <idcu/storage_module/storage_module.h>

int main() {
    idcu_StorageModule sm;
    int ret = idcu_storage_module_init(&sm);
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_storage_module_start(&sm);
    if (ret != IDCU_ERR_OK) {
        idcu_storage_module_destroy(&sm);
        return 1;
    }
    
    const char* value = "persistent data";
    idcu_storage_module_put(&sm, "key1", value, strlen(value));
    
    void* out_value;
    size_t out_size;
    idcu_storage_module_get(&sm, "key1", &out_value, &out_size);
    
    idcu_storage_module_delete(&sm, "key1");
    
    idcu_storage_module_stop(&sm);
    idcu_storage_module_destroy(&sm);
    
    return 0;
}
```
