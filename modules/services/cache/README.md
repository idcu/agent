# Cache Service Module

## 概述
数据缓存服务模块，提供高性能的内存缓存功能。

## 功能特性
- 支持多种缓存策略：LRU、FIFO、LFU
- TTL（Time-To-Live）过期机制
- 命中率统计
- 线程安全
- 便捷的 API（put/get/remove/clear）
- 支持二进制数据、字符串、整数

## 使用示例

```c
#include "cache.h"

// 初始化缓存
idcu_Cache* cache = NULL;
idcu_cache_init(&cache, 1024, IDCU_CACHE_POLICY_LRU);

// 存储数据
idcu_cache_put_string(cache, "key1", "hello world", 60000);
idcu_cache_put_int(cache, "counter", 123, 0);

// 获取数据
char buffer[256];
idcu_cache_get_string(cache, "key1", buffer, sizeof(buffer));

int64_t value;
idcu_cache_get_int(cache, "counter", &value);

// 清理
idcu_cache_destroy(cache);
```
