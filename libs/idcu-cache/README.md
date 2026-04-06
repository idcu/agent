# idcu-cache

IDCU 项目的缓存服务库，提供 LRU、FIFO、LFU 缓存策略。

## 特性

- 多种缓存策略（LRU、FIFO、LFU）
- TTL（生存时间）支持
- 字符串、整数和二进制数据缓存
- 线程安全
- 缓存统计（命中/未命中）
- 跨平台支持（Windows、Linux、macOS）

## 快速开始

### 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 安装

```bash
cmake --install .
```

### 在其他项目中使用

```cmake
find_package(idcu-cache REQUIRED)
add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE idcu::cache)
```

## API 文档

### 缓存初始化和销毁

```c
#include <idcu/cache/cache.h>

int idcu_cache_init(idcu_Cache** cache, size_t max_entries, idcu_CachePolicy policy);
void idcu_cache_destroy(idcu_Cache* cache);
```

### 基本缓存操作

```c
int idcu_cache_put(idcu_Cache* cache, const char* key, const void* value, size_t size, uint64_t ttl_ms);
int idcu_cache_get(idcu_Cache* cache, const char* key, void* buffer, size_t* buffer_size);
int idcu_cache_remove(idcu_Cache* cache, const char* key);
int idcu_cache_clear(idcu_Cache* cache);
```

### 缓存查询

```c
int idcu_cache_contains(idcu_Cache* cache, const char* key);
size_t idcu_cache_size(idcu_Cache* cache);
size_t idcu_cache_count(idcu_Cache* cache);
```

### 便捷方法

```c
int idcu_cache_put_string(idcu_Cache* cache, const char* key, const char* value, uint64_t ttl_ms);
int idcu_cache_get_string(idcu_Cache* cache, const char* key, char* buffer, size_t buffer_size);
int idcu_cache_put_int(idcu_Cache* cache, const char* key, int64_t value, uint64_t ttl_ms);
int idcu_cache_get_int(idcu_Cache* cache, const char* key, int64_t* value);
```

## 测试

```bash
cd build
ctest
```

## 许可证

详见项目根目录的 LICENSE 文件。
