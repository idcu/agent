# 任务 2.1: 创建通用基础库 (idcu-common)

## 目标

创建完整的通用基础库，为所有模块提供统一的基础设施，包括：
- 统一错误码定义
- 通用数据结构（向量、链表、哈希表）
- 并发原语（锁、原子操作）
- 字符串缓冲区
- 配置常量
- 安全工具

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-common/include/idcu/common
mkdir -p libs/idcu-common/src/idcu/common
mkdir -p libs/idcu-common/tests
mkdir -p libs/idcu-common/examples
```

### 2. 创建错误码头文件 (error_code.h)

创建 `libs/idcu-common/include/idcu/common/error_code.h`：

```c
#ifndef IDCU_COMMON_ERROR_CODE_H
#define IDCU_COMMON_ERROR_CODE_H

#include <stddef.h>

typedef enum
{
    IDCU_ERR_OK      = 0,
    IDCU_ERR_SUCCESS = 0,

    IDCU_ERR_GENERAL           = -1,
    IDCU_ERR_INVALID_PARAM     = -2,
    IDCU_ERR_INVALID_ARG       = -2,
    IDCU_ERR_NO_MEMORY         = -3,
    IDCU_ERR_OUT_OF_MEM        = -3,
    IDCU_ERR_PERM_DENIED       = -4,
    IDCU_ERR_QUEUE_FULL        = -5,
    IDCU_ERR_QUEUE_EMPTY       = -6,
    IDCU_ERR_MODULE_NOT_FOUND  = -7,
    IDCU_ERR_MODULE_INIT       = -8,
    IDCU_ERR_CONFIG_LOAD       = -9,
    IDCU_ERR_MSG_SEND          = -10,
    IDCU_ERR_MSG_RECV          = -11,
    IDCU_ERR_CORO_CREATE       = -12,
    IDCU_ERR_SANDBOX_INIT      = -13,
    IDCU_ERR_MODULE_LOAD       = -14,
    IDCU_ERR_MODULE_INVALID    = -15,
    IDCU_ERR_NOT_FOUND         = -16,
    IDCU_ERR_ALREADY_EXISTS    = -17,
    IDCU_ERR_BUSY              = -18,
    IDCU_ERR_MODULE_RUN        = -19,
    IDCU_ERR_MODULE_STOP       = -20,
    IDCU_ERR_MODULE_CRASH      = -21,
    IDCU_ERR_DEPENDENCY        = -22,
    IDCU_ERR_CIRCULAR_DEP      = -23,
    IDCU_ERR_VERSION_MISMATCH  = -24,
    IDCU_ERR_NOT_INITIALIZED   = -25,
    IDCU_ERR_LOCK_FAILED       = -26,
    IDCU_ERR_UNLOCK_FAILED     = -27,
    IDCU_ERR_MSG_TIMEOUT       = -28,
    IDCU_ERR_MSG_DROPPED       = -29,
    IDCU_ERR_HEALTH_CHECK_FAIL = -30,
    IDCU_ERR_CONFIG_PARSE      = -31,
    IDCU_ERR_TIMEOUT           = -32,
    IDCU_ERR_CANCELLED         = -33,
    IDCU_ERR_CONFIG_INVALID    = -34,
    IDCU_ERR_DEADLOCK          = -35,
    IDCU_ERR_SANDBOX_ACCESS    = -36,
    IDCU_ERR_SANDBOX_VIOLATION = -37,
    IDCU_ERR_NETWORK_INIT      = -38,
    IDCU_ERR_NETWORK_CONNECT   = -39,
    IDCU_ERR_NETWORK_SEND      = -40,
    IDCU_ERR_NETWORK_RECV      = -41,
    IDCU_ERR_NETWORK_TIMEOUT   = -42,
    IDCU_ERR_METRICS_COLLECT   = -43,
    IDCU_ERR_FILE_OPEN         = -44,
    IDCU_ERR_FILE_READ         = -45,
    IDCU_ERR_FILE_WRITE        = -46,
    IDCU_ERR_FILE_DELETE       = -47,
    IDCU_ERR_CORO_SCHEDULE     = -48,
    IDCU_ERR_CORO_YIELD        = -49,
    IDCU_ERR_BUFFER_TOO_SMALL  = -50,
    IDCU_ERR_VERSION_TOO_OLD   = -51,
    IDCU_ERR_VERSION_TOO_NEW   = -52,
    IDCU_ERR_VERSION_INCOMPATIBLE = -53,
    IDCU_ERR_MEMORY            = -54,
    IDCU_ERR_LIMIT_EXCEEDED    = -55,
    IDCU_ERR_IO_ERROR          = -56,
    IDCU_ERR_FILE_ERROR        = -56,
    IDCU_ERR_SECURITY_ERROR    = -57,
    IDCU_ERR_RESOURCE_EXHAUSTED = -58,
    IDCU_ERR_VERIFICATION_FAILED = -59
} idcu_ErrorCode;

#define IDCU_SUCCESS                    IDCU_ERR_SUCCESS
#define IDCU_ERROR_INVALID_PARAM        IDCU_ERR_INVALID_PARAM
#define IDCU_ERROR_MEMORY               IDCU_ERR_MEMORY
#define IDCU_ERROR_BUFFER_TOO_SMALL     IDCU_ERR_BUFFER_TOO_SMALL
#define IDCU_ERROR_VERSION_TOO_OLD      IDCU_ERR_VERSION_TOO_OLD
#define IDCU_ERROR_VERSION_TOO_NEW      IDCU_ERR_VERSION_TOO_NEW
#define IDCU_ERROR_VERSION_INCOMPATIBLE IDCU_ERR_VERSION_INCOMPATIBLE

#define IDCU_ERROR_CONTEXT_MAX_LEN 256

typedef struct
{
    int         error_code;
    char        context[IDCU_ERROR_CONTEXT_MAX_LEN];
    const char* file;
    int         line;
} idcu_ErrorInfo;

const char* idcu_err_to_str(int err_code);

static inline int idcu_err_is_ok(int err_code)
{
    return err_code == IDCU_ERR_OK;
}

void idcu_err_set_last_error(int err_code, const char* context, const char* file, int line);
const idcu_ErrorInfo* idcu_err_get_last_error(void);
void idcu_err_clear_last_error(void);

#define IDCU_ERR_SET(code, ctx) idcu_err_set_last_error((code), (ctx), __FILE__, __LINE__)

#endif
```

### 3. 创建错误码实现文件 (error_code.c)

创建 `libs/idcu-common/src/idcu/common/error_code.c`：

```c
#include "idcu/common/error_code.h"
#include <string.h>

static idcu_ErrorInfo g_last_error = {0};

const char* idcu_err_to_str(int err_code)
{
    switch (err_code) {
    case IDCU_ERR_OK: return "Success";
    case IDCU_ERR_GENERAL: return "General error";
    case IDCU_ERR_INVALID_PARAM: return "Invalid parameter";
    case IDCU_ERR_NO_MEMORY: return "Out of memory";
    case IDCU_ERR_PERM_DENIED: return "Permission denied";
    case IDCU_ERR_QUEUE_FULL: return "Queue full";
    case IDCU_ERR_QUEUE_EMPTY: return "Queue empty";
    case IDCU_ERR_MODULE_NOT_FOUND: return "Module not found";
    case IDCU_ERR_MODULE_INIT: return "Module initialization failed";
    case IDCU_ERR_CONFIG_LOAD: return "Config load failed";
    case IDCU_ERR_MSG_SEND: return "Message send failed";
    case IDCU_ERR_MSG_RECV: return "Message receive failed";
    case IDCU_ERR_CORO_CREATE: return "Coroutine create failed";
    case IDCU_ERR_SANDBOX_INIT: return "Sandbox init failed";
    case IDCU_ERR_MODULE_LOAD: return "Module load failed";
    case IDCU_ERR_MODULE_INVALID: return "Invalid module";
    case IDCU_ERR_NOT_FOUND: return "Not found";
    case IDCU_ERR_ALREADY_EXISTS: return "Already exists";
    case IDCU_ERR_BUSY: return "Resource busy";
    case IDCU_ERR_MODULE_RUN: return "Module run failed";
    case IDCU_ERR_MODULE_STOP: return "Module stop failed";
    case IDCU_ERR_MODULE_CRASH: return "Module crashed";
    case IDCU_ERR_DEPENDENCY: return "Dependency error";
    case IDCU_ERR_CIRCULAR_DEP: return "Circular dependency";
    case IDCU_ERR_VERSION_MISMATCH: return "Version mismatch";
    case IDCU_ERR_NOT_INITIALIZED: return "Not initialized";
    case IDCU_ERR_LOCK_FAILED: return "Lock failed";
    case IDCU_ERR_UNLOCK_FAILED: return "Unlock failed";
    case IDCU_ERR_MSG_TIMEOUT: return "Message timeout";
    case IDCU_ERR_MSG_DROPPED: return "Message dropped";
    case IDCU_ERR_HEALTH_CHECK_FAIL: return "Health check failed";
    case IDCU_ERR_CONFIG_PARSE: return "Config parse error";
    case IDCU_ERR_TIMEOUT: return "Timeout";
    case IDCU_ERR_CANCELLED: return "Cancelled";
    case IDCU_ERR_CONFIG_INVALID: return "Invalid config";
    case IDCU_ERR_DEADLOCK: return "Deadlock detected";
    case IDCU_ERR_SANDBOX_ACCESS: return "Sandbox access denied";
    case IDCU_ERR_SANDBOX_VIOLATION: return "Sandbox violation";
    case IDCU_ERR_NETWORK_INIT: return "Network init failed";
    case IDCU_ERR_NETWORK_CONNECT: return "Network connect failed";
    case IDCU_ERR_NETWORK_SEND: return "Network send failed";
    case IDCU_ERR_NETWORK_RECV: return "Network receive failed";
    case IDCU_ERR_NETWORK_TIMEOUT: return "Network timeout";
    case IDCU_ERR_METRICS_COLLECT: return "Metrics collect failed";
    case IDCU_ERR_FILE_OPEN: return "File open failed";
    case IDCU_ERR_FILE_READ: return "File read failed";
    case IDCU_ERR_FILE_WRITE: return "File write failed";
    case IDCU_ERR_FILE_DELETE: return "File delete failed";
    case IDCU_ERR_CORO_SCHEDULE: return "Coroutine schedule failed";
    case IDCU_ERR_CORO_YIELD: return "Coroutine yield failed";
    case IDCU_ERR_BUFFER_TOO_SMALL: return "Buffer too small";
    case IDCU_ERR_VERSION_TOO_OLD: return "Version too old";
    case IDCU_ERR_VERSION_TOO_NEW: return "Version too new";
    case IDCU_ERR_VERSION_INCOMPATIBLE: return "Version incompatible";
    case IDCU_ERR_MEMORY: return "Memory allocation failed";
    case IDCU_ERR_LIMIT_EXCEEDED: return "Limit exceeded";
    case IDCU_ERR_IO_ERROR: return "IO error";
    case IDCU_ERR_SECURITY_ERROR: return "Security error";
    case IDCU_ERR_RESOURCE_EXHAUSTED: return "Resource exhausted";
    case IDCU_ERR_VERIFICATION_FAILED: return "Verification failed";
    default: return "Unknown error";
    }
}

void idcu_err_set_last_error(int err_code, const char* context, const char* file, int line)
{
    g_last_error.error_code = err_code;
    g_last_error.file = file;
    g_last_error.line = line;

    if (context) {
        strncpy(g_last_error.context, context, IDCU_ERROR_CONTEXT_MAX_LEN - 1);
        g_last_error.context[IDCU_ERROR_CONTEXT_MAX_LEN - 1] = '\0';
    } else {
        g_last_error.context[0] = '\0';
    }
}

const idcu_ErrorInfo* idcu_err_get_last_error(void)
{
    return &g_last_error;
}

void idcu_err_clear_last_error(void)
{
    memset(&g_last_error, 0, sizeof(g_last_error));
}
```

### 4. 创建配置常量头文件 (config.h)

创建 `libs/idcu-common/include/idcu/common/config.h`：

```c
#ifndef IDCU_COMMON_CONFIG_H
#define IDCU_COMMON_CONFIG_H

#define IDCU_CONFIG_MAX_MODULES    32
#define IDCU_CONFIG_MAX_MSG        64
#define IDCU_CONFIG_STACK_CTX_SIZE 256
#define IDCU_CONFIG_LOG_LEVEL      1
#define IDCU_AGENT_VERSION_STR     "1.0.0"
#define IDCU_CONFIG_FILE_PATH      "config/agent.yaml"
#define IDCU_CONFIG_SECTION_MAX    64
#define IDCU_CONFIG_PATH_MAX       512

#define IDCU_PERM_SEND       (1U << 0)
#define IDCU_PERM_RECV       (1U << 1)
#define IDCU_PERM_RUN        (1U << 2)
#define IDCU_PERM_HW         (1U << 3)
#define IDCU_PERM_CONFIG     (1U << 4)
#define IDCU_PERM_LOG        (1U << 5)
#define IDCU_PERM_DEBUG      (1U << 6)
#define IDCU_PERM_MODULE_MGR (1U << 7)

#endif
```

### 5. 创建向量头文件 (vector.h)

创建 `libs/idcu-common/include/idcu/common/vector.h`：

```c
#ifndef IDCU_COMMON_VECTOR_H
#define IDCU_COMMON_VECTOR_H

#include "error_code.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    void** data;
    size_t size;
    size_t capacity;
    size_t element_size;
    void (*element_dtor)(void*);
} idcu_Vector;

int  idcu_vector_init(idcu_Vector* vec, size_t element_size, size_t initial_capacity);
int  idcu_vector_init_with_dtor(idcu_Vector* vec, size_t element_size, size_t initial_capacity,
                                void (*element_dtor)(void*));
void idcu_vector_destroy(idcu_Vector* vec);

int idcu_vector_push_back(idcu_Vector* vec, const void* element);
int idcu_vector_pop_back(idcu_Vector* vec, void* out_element);
int idcu_vector_insert(idcu_Vector* vec, size_t index, const void* element);
int idcu_vector_remove(idcu_Vector* vec, size_t index);

void* idcu_vector_get(const idcu_Vector* vec, size_t index);
int   idcu_vector_set(idcu_Vector* vec, size_t index, const void* element);

size_t idcu_vector_size(const idcu_Vector* vec);
size_t idcu_vector_capacity(const idcu_Vector* vec);
bool   idcu_vector_empty(const idcu_Vector* vec);
void   idcu_vector_clear(idcu_Vector* vec);

int idcu_vector_reserve(idcu_Vector* vec, size_t new_capacity);
int idcu_vector_resize(idcu_Vector* vec, size_t new_size, const void* default_value);

#define IDCU_VECTOR_FOR_EACH(vec, type, var, index)                \
    for (size_t index = 0; index < idcu_vector_size(vec); ++index) \
        for (type* var = (type*)idcu_vector_get(vec, index); var; var = NULL)

#ifdef __cplusplus
}
#endif

#endif
```

### 6. 创建向量实现文件 (vector.c)

创建 `libs/idcu-common/src/idcu/common/vector.c`：

```c
#include "idcu/common/vector.h"
#include <stdlib.h>
#include <string.h>

#define IDCU_VECTOR_DEFAULT_CAPACITY 8
#define IDCU_VECTOR_GROWTH_FACTOR 2

int idcu_vector_init(idcu_Vector* vec, size_t element_size, size_t initial_capacity)
{
    if (!vec || element_size == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (initial_capacity == 0) {
        initial_capacity = IDCU_VECTOR_DEFAULT_CAPACITY;
    }

    vec->data = (void**)malloc(initial_capacity * sizeof(void*));
    if (!vec->data) {
        return IDCU_ERR_NO_MEMORY;
    }

    vec->size = 0;
    vec->capacity = initial_capacity;
    vec->element_size = element_size;
    vec->element_dtor = NULL;

    return IDCU_ERR_OK;
}

int idcu_vector_init_with_dtor(idcu_Vector* vec, size_t element_size, size_t initial_capacity,
                               void (*element_dtor)(void*))
{
    int ret = idcu_vector_init(vec, element_size, initial_capacity);
    if (ret == IDCU_ERR_OK) {
        vec->element_dtor = element_dtor;
    }
    return ret;
}

void idcu_vector_destroy(idcu_Vector* vec)
{
    if (!vec)
        return;

    if (vec->element_dtor) {
        for (size_t i = 0; i < vec->size; ++i) {
            vec->element_dtor(vec->data[i]);
        }
    }

    for (size_t i = 0; i < vec->size; ++i) {
        free(vec->data[i]);
    }

    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

static int idcu_vector_grow(idcu_Vector* vec)
{
    size_t new_capacity = vec->capacity * IDCU_VECTOR_GROWTH_FACTOR;
    void** new_data = (void**)realloc(vec->data, new_capacity * sizeof(void*));
    if (!new_data) {
        return IDCU_ERR_NO_MEMORY;
    }
    vec->data = new_data;
    vec->capacity = new_capacity;
    return IDCU_ERR_OK;
}

int idcu_vector_push_back(idcu_Vector* vec, const void* element)
{
    if (!vec || !element) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (vec->size >= vec->capacity) {
        int ret = idcu_vector_grow(vec);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
    }

    void* new_element = malloc(vec->element_size);
    if (!new_element) {
        return IDCU_ERR_NO_MEMORY;
    }
    memcpy(new_element, element, vec->element_size);
    vec->data[vec->size++] = new_element;

    return IDCU_ERR_OK;
}

int idcu_vector_pop_back(idcu_Vector* vec, void* out_element)
{
    if (!vec || vec->size == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    void* element = vec->data[--vec->size];
    if (out_element) {
        memcpy(out_element, element, vec->element_size);
    }

    if (vec->element_dtor) {
        vec->element_dtor(element);
    }
    free(element);

    return IDCU_ERR_OK;
}

int idcu_vector_insert(idcu_Vector* vec, size_t index, const void* element)
{
    if (!vec || !element || index > vec->size) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (vec->size >= vec->capacity) {
        int ret = idcu_vector_grow(vec);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
    }

    for (size_t i = vec->size; i > index; --i) {
        vec->data[i] = vec->data[i - 1];
    }

    void* new_element = malloc(vec->element_size);
    if (!new_element) {
        return IDCU_ERR_NO_MEMORY;
    }
    memcpy(new_element, element, vec->element_size);
    vec->data[index] = new_element;
    vec->size++;

    return IDCU_ERR_OK;
}

int idcu_vector_remove(idcu_Vector* vec, size_t index)
{
    if (!vec || index >= vec->size) {
        return IDCU_ERR_INVALID_PARAM;
    }

    void* element = vec->data[index];

    if (vec->element_dtor) {
        vec->element_dtor(element);
    }
    free(element);

    for (size_t i = index; i < vec->size - 1; ++i) {
        vec->data[i] = vec->data[i + 1];
    }
    vec->size--;

    return IDCU_ERR_OK;
}

void* idcu_vector_get(const idcu_Vector* vec, size_t index)
{
    if (!vec || index >= vec->size) {
        return NULL;
    }
    return vec->data[index];
}

int idcu_vector_set(idcu_Vector* vec, size_t index, const void* element)
{
    if (!vec || !element || index >= vec->size) {
        return IDCU_ERR_INVALID_PARAM;
    }

    void* old_element = vec->data[index];
    if (vec->element_dtor) {
        vec->element_dtor(old_element);
    }
    free(old_element);

    void* new_element = malloc(vec->element_size);
    if (!new_element) {
        return IDCU_ERR_NO_MEMORY;
    }
    memcpy(new_element, element, vec->element_size);
    vec->data[index] = new_element;

    return IDCU_ERR_OK;
}

size_t idcu_vector_size(const idcu_Vector* vec) { return vec ? vec->size : 0; }
size_t idcu_vector_capacity(const idcu_Vector* vec) { return vec ? vec->capacity : 0; }
bool idcu_vector_empty(const idcu_Vector* vec) { return vec ? (vec->size == 0) : true; }

void idcu_vector_clear(idcu_Vector* vec)
{
    if (!vec)
        return;

    if (vec->element_dtor) {
        for (size_t i = 0; i < vec->size; ++i) {
            vec->element_dtor(vec->data[i]);
        }
    }

    for (size_t i = 0; i < vec->size; ++i) {
        free(vec->data[i]);
    }

    vec->size = 0;
}

int idcu_vector_reserve(idcu_Vector* vec, size_t new_capacity)
{
    if (!vec) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (new_capacity <= vec->capacity) {
        return IDCU_ERR_OK;
    }

    void** new_data = (void**)realloc(vec->data, new_capacity * sizeof(void*));
    if (!new_data) {
        return IDCU_ERR_NO_MEMORY;
    }
    vec->data = new_data;
    vec->capacity = new_capacity;
    return IDCU_ERR_OK;
}

int idcu_vector_resize(idcu_Vector* vec, size_t new_size, const void* default_value)
{
    if (!vec) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (new_size < vec->size) {
        for (size_t i = new_size; i < vec->size; ++i) {
            if (vec->element_dtor) {
                vec->element_dtor(vec->data[i]);
            }
            free(vec->data[i]);
        }
        vec->size = new_size;
    } else if (new_size > vec->size) {
        for (size_t i = vec->size; i < new_size; ++i) {
            int ret = idcu_vector_push_back(vec, default_value);
            if (ret != IDCU_ERR_OK) {
                return ret;
            }
        }
    }

    return IDCU_ERR_OK;
}
```

### 7. 创建哈希表头文件 (hash_map.h)

创建 `libs/idcu-common/include/idcu/common/hash_map.h`：

```c
#ifndef IDCU_COMMON_HASH_MAP_H
#define IDCU_COMMON_HASH_MAP_H

#include "error_code.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct idcu_HashMapEntry
{
    char* key;
    void* value;
    struct idcu_HashMapEntry* next;
} idcu_HashMapEntry;

typedef struct
{
    idcu_HashMapEntry** buckets;
    size_t bucket_count;
    size_t size;
    size_t value_size;
    void (*value_dtor)(void*);
} idcu_HashMap;

typedef struct
{
    idcu_HashMap* map;
    size_t bucket_index;
    idcu_HashMapEntry* current_entry;
} idcu_HashMapIterator;

int  idcu_hash_map_init(idcu_HashMap* map, size_t bucket_count, size_t value_size);
int  idcu_hash_map_init_with_dtor(idcu_HashMap* map, size_t bucket_count, size_t value_size,
                                   void (*value_dtor)(void*));
void idcu_hash_map_destroy(idcu_HashMap* map);

int  idcu_hash_map_put(idcu_HashMap* map, const char* key, const void* value);
int  idcu_hash_map_get(const idcu_HashMap* map, const char* key, void* out_value);
int  idcu_hash_map_remove(idcu_HashMap* map, const char* key);
bool idcu_hash_map_contains(const idcu_HashMap* map, const char* key);

size_t idcu_hash_map_size(const idcu_HashMap* map);
bool   idcu_hash_map_empty(const idcu_HashMap* map);
void   idcu_hash_map_clear(idcu_HashMap* map);

void idcu_hash_map_iterator_init(idcu_HashMapIterator* iter, idcu_HashMap* map);
bool idcu_hash_map_iterator_next(idcu_HashMapIterator* iter, const char** out_key, void** out_value);

#define IDCU_HASH_MAP_FOR_EACH(map, key_var, value_var, iter)                          \
    for (idcu_hash_map_iterator_init(&(iter), (map));                                    \
         idcu_hash_map_iterator_next(&(iter), &(key_var), (void**)&(value_var));)

#ifdef __cplusplus
}
#endif

#endif
```

### 8. 创建字符串缓冲区头文件 (string_buf.h)

创建 `libs/idcu-common/include/idcu/common/string_buf.h`：

```c
#ifndef IDCU_COMMON_STRING_BUF_H
#define IDCU_COMMON_STRING_BUF_H

#include "error_code.h"
#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    char* data;
    size_t size;
    size_t capacity;
} idcu_StringBuf;

int  idcu_string_buf_init(idcu_StringBuf* buf, size_t initial_capacity);
void idcu_string_buf_destroy(idcu_StringBuf* buf);

int  idcu_string_buf_append(idcu_StringBuf* buf, const char* str);
int  idcu_string_buf_append_n(idcu_StringBuf* buf, const char* str, size_t len);
int  idcu_string_buf_append_char(idcu_StringBuf* buf, char c);
int  idcu_string_buf_append_format(idcu_StringBuf* buf, const char* format, ...);
int  idcu_string_buf_vappend_format(idcu_StringBuf* buf, const char* format, va_list args);

int  idcu_string_buf_clear(idcu_StringBuf* buf);
int  idcu_string_buf_reserve(idcu_StringBuf* buf, size_t new_capacity);

const char* idcu_string_buf_data(const idcu_StringBuf* buf);
size_t      idcu_string_buf_size(const idcu_StringBuf* buf);
size_t      idcu_string_buf_capacity(const idcu_StringBuf* buf);
bool        idcu_string_buf_empty(const idcu_StringBuf* buf);

#ifdef __cplusplus
}
#endif

#endif
```

### 9. 创建锁头文件 (lock.h)

创建 `libs/idcu-common/include/idcu/common/lock.h`：

```c
#ifndef IDCU_COMMON_LOCK_H
#define IDCU_COMMON_LOCK_H

#include "error_code.h"

#ifdef _WIN32
#include <windows.h>
typedef CRITICAL_SECTION idcu_Mutex;
#else
#include <pthread.h>
typedef pthread_mutex_t idcu_Mutex;
#endif

#ifdef __cplusplus
extern "C" {
#endif

int  idcu_mutex_init(idcu_Mutex* mutex);
void idcu_mutex_destroy(idcu_Mutex* mutex);
int  idcu_mutex_lock(idcu_Mutex* mutex);
int  idcu_mutex_unlock(idcu_Mutex* mutex);
int  idcu_mutex_trylock(idcu_Mutex* mutex);

typedef struct
{
    idcu_Mutex* mutex;
} idcu_LockGuard;

int idcu_lock_guard_init(idcu_LockGuard* guard, idcu_Mutex* mutex);
void idcu_lock_guard_destroy(idcu_LockGuard* guard);

#ifdef __cplusplus
}
#endif

#endif
```

### 10. 创建原子操作头文件 (atomic.h)

创建 `libs/idcu-common/include/idcu/common/atomic.h`：

```c
#ifndef IDCU_COMMON_ATOMIC_H
#define IDCU_COMMON_ATOMIC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef volatile int32_t idcu_AtomicInt32;
typedef volatile int64_t idcu_AtomicInt64;
typedef volatile uint32_t idcu_AtomicUInt32;
typedef volatile uint64_t idcu_AtomicUInt64;

int32_t idcu_atomic_load_int32(idcu_AtomicInt32* ptr);
void    idcu_atomic_store_int32(idcu_AtomicInt32* ptr, int32_t value);
int32_t idcu_atomic_fetch_add_int32(idcu_AtomicInt32* ptr, int32_t value);
int32_t idcu_atomic_fetch_sub_int32(idcu_AtomicInt32* ptr, int32_t value);
int32_t idcu_atomic_inc_int32(idcu_AtomicInt32* ptr);
int32_t idcu_atomic_dec_int32(idcu_AtomicInt32* ptr);
int     idcu_atomic_cas_int32(idcu_AtomicInt32* ptr, int32_t expected, int32_t desired);

int64_t idcu_atomic_load_int64(idcu_AtomicInt64* ptr);
void    idcu_atomic_store_int64(idcu_AtomicInt64* ptr, int64_t value);
int64_t idcu_atomic_fetch_add_int64(idcu_AtomicInt64* ptr, int64_t value);
int64_t idcu_atomic_fetch_sub_int64(idcu_AtomicInt64* ptr, int64_t value);
int64_t idcu_atomic_inc_int64(idcu_AtomicInt64* ptr);
int64_t idcu_atomic_dec_int64(idcu_AtomicInt64* ptr);
int     idcu_atomic_cas_int64(idcu_AtomicInt64* ptr, int64_t expected, int64_t desired);

#ifdef __cplusplus
}
#endif

#endif
```

### 11. 创建 CMakeLists.txt

创建 `libs/idcu-common/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-common C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-common STATIC
    src/idcu/common/error_code.c
    src/idcu/common/vector.c
    src/idcu/common/hash_map.c
    src/idcu/common/string_buf.c
    src/idcu/common/lock.c
    src/idcu/common/atomic.c
    src/idcu/common/linked_list.c
    src/idcu/common/security.c
)

target_include_directories(idcu-common PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(WIN32)
    target_link_libraries(idcu-common PRIVATE)
else()
    target_link_libraries(idcu-common PRIVATE pthread)
endif()

add_library(idcu::common ALIAS idcu-common)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 12. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-common/module.yaml`：

```yaml
name: idcu-common
version: 1.0.0
description: Common utilities library for IDCU Agent
author: IDCU Team
license: MIT

dependencies: []

build:
  type: cmake
  targets:
    - idcu-common

headers:
  - idcu/common/error_code.h
  - idcu/common/config.h
  - idcu/common/vector.h
  - idcu/common/hash_map.h
  - idcu/common/string_buf.h
  - idcu/common/lock.h
  - idcu/common/atomic.h
  - idcu/common/linked_list.h
  - idcu/common/security.h
  - idcu/common/option.h

features:
  - error_code: Unified error code definitions
  - vector: Dynamic array implementation
  - hash_map: Hash map implementation
  - string_buf: String buffer for dynamic string manipulation
  - lock: Mutex and synchronization primitives
  - atomic: Atomic operations
  - linked_list: Doubly linked list
  - security: Security utilities
  - option: Optional type

testing:
  enabled: true
  framework: internal
```

### 13. 创建 README.md

创建 `libs/idcu-common/README.md`：

```markdown
# idcu-common

通用工具库，为 IDCU Agent 提供基础支持。

## 功能特性

- **统一错误码**: 完整的错误码定义和错误信息管理
- **数据结构**: 向量、哈希表、链表等常用数据结构
- **并发原语**: 互斥锁、原子操作等并发支持
- **字符串处理**: 动态字符串缓冲区
- **安全工具**: 基础安全辅助函数

## 快速开始

### 错误码使用

```c
#include "idcu/common/error_code.h"

int result = some_function();
if (!idcu_err_is_ok(result)) {
    printf("Error: %s\n", idcu_err_to_str(result));
    return result;
}
```

### 向量使用

```c
#include "idcu/common/vector.h"

idcu_Vector vec;
idcu_vector_init(&vec, sizeof(int), 8);

int value = 42;
idcu_vector_push_back(&vec, &value);

int* retrieved = (int*)idcu_vector_get(&vec, 0);
printf("Value: %d\n", *retrieved);

idcu_vector_destroy(&vec);
```

## 构建

```bash
mkdir build && cd build
cmake ..
make
```

## API 文档

详见 [include/idcu/common/](include/idcu/common/)
```

## 验证检查清单

- [ ] 所有头文件已创建
- [ ] 所有源文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 代码可以成功编译
- [ ] 所有数据结构的基本操作正常工作
- [ ] 错误码转换功能正常

## Git 提交

```bash
git add libs/idcu-common/
git commit -m "feat: add idcu-common library

- Add error_code with unified error definitions
- Add vector dynamic array implementation
- Add hash_map implementation
- Add string_buf for string manipulation
- Add lock and atomic operations
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 编译错误: pthread 未找到 | Linux 系统缺少 pthread 库 | 确保 CMakeLists.txt 中链接了 pthread |
| 内存泄漏 | 忘记调用 destroy 函数 | 确保所有 init 的资源都有对应的 destroy |
| 线程安全问题 | 未正确使用锁 | 在多线程环境中使用 idcu_Mutex 保护共享数据 |
