# idcu-cache-module

IDCU Agent 的缓存业务模块，提供：
- 内存缓存
- 缓存过期（TTL）
- 缓存淘汰策略
- 缓存统计
- 缓存持久化

## 使用方法

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
    
    const char* value = "测试数据";
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

## 许可证

本库采用 [Apache License 2.0](../../../LICENSE) 许可证。
