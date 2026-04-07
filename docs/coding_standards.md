# IDCU Agent C 语言编码规范

## 1. 概述

本文档规定了 IDCU Agent 项目的 C 语言编码规范，所有贡献者必须遵守。

### 1.1 C 语言标准

本项目使用 **C99** 标准。

- CMake 配置：`set(CMAKE_C_STANDARD 99)`
- 编译器标志：`-std=c99`

## 2. 代码风格

代码风格通过 `.clang-format` 自动格式化，主要遵循 Google 风格，主要配置如下：

- 缩进：4 空格
- 列宽：100 字符
- 花括号：Allman 风格（换行）
- 指针对齐：左对齐

**使用方式：**
```bash
# 格式化单个文件
clang-format -i file.c

# 格式化所有源文件
find libs modules app -name "*.c" -o -name "*.h" | xargs clang-format -i
```

## 3. 命名规范

### 3.1 通用规则

- 使用 **snake_case**（小写+下划线）命名
- 避免使用缩写，除非是广泛认可的缩写（如 `ctx`、`cfg`、`len`）

### 3.2 文件命名

- 头文件：`module_name.h`
- 源文件：`module_name.c`
- 测试文件：`test_module_name.c`

### 3.3 函数命名

格式：`prefix_action_object()`

示例：
```c
idcu_err_t idcu_memory_pool_alloc(idcu_memory_pool_t *pool, size_t size);
void idcu_log_info(const char *fmt, ...);
```

### 3.4 宏命名

格式：`PREFIX_NAME`（全大写+下划线）

示例：
```c
#define IDCU_MAX_BUFFER_SIZE 4096
#define IDCU_IS_VALID(x) ((x) != NULL)
```

### 3.5 类型命名

- 结构体：`prefix_name_t`
- 枚举：`prefix_name_t`
- 函数指针：`prefix_name_fn_t`

示例：
```c
typedef struct idcu_memory_pool idcu_memory_pool_t;
typedef enum idcu_error_code idcu_error_code_t;
typedef void (*idcu_callback_fn_t)(void *ctx);
```

### 3.6 变量命名

- 局部变量：`snake_case`
- 成员变量：`snake_case`
- 全局变量：`g_prefix_name`（尽量避免使用）

## 4. 错误处理规范

### 4.1 返回码

使用统一的错误码定义（`idcu_ErrorCode`），定义在 `libs/idcu-common/include/idcu/common/error_code.h`

主要返回码：
- `IDCU_ERR_OK` / `IDCU_ERR_SUCCESS`：成功
- `IDCU_ERR_INVALID_PARAM`：参数无效
- `IDCU_ERR_NO_MEMORY`：内存不足
- `IDCU_ERR_PERM_DENIED`：权限拒绝
- 等等

### 4.2 错误处理模式

```c
idcu_err_t some_function(idcu_context_t *ctx) {
    if (ctx == NULL) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_err_t err = do_something();
    if (err != IDCU_ERR_OK) {
        idcu_log_error("do_something failed: %s", idcu_err_to_str(err));
        return err;
    }

    return IDCU_ERR_OK;
}
```

### 4.3 日志要求

使用 `idcu-log` 模块进行日志记录：

```c
idcu_log_debug("Debug message: %d", value);
idcu_log_info("Info message");
idcu_log_warn("Warning: something might be wrong");
idcu_log_error("Error: %s", idcu_err_to_str(err));
```

## 5. 内存管理

### 5.1 内存池使用场景

以下场景必须使用内存池（`idcu-memory` 模块）：

- 频繁分配/释放的小对象（< 4KB）
- 生命周期与模块/请求绑定的内存
- 性能关键路径

### 5.2 内存分配模式

```c
// 使用内存池
idcu_memory_pool_t *pool = idcu_memory_pool_create(1024 * 1024);
void *ptr = idcu_memory_pool_alloc(pool, size);
// 使用 ptr...
idcu_memory_pool_destroy(pool);  // 自动释放所有分配的内存

// 直接分配（仅用于大对象或长生命周期）
void *ptr = malloc(size);
if (ptr == NULL) {
    return IDCU_ERR_NO_MEMORY;
}
// 使用 ptr...
free(ptr);
```

### 5.3 内存安全

- 禁止使用未初始化的内存
- 数组访问必须进行边界检查
- 字符串操作使用安全函数（如 `strncpy`、`snprintf`）

## 6. 注释规范

### 6.1 Doxygen 风格注释

所有公共 API 必须使用 Doxygen 风格注释。

#### 6.1.1 文件头注释

```c
/**
 * @file memory_pool.h
 * @brief 内存池管理模块
 *
 * 提供高效的内存池实现，支持批量分配和释放。
 */
```

#### 6.1.2 函数注释

```c
/**
 * @brief 创建内存池
 *
 * @param pool_size 内存池总大小（字节）
 * @return idcu_memory_pool_t* 成功返回内存池指针，失败返回 NULL
 */
idcu_memory_pool_t *idcu_memory_pool_create(size_t pool_size);

/**
 * @brief 从内存池分配内存
 *
 * @param pool 内存池指针
 * @param size 要分配的大小（字节）
 * @return void* 成功返回指针，失败返回 NULL
 */
void *idcu_memory_pool_alloc(idcu_memory_pool_t *pool, size_t size);
```

#### 6.1.3 结构体注释

```c
/**
 * @brief 内存池结构体
 *
 * 内部使用，外部不应直接访问成员。
 */
typedef struct idcu_memory_pool {
    size_t total_size;      /**< 总大小 */
    size_t used_size;       /**< 已使用大小 */
    void *memory;           /**< 内存指针 */
    idcu_lock_t lock;       /**< 锁 */
} idcu_memory_pool_t;
```

### 6.2 行内注释

关键逻辑（协程调度、沙箱隔离等）必须添加行内注释：

```c
// 保存当前上下文
idcu_coroutine_context_save(&current->ctx);

// 切换到下一个协程
idcu_coroutine_context_switch(next->ctx);

// 检查沙箱访问权限
if (!idcu_sandbox_check_access(sandbox, address)) {
    return IDCU_ERR_SANDBOX_ACCESS;
}
```

## 7. 静态检查

### 7.1 clang-tidy

使用 `.clang-tidy` 配置进行静态检查，主要检查项：

- bugprone-*：易出错的代码模式
- clang-analyzer-*：Clang 静态分析器
- concurrency-*：并发相关问题
- performance-*：性能问题
- readability-*：可读性问题

**使用方式：**
```bash
# CMake 构建时启用
cmake -DENABLE_CLANG_TIDY=ON ..
cmake --build .
```

### 7.2 CI 检查

CI 流程会自动执行：
1. clang-format 检查（不通过则阻断）
2. clang-tidy 静态检查（不通过则阻断）

## 8. 参考

- [.clang-format](../.clang-format)
- [.clang-tidy](../.clang-tidy)
- [CI 配置](../.github/workflows/ci.yml)
