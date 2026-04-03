# Module - 模块系统

本模块提供模块的定义、注册和动态加载功能。

## 模块内容

### module_def.h - 模块接口定义
定义模块的标准接口和注册机制：
- `idcu_ModuleInterface` 结构体：定义模块的生命周期函数
- `idcu_ModuleState` 枚举：模块状态 (UNINIT, INITED, RUNNING, STOPPED, ERROR)
- `idcu_ModulePrio` 枚举：模块优先级 (LOW, NORMAL, HIGH, REALTIME)
- `IDCU_REGISTER_MODULE` 宏：用于注册静态模块

### module_registry.h/c - 模块注册表
管理已注册模块的生命周期：
- 模块注册和注销
- 按 ID 或名称查找模块
- 模块初始化、运行、停止
- 支持批量操作所有模块
- 最大支持 64 个注册模块

### dynamic_module.h / dynamic_loader.c - 动态模块加载
支持运行时动态加载模块：
- Windows (LoadLibrary) 和 Linux (dlopen) 跨平台支持
- 从共享库加载模块
- 模块引用计数管理
- 模块路径配置
- 最大支持 32 个动态模块

## 使用示例

### 定义和注册静态模块
```c
#include "module/module_def.h"

static int my_module_init(void) {
    printf("模块初始化\n");
    return 0;
}

static int my_module_run(void) {
    // 模块运行逻辑
    return 0;
}

static int my_module_stop(void) {
    printf("模块停止\n");
    return 0;
}

IDCU_REGISTER_MODULE(my_module, my_module_init, my_module_run, my_module_stop);
```

### 使用模块注册表
```c
#include "module/module_registry.h"

idcu_ModuleRegistry registry;
idcu_module_registry_init(&registry);

// 注册模块
idcu_module_registry_register(&registry, &__idcu_module_my_module, IDCU_MOD_PRIO_NORMAL);

// 初始化和运行模块
idcu_module_registry_init_module(&registry, module_id);
idcu_module_registry_run_module(&registry, module_id);

idcu_module_registry_destroy(&registry);
```

### 动态加载模块
```c
#include "module/dynamic_module.h"

idcu_DynamicLoader loader;
idcu_dynamic_loader_init(&loader, "./modules");

// 加载模块
idcu_dynamic_loader_load_module(&loader, "my_plugin", "./plugins/my_plugin.dll");

// 获取和使用模块
idcu_DynamicModule* mod = idcu_dynamic_loader_find_module(&loader, "my_plugin");
if (mod) {
    idcu_dynamic_module_init(mod);
    idcu_dynamic_module_run(mod);
}

idcu_dynamic_loader_destroy(&loader);
```
