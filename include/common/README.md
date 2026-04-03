# Common - 通用工具模块

本模块提供项目的基础工具和通用组件。

## 模块内容

### atomic.h/c - 原子操作
提供跨平台的原子操作接口，支持：
- 32位和64位整数的原子加减
- 原子逻辑运算 (AND/OR/XOR)
- 原子交换和比较交换
- 原子加载和存储

### lock.h/c - 锁机制
提供跨平台的互斥锁和读写锁：
- idcu_Mutex: 互斥锁
- idcu_RwLock: 读写锁 (多读单写)

### config.h - 配置常量
定义项目的全局配置常量：
- 最大模块数
- 最大消息数
- 栈上下文大小
- 日志级别
- 权限定义

### error_code.h - 错误码定义
定义统一的错误码枚举和错误信息转换函数：
- 基础错误码 (成功、通用错误、参数无效等)
- 模块相关错误码
- 网络相关错误码
- `idcu_err_to_str()` 函数将错误码转换为字符串

## 使用示例

### 使用原子操作
```c
#include "common/atomic.h"

volatile int32_t counter = 0;
idcu_atomic_fetch_add_int32(&counter, 1);
```

### 使用互斥锁
```c
#include "common/lock.h"

idcu_Mutex lock;
idcu_mutex_init(&lock);
idcu_mutex_lock(&lock);
// 临界区代码
idcu_mutex_unlock(&lock);
idcu_mutex_destroy(&lock);
```

### 使用错误码
```c
#include "common/error_code.h"

int result = some_function();
if (result != IDCU_ERR_OK) {
    printf("Error: %s\n", idcu_err_to_str(result));
}
```
