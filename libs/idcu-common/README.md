# idcu-common

IDCU 项目的基础通用组件库，提供跨平台的原子操作、锁、错误码等核心功能。

## 特性

- 跨平台支持（Windows、Linux、macOS）
- 原子操作（Atomic）
- 互斥锁和读写锁（Mutex、RWLock）
- 统一的错误码定义
- 零依赖设计

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
find_package(idcu-common REQUIRED)
add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE idcu::common)
```

## API 文档

### 原子操作

```c
#include <idcu/common/atomic.h>

// int32_t 操作
int32_t idcu_atomic_fetch_add_int32(volatile int32_t* ptr, int32_t value);
int32_t idcu_atomic_fetch_sub_int32(volatile int32_t* ptr, int32_t value);
int32_t idcu_atomic_exchange_int32(volatile int32_t* ptr, int32_t value);
int idcu_atomic_compare_exchange_int32(volatile int32_t* ptr, int32_t* expected, int32_t desired);
int32_t idcu_atomic_load_int32(volatile int32_t* ptr);
void idcu_atomic_store_int32(volatile int32_t* ptr, int32_t value);
int32_t idcu_atomic_inc_int32(volatile int32_t* ptr);
int32_t idcu_atomic_dec_int32(volatile int32_t* ptr);

// int64_t 操作
int64_t idcu_atomic_fetch_add_int64(volatile int64_t* ptr, int64_t value);
int64_t idcu_atomic_fetch_sub_int64(volatile int64_t* ptr, int64_t value);
int64_t idcu_atomic_exchange_int64(volatile int64_t* ptr, int64_t value);
int idcu_atomic_compare_exchange_int64(volatile int64_t* ptr, int64_t* expected, int64_t desired);
int64_t idcu_atomic_load_int64(volatile int64_t* ptr);
void idcu_atomic_store_int64(volatile int64_t* ptr, int64_t value);
```

### 锁

```c
#include <idcu/common/lock.h>

// 互斥锁
int idcu_mutex_init(idcu_Mutex* lock);
void idcu_mutex_destroy(idcu_Mutex* lock);
int idcu_mutex_lock(idcu_Mutex* lock);
int idcu_mutex_unlock(idcu_Mutex* lock);

// 读写锁
int idcu_rwlock_init(idcu_RwLock* lock);
void idcu_rwlock_destroy(idcu_RwLock* lock);
int idcu_rwlock_rdlock(idcu_RwLock* lock);
int idcu_rwlock_wrlock(idcu_RwLock* lock);
int idcu_rwlock_unlock(idcu_RwLock* lock);
```

### 错误码

```c
#include <idcu/common/error_code.h>

const char* idcu_err_to_str(int err_code);
int idcu_err_is_ok(int err_code);
```

## 测试

```bash
cd build
ctest
```

## 示例

库提供了以下示例代码：

- [原子操作示例](examples/example_atomic.c) - 演示 int32_t 和 int64_t 的原子操作
- [锁使用示例](examples/example_lock.c) - 演示互斥锁和读写锁的使用

### 编译和运行示例

```bash
cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build .

# 运行原子操作示例
./example_atomic

# 运行锁示例
./example_lock
```

## API 文档

详细的 API 文档请参考：[idcu-common API 文档](../../docs/api/idcu-common.md)

## 许可证

详见项目根目录的 LICENSE 文件。
