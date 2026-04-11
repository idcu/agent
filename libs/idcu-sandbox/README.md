# idcu-sandbox

IDCU Agent 的沙箱库。

## 功能特性
- 代码沙箱隔离
- 资源限制
- 安全执行环境
- 线程安全实现

## 使用方法

```c
#include <idcu/sandbox/sandbox.h>

idcu_Sandbox* sandbox;
int ret = idcu_sandbox_init(&sandbox);
if (ret == IDCU_ERR_OK) {
    // 设置资源限制
    idcu_sandbox_set_memory_limit(sandbox, 1024 * 1024); // 1MB
    idcu_sandbox_set_cpu_limit(sandbox, 10); // 10% CPU
    
    // 在沙箱中执行
    idcu_sandbox_execute(sandbox, my_function, NULL);
    
    // 清理
    idcu_sandbox_destroy(sandbox);
}
```

## API 参考
- `idcu_sandbox_init()` - 初始化沙箱
- `idcu_sandbox_destroy()` - 销毁沙箱
- `idcu_sandbox_is_initialized()` - 检查是否已初始化
- `idcu_sandbox_set_memory_limit()` - 设置内存限制
- `idcu_sandbox_set_cpu_limit()` - 设置 CPU 限制
- `idcu_sandbox_execute()` - 在沙箱中执行
- `idcu_sandbox_get_operation_count()` - 获取操作计数
- `idcu_sandbox_get_error_count()` - 获取错误计数

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
