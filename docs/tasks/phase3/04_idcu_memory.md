# 任务 3.4: idcu-memory - 内存池管理库

## 目标

创建内存池管理库，支持：
- 固定大小内存块分配
- 内存池初始化和销毁
- 快速的分配和释放
- 内存使用统计
- 线程安全

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-memory/include/idcu/memory
mkdir -p libs/idcu-memory/src/idcu/memory
mkdir -p libs/idcu-memory/tests
mkdir -p libs/idcu-memory/examples
```

### 2. 创建内存池头文件 (memory_pool.h)

创建 `libs/idcu-memory/include/idcu/memory/memory_pool.h`：

```c
#ifndef IDCU_MEMORY_MEMORY_POOL_H
#define IDCU_MEMORY_MEMORY_POOL_H

#include "idcu/common/error_code.h"
#include "idcu/common/lock.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct idcu_MemoryBlock
{
    struct idcu_MemoryBlock* next;
    uint8_t in_use;
} idcu_MemoryBlock;

typedef struct
{
    uint8_t* memory;
    size_t block_size;
    size_t block_count;
    size_t total_size;
    idcu_MemoryBlock* free_list;
    idcu_Mutex lock;
    uint64_t alloc_count;
    uint64_t free_count;
    uint64_t peak_usage;
    uint64_t current_usage;
    int initialized;
} idcu_MemoryPool;

int  idcu_memory_pool_init(idcu_MemoryPool* pool, size_t block_size, size_t block_count);
void idcu_memory_pool_destroy(idcu_MemoryPool* pool);

void* idcu_memory_pool_alloc(idcu_MemoryPool* pool);
void  idcu_memory_pool_free(idcu_MemoryPool* pool, void* ptr);

void* idcu_memory_pool_calloc(idcu_MemoryPool* pool);
void* idcu_memory_pool_realloc(idcu_MemoryPool* pool, void* ptr);

int idcu_memory_pool_is_from(const idcu_MemoryPool* pool, const void* ptr);

size_t idcu_memory_pool_get_block_size(const idcu_MemoryPool* pool);
size_t idcu_memory_pool_get_block_count(const idcu_MemoryPool* pool);
size_t idcu_memory_pool_get_free_count(const idcu_MemoryPool* pool);
size_t idcu_memory_pool_get_used_count(const idcu_MemoryPool* pool);

uint64_t idcu_memory_pool_get_alloc_count(const idcu_MemoryPool* pool);
uint64_t idcu_memory_pool_get_free_count_total(const idcu_MemoryPool* pool);
uint64_t idcu_memory_pool_get_peak_usage(const idcu_MemoryPool* pool);
uint64_t idcu_memory_pool_get_current_usage(const idcu_MemoryPool* pool);

void idcu_memory_pool_reset_stats(idcu_MemoryPool* pool);

typedef struct
{
    size_t block_size;
    size_t block_count;
    size_t free_count;
    size_t used_count;
    uint64_t total_allocated;
    uint64_t total_freed;
    uint64_t peak_usage;
    uint64_t current_usage;
} idcu_MemoryPoolStats;

void idcu_memory_pool_get_stats(const idcu_MemoryPool* pool, idcu_MemoryPoolStats* stats);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建内存管理头文件 (memory.h)

创建 `libs/idcu-memory/include/idcu/memory/memory.h`：

```c
#ifndef IDCU_MEMORY_MEMORY_H
#define IDCU_MEMORY_MEMORY_H

#include "memory_pool.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* idcu_malloc(size_t size);
void* idcu_calloc(size_t count, size_t size);
void* idcu_realloc(void* ptr, size_t size);
void  idcu_free(void* ptr);

void* idcu_malloc_aligned(size_t size, size_t alignment);
void  idcu_free_aligned(void* ptr);

typedef struct
{
    uint64_t total_allocated;
    uint64_t total_freed;
    uint64_t current_usage;
    uint64_t peak_usage;
    uint64_t allocation_count;
} idcu_MemoryStats;

void idcu_memory_get_stats(idcu_MemoryStats* stats);
void idcu_memory_reset_stats(void);

#ifdef __cplusplus
}
#endif

#endif
```

### 4. 创建 CMakeLists.txt

创建 `libs/idcu-memory/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-memory VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-memory STATIC
    src/idcu/memory/memory_pool.c
    src/idcu/memory/memory.c
)

target_include_directories(idcu-memory PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-memory PRIVATE
    idcu::common
)

add_library(idcu::memory ALIAS idcu-memory)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 5. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-memory/module.yaml`：

```yaml
name: idcu-memory
version: 1.0.0
description: Memory pool management library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common

build:
  type: cmake
  targets:
    - idcu-memory

headers:
  - idcu/memory/memory_pool.h
  - idcu/memory/memory.h

features:
  - memory_pool: Fixed-size block memory pool
  - fast_alloc: Fast allocation and deallocation
  - thread_safe: Thread-safe operations
  - stats: Memory usage statistics
  - aligned_alloc: Aligned memory allocation

testing:
  enabled: true
  framework: internal
```

### 6. 创建 README.md

创建 `libs/idcu-memory/README.md`：

```markdown
# idcu-memory

IDCU Agent 的内存池管理库。

## 功能特性

- **内存池**: 固定大小内存块分配
- **快速分配**: 高效的内存分配和释放
- **线程安全**: 多线程环境安全使用
- **统计信息**: 内存使用统计
- **对齐分配**: 对齐内存分配支持

## 快速开始

### 使用内存池

```c
#include "idcu/memory/memory_pool.h"

idcu_MemoryPool pool;
idcu_memory_pool_init(&pool, 64, 1024);

void* ptr = idcu_memory_pool_alloc(&pool);
if (ptr) {
    memset(ptr, 0, 64);
    idcu_memory_pool_free(&pool, ptr);
}

idcu_MemoryPoolStats stats;
idcu_memory_pool_get_stats(&pool, &stats);
printf("Used: %zu, Free: %zu\n", stats.used_count, stats.free_count);

idcu_memory_pool_destroy(&pool);
```

### 使用标准分配器

```c
#include "idcu/memory/memory.h"

void* ptr = idcu_malloc(1024);
if (ptr) {
    idcu_free(ptr);
}
```

## API 文档

详见 [include/idcu/memory/](include/idcu/memory/)
```

## 验证检查清单

- [ ] 内存池头文件已创建
- [ ] 内存管理头文件已创建
- [ ] 实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 内存池可以正常分配和释放
- [ ] 统计功能正常工作

## Git 提交

```bash
git add libs/idcu-memory/
git commit -m "feat: add idcu-memory library

- Add memory pool with fixed-size blocks
- Add fast allocation and deallocation
- Add thread-safe operations
- Add memory usage statistics
- Add aligned memory allocation
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 内存耗尽 | 内存池太小 | 增加 block_count 参数 |
| 双倍释放 | 重复释放同一指针 | 确保每个指针只释放一次 |
| 野指针 | 使用已释放的内存 | 使用内存池统计追踪使用情况 |
