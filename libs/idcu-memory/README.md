# idcu-memory

IDCU 项目的独立内存池库，提供高效的固定大小内存分配功能。

## 特性

- 跨平台支持（Windows、Linux、macOS）
- 多粒度内存块分配
- 线程安全操作
- 内存使用统计
- 峰值使用追踪

## 快速开始

### 构建

```bash
mkdir build &amp;&amp; cd build
cmake ..
cmake --build .
```

### 安装

```bash
cmake --install .
```

### 在其他项目中使用

```cmake
find_package(idcu-common REQUIRED)
find_package(idcu-memory REQUIRED)
add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE idcu::memory)
```

## API 文档

### 初始化内存池

```c
#include &lt;idcu/memory/memory_pool.h&gt;

idcu_MemoryPool pool;
int ret = idcu_mem_pool_init(&amp;pool);
if (ret == IDCU_ERR_SUCCESS) {
    // 使用内存池
    idcu_mem_pool_destroy(&amp;pool);
}
```

### 分配和释放内存

```c
// 分配内存
void* ptr = idcu_mem_pool_alloc(&amp;pool, 64);
if (ptr) {
    // 使用内存
    idcu_mem_pool_free(&amp;pool, ptr);
}
```

### 获取统计信息

```c
// 获取当前空闲块数量
uint32_t free_count = idcu_mem_pool_get_free_count(&amp;pool, 64);

// 获取总分配字节数
uint64_t total_allocated = idcu_mem_pool_get_total_allocated(&amp;pool);

// 获取峰值使用字节数
uint64_t peak_usage = idcu_mem_pool_get_peak_usage(&amp;pool);
```

## 测试

```bash
cd build
ctest
```

## 示例

库提供了以下示例代码：

- [简单内存池示例](examples/example_basic.c) - 演示内存池基本使用
- [统计信息示例](examples/example_stats.c) - 演示如何获取使用统计

### 编译和运行示例

```bash
cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build .

./example_basic
./example_stats
```

## 依赖

- idcu-common - IDCU 基础通用组件库

## 许可证

详见项目根目录的 LICENSE 文件。
