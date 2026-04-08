# 任务 5.9: cache-module - 缓存业务模块

## 目标

创建缓存业务模块，支持：
- 多种缓存策略（LRU、LFU、FIFO）
- 多级缓存
- 缓存过期
- 缓存持久化
- 缓存预热
- 缓存统计
- 分布式缓存集成

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/business/cache-module/include/idcu/cache_module
mkdir -p modules/business/cache-module/src/idcu/cache_module
mkdir -p modules/business/cache-module/tests
mkdir -p modules/business/cache-module/examples
```

### 2. 创建缓存模块头文件 (cache_module.h)

创建 `modules/business/cache-module/include/idcu/cache_module/cache_module.h`：

```c
#ifndef IDCU_CACHE_MODULE_CACHE_MODULE_H
#define IDCU_CACHE_MODULE_CACHE_MODULE_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include "idcu/sdk/sdk.h"
#include "idcu/cache/cache.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_CacheId;

typedef enum
{
    IDCU_CACHE_POLICY_LRU = 0,
    IDCU_CACHE_POLICY_LFU,
    IDCU_CACHE_POLICY_FIFO,
    IDCU_CACHE_POLICY_RANDOM
} idcu_CachePolicy;

typedef enum
{
    IDCU_CACHE_TIER_MEMORY = 0,
    IDCU_CACHE_TIER_DISK,
    IDCU_CACHE_TIER_REMOTE
} idcu_CacheTier;

typedef struct
{
    char key[256];
    char value[4096];
    size_t value_size;
    uint64_t created_time;
    uint64_t last_access_time;
    uint64_t access_count;
    uint64_t ttl_ms;
    idcu_CacheTier tier;
} idcu_CacheEntry;

typedef struct
{
    idcu_CacheId cache_id;
    char name[128];
    idcu_CachePolicy policy;
    size_t max_entries;
    size_t max_memory_bytes;
    uint64_t default_ttl_ms;
    int enable_persistence;
    char persistence_path[1024];
    int enable_stats;
} idcu_CacheConfig;

typedef struct
{
    uint64_t hits;
    uint64_t misses;
    uint64_t evictions;
    uint64_t sets;
    uint64_t gets;
    uint64_t deletes;
    uint64_t expired;
    size_t current_entries;
    size_t current_memory_bytes;
    uint64_t start_time;
} idcu_CacheStats;

typedef struct
{
    idcu_SdkContext* sdk;
    idcu_CacheConfig config;
    
    idcu_HashMap cache_map;
    idcu_Vector lru_list;
    idcu_Mutex cache_lock;
    
    idcu_CacheStats stats;
    idcu_Mutex stats_lock;
    
    int running;
} idcu_CacheModule;

int  idcu_cache_module_config_init(idcu_CacheConfig* config);

int  idcu_cache_module_init(idcu_CacheModule* module, idcu_SdkContext* sdk,
                              const idcu_CacheConfig* config);
void idcu_cache_module_destroy(idcu_CacheModule* module);

int  idcu_cache_module_start(idcu_CacheModule* module);
int  idcu_cache_module_stop(idcu_CacheModule* module);

int  idcu_cache_module_set(idcu_CacheModule* module, const char* key,
                            const void* value, size_t value_size);
int  idcu_cache_module_set_with_ttl(idcu_CacheModule* module, const char* key,
                                      const void* value, size_t value_size,
                                      uint64_t ttl_ms);

int  idcu_cache_module_get(idcu_CacheModule* module, const char* key,
                            void* value, size_t* value_size);
int  idcu_cache_module_exists(idcu_CacheModule* module, const char* key);

int  idcu_cache_module_delete(idcu_CacheModule* module, const char* key);
int  idcu_cache_module_clear(idcu_CacheModule* module);

int  idcu_cache_module_get_stats(idcu_CacheModule* module, idcu_CacheStats* stats);
int  idcu_cache_module_reset_stats(idcu_CacheModule* module);

int  idcu_cache_module_save_to_disk(idcu_CacheModule* module, const char* path);
int  idcu_cache_module_load_from_disk(idcu_CacheModule* module, const char* path);

int  idcu_cache_module_get_all_keys(idcu_CacheModule* module, idcu_Vector* keys);
int  idcu_cache_module_get_entry_count(idcu_CacheModule* module, size_t* count);

int  idcu_cache_module_warm_up(idcu_CacheModule* module, const idcu_Vector* keys);

int  idcu_cache_module_flush_expired(idcu_CacheModule* module);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建模块配置文件 (module.yaml)

创建 `modules/business/cache-module/module.yaml`：

```yaml
name: cache-module
version: 1.0.0
description: Cache business module for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-sdk
  - idcu-cache
  - idcu-storage
  - idcu-log

build:
  type: cmake
  targets:
    - cache-module

headers:
  - idcu/cache_module/cache_module.h

features:
  - policies: Multiple cache policies (LRU, LFU, FIFO)
  - multi_tier: Multi-level cache
  - expiration: Cache expiration
  - persistence: Cache persistence
  - warmup: Cache warm-up
  - stats: Cache statistics
  - distributed: Distributed cache integration

testing:
  enabled: true
  framework: internal
```

### 4. 创建 README.md

创建 `modules/business/cache-module/README.md`：

```markdown
# cache-module

IDCU Agent 的缓存业务模块。

## 功能特性

- **多种策略**: LRU、LFU、FIFO等缓存策略
- **多级缓存**: 多级缓存支持
- **过期管理**: 缓存过期管理
- **持久化**: 缓存持久化
- **预热**: 缓存预热
- **统计**: 缓存统计
- **分布式**: 分布式缓存集成

## 快速开始

### 初始化缓存模块

```c
#include "idcu/cache_module/cache_module.h"

idcu_CacheConfig config;
idcu_cache_module_config_init(&config);

config.policy = IDCU_CACHE_POLICY_LRU;
config.max_entries = 10000;
config.max_memory_bytes = 100 * 1024 * 1024;
config.default_ttl_ms = 3600 * 1000;
config.enable_persistence = 1;
config.enable_stats = 1;

idcu_CacheModule module;
idcu_cache_module_init(&module, sdk_context, &config);
```

### 启动缓存

```c
idcu_cache_module_start(&module);
```

### 设置缓存

```c
const char* key = "user:123";
const char* value = "{\"name\":\"John\",\"age\":30}";
idcu_cache_module_set(&module, key, value, strlen(value));
```

### 设置带TTL的缓存

```c
const char* key = "session:abc";
const char* value = "session_data";
idcu_cache_module_set_with_ttl(&module, key, value, strlen(value), 300000);
```

### 获取缓存

```c
char value[1024];
size_t value_size = sizeof(value);

if (idcu_cache_module_get(&module, "user:123", value, &value_size) == IDCU_ERR_OK) {
    printf("Value: %s\n", value);
}
```

### 检查缓存是否存在

```c
if (idcu_cache_module_exists(&module, "user:123")) {
    printf("Cache exists\n");
}
```

### 删除缓存

```c
idcu_cache_module_delete(&module, "user:123");
```

### 清空缓存

```c
idcu_cache_module_clear(&module);
```

### 获取缓存统计

```c
idcu_CacheStats stats;
idcu_cache_module_get_stats(&module, &stats);

printf("Hits: %" PRIu64 "\n", stats.hits);
printf("Misses: %" PRIu64 "\n", stats.misses);
printf("Hit rate: %.2f%%\n", 
       stats.hits + stats.misses > 0 
           ? (double)stats.hits / (stats.hits + stats.misses) * 100 
           : 0);
```

### 持久化缓存

```c
idcu_cache_module_save_to_disk(&module, "./cache.dat");
```

### 加载缓存

```c
idcu_cache_module_load_from_disk(&module, "./cache.dat");
```

### 预热缓存

```c
idcu_Vector keys;
idcu_vector_init(&keys, sizeof(char*));

const char* key1 = "user:123";
const char* key2 = "user:456";
idcu_vector_push(&keys, &key1);
idcu_vector_push(&keys, &key2);

idcu_cache_module_warm_up(&module, &keys);

idcu_vector_destroy(&keys);
```

### 清理过期缓存

```c
idcu_cache_module_flush_expired(&module);
```

### 停止缓存

```c
idcu_cache_module_stop(&module);
idcu_cache_module_destroy(&module);
```

## 缓存策略

| 策略 | 说明 |
|-----|------|
| LRU | 最近最少使用 |
| LFU | 最不经常使用 |
| FIFO | 先进先出 |
| RANDOM | 随机 |

## 缓存层级

| 层级 | 说明 |
|-----|------|
| MEMORY | 内存缓存 |
| DISK | 磁盘缓存 |
| REMOTE | 远程缓存 |

## API 文档

详见 [include/idcu/cache_module/cache_module.h](include/idcu/cache_module/cache_module.h)
```

## 验证检查清单

- [ ] 缓存模块头文件已创建
- [ ] 缓存模块实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以设置和获取缓存
- [ ] 缓存过期功能正常工作
- [ ] 缓存统计功能正常

## Git 提交

```bash
git add modules/business/cache-module/
git commit -m "feat: add cache-module business module

- Add multiple cache policies (LRU, LFU, FIFO)
- Add multi-level cache
- Add cache expiration
- Add cache persistence
- Add cache warm-up
- Add cache statistics
- Add distributed cache integration
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 缓存命中率低 | 缓存策略或TTL设置不当 | 调整缓存策略和TTL |
| 内存占用过高 | 缓存大小限制不当 | 调整缓存大小限制 |
| 持久化失败 | 磁盘空间或权限问题 | 检查磁盘空间和权限 |
