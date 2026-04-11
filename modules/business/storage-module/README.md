# idcu-storage-module

IDCU Agent 的存储业务模块，提供：
- 持久化存储
- 键值存储
- 数据序列化
- 数据备份和恢复
- 存储加密

## 使用方法

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
    
    const char* value = "持久化数据";
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

## 许可证

本库采用 [Apache License 2.0](../../../LICENSE) 许可证。
