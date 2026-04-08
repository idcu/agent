# idcu-common

IDCU 项目的基础通用组件库，提供跨平台的原子操作、锁、数据结构和错误码等核心功能。

## 特性

- 跨平台支持（Windows、Linux、macOS）
- 原子操作（Atomic）
- 互斥锁和读写锁（Mutex、RWLock）
- 统一的错误码定义
- 动态数组（Vector）
- 双向链表（LinkedList）
- 哈希表（HashMap）
- 字符串缓冲区（StringBuf）
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

### 动态数组（Vector）

```c
#include <idcu/common/vector.h>

// 初始化向量
int idcu_vector_init(idcu_Vector* vec, size_t element_size, size_t initial_capacity);
int idcu_vector_init_with_dtor(idcu_Vector* vec, size_t element_size, size_t initial_capacity, 
                                void (*element_dtor)(void*));
void idcu_vector_destroy(idcu_Vector* vec);

// 添加和删除元素
int idcu_vector_push_back(idcu_Vector* vec, const void* element);
int idcu_vector_pop_back(idcu_Vector* vec, void* out_element);
int idcu_vector_insert(idcu_Vector* vec, size_t index, const void* element);
int idcu_vector_remove(idcu_Vector* vec, size_t index);

// 访问元素
void* idcu_vector_get(const idcu_Vector* vec, size_t index);
int idcu_vector_set(idcu_Vector* vec, size_t index, const void* element);

// 容量管理
size_t idcu_vector_size(const idcu_Vector* vec);
size_t idcu_vector_capacity(const idcu_Vector* vec);
bool idcu_vector_empty(const idcu_Vector* vec);
void idcu_vector_clear(idcu_Vector* vec);
int idcu_vector_reserve(idcu_Vector* vec, size_t new_capacity);
int idcu_vector_resize(idcu_Vector* vec, size_t new_size, const void* default_value);

// 遍历宏
IDCU_VECTOR_FOR_EACH(vec, type, var, index)
```

### 双向链表（LinkedList）

```c
#include <idcu/common/linked_list.h>

// 初始化链表
int idcu_linked_list_init(idcu_LinkedList* list, size_t element_size);
int idcu_linked_list_init_with_dtor(idcu_LinkedList* list, size_t element_size,
                                       void (*element_dtor)(void*));
void idcu_linked_list_destroy(idcu_LinkedList* list);

// 添加和删除元素
int idcu_linked_list_push_front(idcu_LinkedList* list, const void* element);
int idcu_linked_list_push_back(idcu_LinkedList* list, const void* element);
int idcu_linked_list_pop_front(idcu_LinkedList* list, void* out_element);
int idcu_linked_list_pop_back(idcu_LinkedList* list, void* out_element);
int idcu_linked_list_insert_before(idcu_LinkedList* list, size_t index, const void* element);
int idcu_linked_list_insert_after(idcu_LinkedList* list, size_t index, const void* element);
int idcu_linked_list_remove(idcu_LinkedList* list, size_t index);

// 访问元素
void* idcu_linked_list_get(const idcu_LinkedList* list, size_t index);
void* idcu_linked_list_front(const idcu_LinkedList* list);
void* idcu_linked_list_back(const idcu_LinkedList* list);

// 容量和状态
size_t idcu_linked_list_size(const idcu_LinkedList* list);
bool idcu_linked_list_empty(const idcu_LinkedList* list);
void idcu_linked_list_clear(idcu_LinkedList* list);

// 迭代器
void idcu_linked_list_iter_init(idcu_LinkedListIterator* iter, idcu_LinkedList* list);
void idcu_linked_list_iter_init_reverse(idcu_LinkedListIterator* iter, idcu_LinkedList* list);
bool idcu_linked_list_iter_next(idcu_LinkedListIterator* iter, void** out_element);
void idcu_linked_list_iter_destroy(idcu_LinkedListIterator* iter);

// 遍历宏
IDCU_LINKED_LIST_FOR_EACH(list, type, var)
IDCU_LINKED_LIST_FOR_EACH_REVERSE(list, type, var)
```

### 哈希表（HashMap）

```c
#include <idcu/common/hash_map.h>

// 初始化哈希表
int idcu_hash_map_init(idcu_HashMap* map, size_t value_size, size_t initial_buckets);
int idcu_hash_map_init_with_dtor(idcu_HashMap* map, size_t value_size, size_t initial_buckets,
                                   void (*value_dtor)(void*));
void idcu_hash_map_destroy(idcu_HashMap* map);

// 插入、查询和删除
int idcu_hash_map_insert(idcu_HashMap* map, const char* key, const void* value);
int idcu_hash_map_get(const idcu_HashMap* map, const char* key, void* out_value);
bool idcu_hash_map_contains(const idcu_HashMap* map, const char* key);
int idcu_hash_map_remove(idcu_HashMap* map, const char* key);
void idcu_hash_map_clear(idcu_HashMap* map);

// 容量和状态
size_t idcu_hash_map_size(const idcu_HashMap* map);
bool idcu_hash_map_empty(const idcu_HashMap* map);

// 迭代器
void idcu_hash_map_iter_init(idcu_HashMapIterator* iter, idcu_HashMap* map);
bool idcu_hash_map_iter_next(idcu_HashMapIterator* iter, const char** out_key, void** out_value);
void idcu_hash_map_iter_destroy(idcu_HashMapIterator* iter);

// 遍历宏
IDCU_HASH_MAP_FOR_EACH(map, key_var, value_var, type)
```

### 字符串缓冲区（StringBuf）

```c
#include <idcu/common/string_buf.h>

// 初始化和销毁
int idcu_strbuf_init(idcu_StringBuf* buf, size_t initial_capacity);
void idcu_strbuf_destroy(idcu_StringBuf* buf);

// 追加操作
int idcu_strbuf_append(idcu_StringBuf* buf, const char* str);
int idcu_strbuf_append_n(idcu_StringBuf* buf, const char* str, size_t n);
int idcu_strbuf_append_char(idcu_StringBuf* buf, char c);
int idcu_strbuf_append_int(idcu_StringBuf* buf, int64_t value);
int idcu_strbuf_append_double(idcu_StringBuf* buf, double value, int precision);
int idcu_strbuf_append_format(idcu_StringBuf* buf, const char* format, ...);
int idcu_strbuf_append_format_v(idcu_StringBuf* buf, const char* format, va_list args);

// 插入和删除
int idcu_strbuf_insert(idcu_StringBuf* buf, size_t pos, const char* str);
int idcu_strbuf_remove(idcu_StringBuf* buf, size_t pos, size_t len);

// 容量管理
int idcu_strbuf_clear(idcu_StringBuf* buf);
int idcu_strbuf_reserve(idcu_StringBuf* buf, size_t new_capacity);
int idcu_strbuf_resize(idcu_StringBuf* buf, size_t new_size, char fill_char);

// 访问和状态
const char* idcu_strbuf_data(const idcu_StringBuf* buf);
char* idcu_strbuf_detach(idcu_StringBuf* buf);
size_t idcu_strbuf_size(const idcu_StringBuf* buf);
size_t idcu_strbuf_capacity(const idcu_StringBuf* buf);
int idcu_strbuf_empty(const idcu_StringBuf* buf);

// 字符串操作
int idcu_strbuf_compare(const idcu_StringBuf* buf, const char* str);
int idcu_strbuf_find(const idcu_StringBuf* buf, const char* substr, size_t start_pos);
int idcu_strbuf_replace(idcu_StringBuf* buf, const char* old_str, const char* new_str);
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
