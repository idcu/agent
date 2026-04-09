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
