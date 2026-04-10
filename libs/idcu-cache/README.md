# idcu-cache

内存缓存库 for IDCU Agent.

## 功能特性

- **多种淘汰策略**: LRU (默认)、LFU、FIFO、Random
- **TTL过期**: 支持每个缓存项的TTL设置
- **容量限制**: 支持最大条目数和最大内存限制
- **统计信息**: 命中/未命中/淘汰计数、命中率
- **线程安全**: 每个缓存实例独立互斥锁
- **哈希表+双向链表**: 高效的O(1)查找和更新
- **惰性过期**: 访问时检查过期，定期清理

## 使用方法

```c
#include <idcu/cache/cache.h>

// 初始化缓存
idcu_CacheConfig config = {
    .policy = IDCU_CACHE_POLICY_LRU,
    .max_entries = 1000,
    .max_memory = 1024 * 1024 * 10, // 10MB
    .default_ttl_ms = 3600000 // 1小时
};

idcu_Cache cache;
int ret = idcu_cache_init(&cache, &config);
if (ret != IDCU_ERR_OK) {
    // 处理错误
}

// 写入缓存
idcu_cache_put_string(&cache, "key1", "value1");
idcu_cache_put_int(&cache, "key2", 42);

// 带TTL写入
idcu_cache_put_ttl(&cache, "temp_key", "temp_value", 12, 60000); // 60秒过期

// 读取缓存
char buffer[256];
if (idcu_cache_get_string(&cache, "key1", buffer, sizeof(buffer)) == IDCU_ERR_OK) {
    printf("Value: %s\n", buffer);
}

// 获取统计信息
uint64_t hits = idcu_cache_get_hit_count(&cache);
uint64_t misses = idcu_cache_get_miss_count(&cache);
double rate = idcu_cache_get_hit_rate(&cache);
printf("Hit rate: %.2f%%\n", rate * 100);

// 删除缓存项
idcu_cache_remove(&cache, "key1");

// 清空缓存
idcu_cache_clear(&cache);

// 销毁缓存
idcu_cache_destroy(&cache);
```

## 淘汰策略

### LRU (Least Recently Used)
- 淘汰最近最少使用的项
- 最常用的策略，适合大多数场景

### LFU (Least Frequently Used)
- 淘汰使用频率最低的项
- 适合某些特定访问模式

### FIFO (First In First Out)
- 按顺序淘汰最早进入的项
- 简单但效果一般

### Random
- 随机淘汰
- 用作基准比较

## API参考

### 核心API
- `idcu_cache_init()` - 初始化缓存
- `idcu_cache_destroy()` - 销毁缓存

### 写入操作
- `idcu_cache_put()` - 写入二进制数据
- `idcu_cache_put_ttl()` - 写入带TTL的数据
- `idcu_cache_put_string()` - 写入字符串
- `idcu_cache_put_int()` - 写入整数

### 读取操作
- `idcu_cache_get()` - 读取二进制数据
- `idcu_cache_get_string()` - 读取字符串
- `idcu_cache_get_int()` - 读取整数
- `idcu_cache_contains()` - 检查键是否存在

### 管理操作
- `idcu_cache_remove()` - 删除缓存项
- `idcu_cache_clear()` - 清空所有缓存
- `idcu_cache_get_count()` - 获取缓存项数量
- `idcu_cache_get_memory_usage()` - 获取内存使用量

### 统计信息
- `idcu_cache_get_hit_count()` - 获取命中次数
- `idcu_cache_get_miss_count()` - 获取未命中次数
- `idcu_cache_get_evict_count()` - 获取淘汰次数
- `idcu_cache_get_hit_rate()` - 获取命中率

## 性能指标

- 缓存读写延迟：≤ 100ns（平均）
- 支持条目数：≤ 100000
- 最大键长：256字节
- 最大值长：64KB
- 命中率：≥ 80%（正常工作负载）

## 配置建议

```c
// 小容量高速缓存
idcu_CacheConfig small = {
    .policy = IDCU_CACHE_POLICY_LRU,
    .max_entries = 100,
    .max_memory = 1024 * 1024, // 1MB
    .default_ttl_ms = 60000 // 1分钟
};

// 中等容量
idcu_CacheConfig medium = {
    .policy = IDCU_CACHE_POLICY_LRU,
    .max_entries = 10000,
    .max_memory = 1024 * 1024 * 100, // 100MB
    .default_ttl_ms = 3600000 // 1小时
};

// 大容量
idcu_CacheConfig large = {
    .policy = IDCU_CACHE_POLICY_LRU,
    .max_entries = 100000,
    .max_memory = 1024 * 1024 * 1024, // 1GB
    .default_ttl_ms = 3600000 * 24 // 24小时
};
```

## 依赖

- idcu-common

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

MIT
