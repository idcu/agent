# Cache Module

## 概述
数据缓存业务模块，提供便捷的缓存管理功能。

## 功能特性
- 多缓存实例管理
- 完整的缓存 CRUD 操作
- 模块生命周期管理

## 使用示例

```c
// 创建缓存实例
int cache_id = 0;
idcu_module_call(cache_module, create_cache, &cache_id, 1000, IDCU_CACHE_POLICY_LRU);

// 存储数据
idcu_module_call(cache_module, put_string, cache_id, "key1", "value1", 60000);

// 获取数据
char buffer[256];
idcu_module_call(cache_module, get_string, cache_id, "key1", buffer, sizeof(buffer));
```
