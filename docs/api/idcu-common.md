# idcu-common API 参考文档

## 目录

1. [概述](#概述)
2. [原子操作 (atomic.h)](#原子操作-atomich)
3. [锁 (lock.h)](#锁-lockh)
4. [错误码 (error_code.h)](#错误码-error_codeh)
5. [配置 (config.h)](#配置-configh)
6. [向量 (vector.h)](#向量-vectorh)
7. [链表 (linked_list.h)](#链表-linked_listh)
8. [哈希表 (hash_map.h)](#哈希表-hash_maph)
9. [字符串缓冲区 (string_buf.h)](#字符串缓冲区-string_bufh)
10. [选项 (option.h)](#选项-optionh)

---

## 概述

`idcu-common` 是 IDCU 项目的基础通用组件库，提供跨平台的原子操作、锁、错误码等核心功能。

### 头文件引入

```c
#include <idcu/common/atomic.h>
#include <idcu/common/lock.h>
#include <idcu/common/error_code.h>
#include <idcu/common/config.h>
#include <idcu/common/vector.h>
#include <idcu/common/linked_list.h>
#include <idcu/common/hash_map.h>
#include <idcu/common/string_buf.h>
#include <idcu/common/option.h>
```

---

## 原子操作 (atomic.h)

提供跨平台的原子操作支持，支持 int32_t 和 int64_t 类型。

### 函数列表

#### int32_t 操作

| 函数 | 说明 |
|------|------|
| `idcu_atomic_fetch_add_int32` | 原子加法并返回原值 |
| `idcu_atomic_fetch_sub_int32` | 原子减法并返回原值 |
| `idcu_atomic_fetch_or_int32` | 原子按位或并返回原值 |
| `idcu_atomic_fetch_and_int32` | 原子按位与并返回原值 |
| `idcu_atomic_fetch_xor_int32` | 原子按位异或并返回原值 |
| `idcu_atomic_exchange_int32` | 原子交换值并返回原值 |
| `idcu_atomic_compare_exchange_int32` | 原子比较并交换 |
| `idcu_atomic_load_int32` | 原子加载值 |
| `idcu_atomic_store_int32` | 原子存储值 |
| `idcu_atomic_inc_int32` | 原子递增并返回原值 |
| `idcu_atomic_dec_int32` | 原子递减并返回原值 |

#### int64_t 操作

| 函数 | 说明 |
|------|------|
| `idcu_atomic_fetch_add_int64` | 原子加法并返回原值 |
| `idcu_atomic_fetch_sub_int64` | 原子减法并返回原值 |
| `idcu_atomic_exchange_int64` | 原子交换值并返回原值 |
| `idcu_atomic_compare_exchange_int64` | 原子比较并交换 |
| `idcu_atomic_load_int64` | 原子加载值 |
| `idcu_atomic_store_int64` | 原子存储值 |

### 函数详情

#### idcu_atomic_fetch_add_int32

```c
int32_t idcu_atomic_fetch_add_int32(volatile int32_t* ptr, int32_t value);
```

**参数**:
- `ptr`: 指向要操作的原子变量的指针
- `value`: 要添加的值

**返回值**:
- 操作前的原值

---

## 锁 (lock.h)

提供跨平台的互斥锁和读写锁支持。

### 数据结构

#### idcu_Mutex

互斥锁结构。

```c
typedef struct {
#ifdef _WIN32
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t mutex;
#endif
    int initialized;
} idcu_Mutex;
```

#### idcu_RwLock

读写锁结构。

```c
typedef struct {
#ifdef _WIN32
    SRWLOCK rwlock;
#else
    pthread_rwlock_t rwlock;
#endif
    int initialized;
} idcu_RwLock;
```

### 互斥锁函数

| 函数 | 说明 |
|------|------|
| `idcu_mutex_init` | 初始化互斥锁 |
| `idcu_mutex_destroy` | 销毁互斥锁 |
| `idcu_mutex_lock` | 加锁 |
| `idcu_mutex_unlock` | 解锁 |

### 读写锁函数

| 函数 | 说明 |
|------|------|
| `idcu_rwlock_init` | 初始化读写锁 |
| `idcu_rwlock_destroy` | 销毁读写锁 |
| `idcu_rwlock_rdlock` | 读锁 |
| `idcu_rwlock_wrlock` | 写锁 |
| `idcu_rwlock_unlock` | 解锁 |

### 函数详情

#### idcu_mutex_init

```c
int idcu_mutex_init(idcu_Mutex* lock);
```

**参数**:
- `lock`: 指向互斥锁结构的指针

**返回值**:
- `IDCU_ERR_OK`: 成功
- 其他错误码: 失败

---

## 错误码 (error_code.h)

定义了 IDCU 系统的统一错误码。

### 错误码枚举

#### 成功状态码

| 常量 | 值 | 说明 |
|------|-----|------|
| `IDCU_ERR_OK` | 0 | 成功 |
| `IDCU_ERR_SUCCESS` | 0 | 成功（别名） |

#### 通用错误类 (-1 ~ -99)

| 常量 | 值 | 说明 |
|------|-----|------|
| `IDCU_ERR_GENERAL` | -1 | 通用错误 |
| `IDCU_ERR_INVALID_PARAM` | -2 | 参数无效 |
| `IDCU_ERR_NO_MEMORY` | -3 | 内存不足 |
| `IDCU_ERR_PERM_DENIED` | -4 | 权限拒绝 |
| `IDCU_ERR_QUEUE_FULL` | -5 | 队列满 |
| `IDCU_ERR_QUEUE_EMPTY` | -6 | 队列空 |
| `IDCU_ERR_MODULE_NOT_FOUND` | -7 | 模块未找到 |
| `IDCU_ERR_MODULE_INIT` | -8 | 模块初始化失败 |
| `IDCU_ERR_CONFIG_LOAD` | -9 | 配置加载失败 |
| `IDCU_ERR_MSG_SEND` | -10 | 消息发送失败 |
| `IDCU_ERR_MSG_RECV` | -11 | 消息接收失败 |
| `IDCU_ERR_CORO_CREATE` | -12 | 协程创建失败 |
| `IDCU_ERR_SANDBOX_INIT` | -13 | 沙箱初始化失败 |
| `IDCU_ERR_MODULE_LOAD` | -14 | 模块加载失败 |
| `IDCU_ERR_MODULE_INVALID` | -15 | 模块无效 |
| `IDCU_ERR_NOT_FOUND` | -16 | 未找到 |
| `IDCU_ERR_ALREADY_EXISTS` | -17 | 已存在 |
| `IDCU_ERR_BUSY` | -18 | 忙 |
| `IDCU_ERR_MODULE_RUN` | -19 | 模块运行失败 |
| `IDCU_ERR_MODULE_STOP` | -20 | 模块停止失败 |
| `IDCU_ERR_MODULE_CRASH` | -21 | 模块崩溃 |
| `IDCU_ERR_DEPENDENCY` | -22 | 依赖错误 |
| `IDCU_ERR_CIRCULAR_DEP` | -23 | 循环依赖 |
| `IDCU_ERR_VERSION_MISMATCH` | -24 | 版本不匹配 |
| `IDCU_ERR_NOT_INITIALIZED` | -25 | 未初始化 |
| `IDCU_ERR_LOCK_FAILED` | -26 | 加锁失败 |
| `IDCU_ERR_UNLOCK_FAILED` | -27 | 解锁失败 |
| `IDCU_ERR_MSG_TIMEOUT` | -28 | 消息超时 |
| `IDCU_ERR_MSG_DROPPED` | -29 | 消息丢弃 |
| `IDCU_ERR_HEALTH_CHECK_FAIL` | -30 | 健康检查失败 |
| `IDCU_ERR_CONFIG_PARSE` | -31 | 配置解析错误 |
| `IDCU_ERR_TIMEOUT` | -32 | 超时 |
| `IDCU_ERR_CANCELLED` | -33 | 已取消 |
| `IDCU_ERR_CONFIG_INVALID` | -34 | 配置无效 |
| `IDCU_ERR_DEADLOCK` | -35 | 死锁检测 |
| `IDCU_ERR_SANDBOX_ACCESS` | -36 | 沙箱访问拒绝 |
| `IDCU_ERR_SANDBOX_VIOLATION` | -37 | 沙箱违规 |
| `IDCU_ERR_NETWORK_INIT` | -38 | 网络初始化失败 |
| `IDCU_ERR_NETWORK_CONNECT` | -39 | 网络连接失败 |
| `IDCU_ERR_NETWORK_SEND` | -40 | 网络发送失败 |
| `IDCU_ERR_NETWORK_RECV` | -41 | 网络接收失败 |
| `IDCU_ERR_NETWORK_TIMEOUT` | -42 | 网络超时 |
| `IDCU_ERR_METRICS_COLLECT` | -43 | 指标收集失败 |
| `IDCU_ERR_FILE_OPEN` | -44 | 文件打开失败 |
| `IDCU_ERR_FILE_READ` | -45 | 文件读取失败 |
| `IDCU_ERR_FILE_WRITE` | -46 | 文件写入失败 |
| `IDCU_ERR_FILE_DELETE` | -47 | 文件删除失败 |
| `IDCU_ERR_CORO_SCHEDULE` | -48 | 协程调度失败 |
| `IDCU_ERR_CORO_YIELD` | -49 | 协程让出失败 |
| `IDCU_ERR_BUFFER_TOO_SMALL` | -50 | 缓冲区太小 |
| `IDCU_ERR_VERSION_TOO_OLD` | -51 | 版本过旧 |
| `IDCU_ERR_VERSION_TOO_NEW` | -52 | 版本过新 |
| `IDCU_ERR_VERSION_INCOMPATIBLE` | -53 | 版本不兼容 |
| `IDCU_ERR_MEMORY` | -54 | 内存分配失败 |

### 函数列表

| 函数 | 说明 |
|------|------|
| `idcu_err_to_str` | 将错误码转换为字符串 |
| `idcu_err_is_ok` | 检查错误码是否表示成功 |

### 函数详情

#### idcu_err_to_str

```c
const char* idcu_err_to_str(int err_code);
```

**参数**:
- `err_code`: 错误码

**返回值**:
- 错误描述字符串

#### idcu_err_is_ok

```c
static inline int idcu_err_is_ok(int err_code);
```

**参数**:
- `err_code`: 错误码

**返回值**:
- 1: 成功
- 0: 失败

---

## 配置 (config.h)

提供系统配置常量定义。

### 常量定义

| 常量 | 值 | 说明 |
|------|-----|------|
| `IDCU_CONFIG_MAX_MODULES` | 32 | 最大模块数 |
| `IDCU_CONFIG_MAX_MSG` | 64 | 最大消息数 |
| `IDCU_CONFIG_STACK_CTX_SIZE` | 256 | 栈上下文大小 |
| `IDCU_CONFIG_LOG_LEVEL` | 1 | 日志级别 |
| `IDCU_AGENT_VERSION_STR` | "1.0.0" | Agent 版本字符串 |
| `IDCU_CONFIG_FILE_PATH` | "config/agent.cfg" | 配置文件路径 |
| `IDCU_CONFIG_SECTION_MAX` | 64 | 配置节最大数量 |
| `IDCU_CONFIG_PATH_MAX` | 512 | 路径最大长度 |

### 权限定义

| 常量 | 说明 |
|------|------|
| `IDCU_PERM_SEND` | 发送权限 |
| `IDCU_PERM_RECV` | 接收权限 |
| `IDCU_PERM_RUN` | 运行权限 |
| `IDCU_PERM_HW` | 硬件权限 |
| `IDCU_PERM_CONFIG` | 配置权限 |
| `IDCU_PERM_LOG` | 日志权限 |
| `IDCU_PERM_DEBUG` | 调试权限 |
| `IDCU_PERM_MODULE_MGR` | 模块管理权限 |

---

## 向量 (vector.h)

提供动态数组（向量）容器。

### 数据结构

```c
typedef struct {
    void** data;
    size_t size;
    size_t capacity;
    size_t element_size;
    void (*element_dtor)(void*);
} idcu_Vector;
```

### 函数列表

| 函数 | 说明 |
|------|------|
| `idcu_vector_init` | 初始化向量 |
| `idcu_vector_init_with_dtor` | 初始化向量（带析构函数） |
| `idcu_vector_destroy` | 销毁向量 |
| `idcu_vector_push_back` | 在末尾添加元素 |
| `idcu_vector_pop_back` | 移除末尾元素 |
| `idcu_vector_insert` | 在指定位置插入元素 |
| `idcu_vector_remove` | 移除指定位置元素 |
| `idcu_vector_get` | 获取指定位置元素 |
| `idcu_vector_set` | 设置指定位置元素 |
| `idcu_vector_size` | 获取向量大小 |
| `idcu_vector_capacity` | 获取向量容量 |
| `idcu_vector_empty` | 检查是否为空 |
| `idcu_vector_clear` | 清空向量 |
| `idcu_vector_reserve` | 预留容量 |
| `idcu_vector_resize` | 调整大小 |

### 宏定义

| 宏 | 说明 |
|----|------|
| `IDCU_VECTOR_FOR_EACH` | 遍历向量 |

---

## 链表 (linked_list.h)

提供双向链表容器。

### 数据结构

```c
typedef struct idcu_ListNode {
    void* data;
    idcu_ListNode* prev;
    idcu_ListNode* next;
} idcu_ListNode;

typedef struct {
    idcu_ListNode* head;
    idcu_ListNode* tail;
    size_t size;
    size_t element_size;
    void (*element_dtor)(void*);
} idcu_LinkedList;

typedef struct {
    idcu_LinkedList* list;
    idcu_ListNode* current;
    int direction;
} idcu_LinkedListIterator;
```

### 函数列表

| 函数 | 说明 |
|------|------|
| `idcu_linked_list_init` | 初始化链表 |
| `idcu_linked_list_init_with_dtor` | 初始化链表（带析构函数） |
| `idcu_linked_list_destroy` | 销毁链表 |
| `idcu_linked_list_push_front` | 在头部添加元素 |
| `idcu_linked_list_push_back` | 在尾部添加元素 |
| `idcu_linked_list_pop_front` | 移除头部元素 |
| `idcu_linked_list_pop_back` | 移除尾部元素 |
| `idcu_linked_list_insert_before` | 在指定位置前插入 |
| `idcu_linked_list_insert_after` | 在指定位置后插入 |
| `idcu_linked_list_remove` | 移除指定位置元素 |
| `idcu_linked_list_get` | 获取指定位置元素 |
| `idcu_linked_list_front` | 获取头部元素 |
| `idcu_linked_list_back` | 获取尾部元素 |
| `idcu_linked_list_size` | 获取链表大小 |
| `idcu_linked_list_empty` | 检查是否为空 |
| `idcu_linked_list_clear` | 清空链表 |
| `idcu_linked_list_iter_init` | 初始化迭代器 |
| `idcu_linked_list_iter_init_reverse` | 初始化反向迭代器 |
| `idcu_linked_list_iter_next` | 获取下一个元素 |
| `idcu_linked_list_iter_destroy` | 销毁迭代器 |

### 宏定义

| 宏 | 说明 |
|----|------|
| `IDCU_LINKED_LIST_FOR_EACH` | 正向遍历链表 |
| `IDCU_LINKED_LIST_FOR_EACH_REVERSE` | 反向遍历链表 |

---

## 哈希表 (hash_map.h)

提供哈希表容器。

### 数据结构

```c
typedef struct {
    // 内部实现细节
} idcu_HashMap;
```

### 函数列表

| 函数 | 说明 |
|------|------|
| `idcu_hash_map_init` | 初始化哈希表 |
| `idcu_hash_map_destroy` | 销毁哈希表 |
| `idcu_hash_map_insert` | 插入键值对 |
| `idcu_hash_map_get` | 获取值 |
| `idcu_hash_map_remove` | 移除键值对 |
| `idcu_hash_map_contains` | 检查键是否存在 |
| `idcu_hash_map_size` | 获取哈希表大小 |
| `idcu_hash_map_empty` | 检查是否为空 |
| `idcu_hash_map_clear` | 清空哈希表 |

---

## 字符串缓冲区 (string_buf.h)

提供动态字符串缓冲区。

### 数据结构

```c
typedef struct {
    char* data;
    size_t size;
    size_t capacity;
} idcu_StringBuf;
```

### 函数列表

| 函数 | 说明 |
|------|------|
| `idcu_string_buf_init` | 初始化字符串缓冲区 |
| `idcu_string_buf_destroy` | 销毁字符串缓冲区 |
| `idcu_string_buf_append` | 追加字符串 |
| `idcu_string_buf_append_char` | 追加字符 |
| `idcu_string_buf_append_format` | 格式化追加 |
| `idcu_string_buf_clear` | 清空缓冲区 |
| `idcu_string_buf_c_str` | 获取 C 字符串 |
| `idcu_string_buf_size` | 获取大小 |

---

## 选项 (option.h)

提供可选值类型（类似 Rust 的 Option）。

### 数据结构

```c
typedef struct {
    int has_value;
    void* value;
    size_t value_size;
} idcu_Option;
```

### 函数列表

| 函数 | 说明 |
|------|------|
| `idcu_option_none` | 创建无值选项 |
| `idcu_option_some` | 创建有值选项 |
| `idcu_option_has_value` | 检查是否有值 |
| `idcu_option_get` | 获取值 |
| `idcu_option_get_or` | 获取值或默认值 |
| `idcu_option_destroy` | 销毁选项 |

---

## 许可证

详见项目根目录的 LICENSE 文件。
