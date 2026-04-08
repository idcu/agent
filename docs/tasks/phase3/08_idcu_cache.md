# 任务 3.8: idcu-cache - 内存缓存库

## 目标

创建内存缓存库，支持：
- LRU 缓存策略
- 缓存过期
- 最大容量限制
- 命中率统计
- 线程安全
- 多种缓存淘汰策略

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-cache/include/idcu/cache
mkdir -p libs/idcu-cache/src/idcu/cache
mkdir -p libs/idcu-cache/tests
mkdir -p libs/idcu-cache/examples
```

### 2. 创建缓存头文件 (cache.h)

创建 `libs/idcu-cache/include/idcu/cache/cache.h`：

```c
#ifndef IDCU_CACHE_CACHE_H
#define IDCU_CACHE_CACHE_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_CACHE_POLICY_LRU = 0,
    IDCU_CACHE_POLICY_LFU,
    IDCU_CACHE_POLICY_FIFO,
    IDCU_CACHE_POLICY_RANDOM
} idcu_CachePolicy;

typedef struct idcu_CacheEntry
{
    char* key;
    void* value;
    size_t value_size;
    uint64_t created_at;
    uint64_t last_accessed_at;
    uint64_t access_count;
    uint64_t expire_at;
    struct idcu_CacheEntry* prev;
    struct idcu_CacheEntry* next;
} idcu_CacheEntry;

typedef struct
{
    idcu_HashMap entries;
    idcu_CacheEntry* head;
    idcu_CacheEntry* tail;
    idcu_Mutex lock;
    idcu_CachePolicy policy;
    size_t max_entries;
    size_t current_entries;
    size_t max_memory;
    size_t current_memory;
    uint64_t default_ttl_ms;
    uint64_t hit_count;
    uint64_t miss_count;
    uint64_t evict_count;
    int initialized;
} idcu_Cache;

typedef struct
{
    idcu_CachePolicy policy;
    size_t max_entries;
    size_t max_memory;
    uint64_t default_ttl_ms;
} idcu_CacheConfig;

int  idcu_cache_config_init(idcu_CacheConfig* config);

int  idcu_cache_init(idcu_Cache* cache, const idcu_CacheConfig* config);
void idcu_cache_destroy(idcu_Cache* cache);

int  idcu_cache_put(idcu_Cache* cache, const char* key, const void* value, size_t value_size);
int  idcu_cache_put_with_ttl(idcu_Cache* cache, const char* key, const void* value, size_t value_size, uint64_t ttl_ms);
int  idcu_cache_put_string(idcu_Cache* cache, const char* key, const char* value);
int  idcu_cache_put_int(idcu_Cache* cache, const char* key, int64_t value);
int  idcu_cache_put_double(idcu_Cache* cache, const char* key, double value);

int  idcu_cache_get(idcu_Cache* cache, const char* key, void* buffer, size_t buffer_size, size_t* value_size);
int  idcu_cache_get_string(idcu_Cache* cache, const char* key, char* buffer, size_t buffer_size);
int  idcu_cache_get_int(idcu_Cache* cache, const char* key, int64_t* value);
int  idcu_cache_get_double(idcu_Cache* cache, const char* key, double* value);

int  idcu_cache_remove(idcu_Cache* cache, const char* key);
int  idcu_cache_clear(idcu_Cache* cache);
int  idcu_cache_exists(idcu_Cache* cache, const char* key);
size_t idcu_cache_size(idcu_Cache* cache);
size_t idcu_cache_memory_usage(idcu_Cache* cache);

uint64_t idcu_cache_get_hit_count(const idcu_Cache* cache);
uint64_t idcu_cache_get_miss_count(const idcu_Cache* cache);
uint64_t idcu_cache_get_evict_count(const idcu_Cache* cache);
double idcu_cache_get_hit_rate(const idcu_Cache* cache);
void idcu_cache_reset_stats(idcu_Cache* cache);

typedef struct
{
    size_t current_entries;
    size_t max_entries;
    size_t current_memory;
    size_t max_memory;
    uint64_t hit_count;
    uint64_t miss_count;
    uint64_t evict_count;
    double hit_rate;
} idcu_CacheStats;

void idcu_cache_get_stats(const idcu_Cache* cache, idcu_CacheStats* stats);

int idcu_cache_set_policy(idcu_Cache* cache, idcu_CachePolicy policy);
int idcu_cache_set_max_entries(idcu_Cache* cache, size_t max_entries);
int idcu_cache_set_max_memory(idcu_Cache* cache, size_t max_memory);
int idcu_cache_set_default_ttl(idcu_Cache* cache, uint64_t ttl_ms);

int idcu_cache_cleanup_expired(idcu_Cache* cache);

typedef struct
{
    char* key;
    void* value;
    size_t value_size;
    uint64_t age_ms;
} idcu_CacheEntryInfo;

int  idcu_cache_get_all_keys(idcu_Cache* cache, idcu_Vector* keys);
int  idcu_cache_get_all_entries(idcu_Cache* cache, idcu_Vector* entries);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `libs/idcu-cache/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-cache VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-cache STATIC
    src/idcu/cache/cache.c
)

target_include_directories(idcu-cache PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-cache PRIVATE
    idcu::common
)

add_library(idcu::cache ALIAS idcu-cache)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-cache/module.yaml`：

```yaml
name: idcu-cache
version: 1.0.0
description: In-memory cache library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common

build:
  type: cmake
  targets:
    - idcu-cache

headers:
  - idcu/cache/cache.h

features:
  - lru: LRU cache policy
  - lfu: LFU cache policy
  - fifo: FIFO cache policy
  - random: Random cache policy
  - ttl: TTL (time-to-live) support
  - capacity: Max capacity and memory limits
  - stats: Hit/miss statistics
  - thread_safe: Thread-safe operations

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `libs/idcu-cache/README.md`：

```markdown
# idcu-cache

IDCU Agent 的内存缓存库。

## 功能特性

- **LRU 策略**: LRU（最近最少使用）缓存策略
- **LFU 策略**: LFU（最不经常使用）缓存策略
- **FIFO 策略**: FIFO（先进先出）缓存策略
- **随机策略**: 随机淘汰策略
- **TTL 支持**: TTL（过期时间）支持
- **容量限制**: 最大容量和内存限制
- **统计信息**: 命中率统计
- **线程安全**: 线程安全操作

## 快速开始

### 初始化缓存

```c
#include "idcu/cache/cache.h"

idcu_CacheConfig config;
idcu_cache_config_init(&config);

config.policy = IDCU_CACHE_POLICY_LRU;
config.max_entries = 1000;
config.max_memory = 10 * 1024 * 1024;
config.default_ttl_ms = 3600000;

idcu_Cache cache;
idcu_cache_init(&cache, &config);
```

### 添加缓存项

```c
idcu_cache_put_string(&cache, "user:1", "John Doe");
idcu_cache_put_int(&cache, "count", 42);
idcu_cache_put_double(&cache, "score", 95.5);

const char* data = "cached data";
idcu_cache_put(&cache, "data", data, strlen(data));

idcu_cache_put_with_ttl(&cache, "temp", "temporary", 10, 60000);
```

### 获取缓存项

```c
char name[256];
idcu_cache_get_string(&cache, "user:1", name, sizeof(name));

int64_t count;
idcu_cache_get_int(&cache, "count", &count);

double score;
idcu_cache_get_double(&cache, "score", &score);

char buffer[1024];
size_t size;
idcu_cache_get(&cache, "data", buffer, sizeof(buffer), &size);
```

### 删除缓存项

```c
idcu_cache_remove(&cache, "user:1");
idcu_cache_clear(&cache);
```

### 检查是否存在

```c
if (idcu_cache_exists(&cache, "user:1")) {
    printf("Cache hit!\n");
} else {
    printf("Cache miss!\n");
}
```

### 获取统计信息

```c
idcu_CacheStats stats;
idcu_cache_get_stats(&cache, &stats);

printf("Hits: %" PRIu64 "\n", stats.hit_count);
printf("Misses: %" PRIu64 "\n", stats.miss_count);
printf("Hit rate: %.2f%%\n", stats.hit_rate * 100);
printf("Evictions: %" PRIu64 "\n", stats.evict_count);
```

### 清理过期项

```c
idcu_cache_cleanup_expired(&cache);
```

### 销毁缓存

```c
idcu_cache_destroy(&cache);
```

## 缓存策略

| 策略 | 说明 |
|-----|------|
| LRU | 最近最少使用 |
| LFU | 最不经常使用 |
| FIFO | 先进先出 |
| RANDOM | 随机淘汰 |

## API 文档

详见 [include/idcu/cache/cache.h](include/idcu/cache/cache.h)
```

## 验证检查清单

- [ ] 缓存头文件已创建
- [ ] 缓存实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 缓存可以正常读写
- [ ] LRU 淘汰策略正常工作
- [ ] 统计功能正常工作

## Git 提交

```bash
git add libs/idcu-cache/
git commit -m "feat: add idcu-cache library

- Add LRU cache policy
- Add LFU cache policy
- Add FIFO cache policy
- Add Random cache policy
- Add TTL (time-to-live) support
- Add max capacity and memory limits
- Add hit/miss statistics
- Add thread-safe operations
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 缓存频繁失效 | TTL 设置太短 | 增加 TTL 时间 |
| 内存占用过高 | max_memory 设置太大 | 减少 max_memory 或 max_entries |
| 命中率低 | 缓存策略不适合 | 尝试不同的缓存策略 |
