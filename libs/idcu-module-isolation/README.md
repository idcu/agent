# idcu-module-isolation

IDCU Agent 的模块隔离库。

## 功能特性
- 模块隔离执行
- 资源访问控制
- 模块间安全通信
- 线程安全实现

## 使用方法

```c
#include <idcu/module-isolation/module-isolation.h>

idcu_ModuleIsolation* isolation;
int ret = idcu_module_isolation_init(&isolation);
if (ret == IDCU_ERR_OK) {
    // 加载隔离模块
    idcu_module_isolation_load(isolation, "my-module");
    
    // 在隔离环境中执行
    idcu_module_isolation_execute(isolation, "my-function", NULL);
    
    // 清理
    idcu_module_isolation_destroy(isolation);
}
```

## API 参考
- `idcu_module_isolation_init()` - 初始化模块隔离
- `idcu_module_isolation_destroy()` - 销毁模块隔离
- `idcu_module_isolation_is_initialized()` - 检查是否已初始化
- `idcu_module_isolation_load()` - 加载模块
- `idcu_module_isolation_unload()` - 卸载模块
- `idcu_module_isolation_execute()` - 执行模块函数
- `idcu_module_isolation_get_operation_count()` - 获取操作计数
- `idcu_module_isolation_get_error_count()` - 获取错误计数

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
