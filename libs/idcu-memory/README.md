# idcu-memory

内存池管理库 for IDCU Agent.

## 功能特性

- 多尺寸类别内存池（16、32、64、128、256、512、1024、2048 字节）
- 快速分配和释放（延迟 ≤ 100ns）
- 线程安全操作（每个尺寸类别独立锁）
- 内存使用统计（累计分配、峰值使用、空闲块数）
- 安全检查（空指针、缓冲区溢出）
- 内存保护标记（用于检测损坏）

## 使用方法

```c
#include <idcu/memory/memory.h>

// 初始化内存池
idcu_MemoryPool pool;
int ret = idcu_mem_pool_init(&pool);
if (ret != IDCU_ERR_OK) {
    // 处理错误
}

// 分配内存
void* ptr = idcu_mem_pool_alloc(&pool, 64);
if (ptr) {
    // 使用内存
    strcpy((char*)ptr, "Hello World!");
    
    // 释放内存
    idcu_mem_pool_free(&pool, ptr);
}

// 销毁内存池
idcu_mem_pool_destroy(&pool);
```

## API 参考

### 核心功能
- `idcu_mem_pool_init()` - 初始化内存池
- `idcu_mem_pool_destroy()` - 销毁内存池
- `idcu_mem_pool_alloc()` - 分配内存
- `idcu_mem_pool_free()` - 释放内存

### 统计功能
- `idcu_mem_pool_get_free_count()` - 获取空闲块数
- `idcu_mem_pool_get_total_allocated()` - 获取累计分配字节数
- `idcu_mem_pool_get_peak_usage()` - 获取峰值使用字节数

### 安全检查
- `idcu_mem_check_null()` - 检查空指针
- `idcu_mem_check_overflow()` - 检查缓冲区溢出
- `idcu_mem_safe_copy()` - 安全内存拷贝
- `idcu_mem_pool_get_safety_stats()` - 获取安全统计

## 构建

```bash
cmake -B build && cmake --build build
```

## 测试

```bash
cd build
ctest --verbose
```

## 性能指标

- 单次分配延迟：≤ 100ns
- 单次释放延迟：≤ 100ns
- 内存碎片率：≤ 5%
- 多线程吞吐量：≥ 1,000,000 次/秒

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
