# idcu-module-system API 文档

用于动态模块加载和管理的模块系统。

## 模块定义

```c
typedef struct idcu_ModuleDef {
    const char* name;
    const char* version_str;
    const char* description;
    const char* category;
    
    idcu_ErrorCode (*init)(void);
    idcu_ErrorCode (*start)(void);
    idcu_ErrorCode (*stop)(void);
    idcu_ErrorCode (*destroy)(void);
    
    const char** dependencies;
    size_t dependency_count;
} idcu_ModuleDef;
```

## 模块系统

```c
typedef struct idcu_ModuleSystem idcu_ModuleSystem;
```

### 模块系统函数

```c
int idcu_module_system_init(idcu_ModuleSystem* system);
void idcu_module_system_destroy(idcu_ModuleSystem* system);

int idcu_module_system_register(idcu_ModuleSystem* system, const idcu_ModuleDef* module_def);
int idcu_module_system_unregister(idcu_ModuleSystem* system, const char* name);

int idcu_module_system_find_by_name(idcu_ModuleSystem* system, const char* name, idcu_Module** out_module);
int idcu_module_system_get_all(idcu_ModuleSystem* system, idcu_ModuleInfo** out_infos, size_t* out_count);
int idcu_module_system_get_by_category(idcu_ModuleSystem* system, const char* category, idcu_ModuleInfo** out_infos, size_t* out_count);

int idcu_module_system_init_module(idcu_ModuleSystem* system, const char* name);
int idcu_module_system_start_module(idcu_ModuleSystem* system, const char* name);
int idcu_module_system_stop_module(idcu_ModuleSystem* system, const char* name);
int idcu_module_system_destroy_module(idcu_ModuleSystem* system, const char* name);

int idcu_module_system_init_all(idcu_ModuleSystem* system);
int idcu_module_system_start_all(idcu_ModuleSystem* system);
int idcu_module_system_stop_all(idcu_ModuleSystem* system);
int idcu_module_system_destroy_all(idcu_ModuleSystem* system);
```

## 模块信息

```c
typedef struct {
    const char* name;
    const char* version_str;
    const char* description;
    const char* category;
    idcu_ModuleState state;
} idcu_ModuleInfo;
```

## 模块状态

```c
typedef enum {
    IDCU_MODULE_STATE_UNREGISTERED,
    IDCU_MODULE_STATE_REGISTERED,
    IDCU_MODULE_STATE_INITIALIZED,
    IDCU_MODULE_STATE_STARTED,
    IDCU_MODULE_STATE_STOPPED,
    IDCU_MODULE_STATE_DESTROYED
} idcu_ModuleState;
```

## 示例

```c
#include <idcu/module/module.h>
#include <stdio.h>

// 定义一个模块
static idcu_ErrorCode my_module_init(void) {
    printf("我的模块已初始化\n");
    return IDCU_ERR_OK;
}

static idcu_ErrorCode my_module_start(void) {
    printf("我的模块已启动\n");
    return IDCU_ERR_OK;
}

static idcu_ErrorCode my_module_stop(void) {
    printf("我的模块已停止\n");
    return IDCU_ERR_OK;
}

static idcu_ErrorCode my_module_destroy(void) {
    printf("我的模块已销毁\n");
    return IDCU_ERR_OK;
}

static const idcu_ModuleDef my_module = {
    .name = "my-module",
    .version_str = "1.0.0",
    .description = "我的示例模块",
    .category = "example",
    .init = my_module_init,
    .start = my_module_start,
    .stop = my_module_stop,
    .destroy = my_module_destroy,
    .dependencies = NULL,
    .dependency_count = 0
};

int main(void) {
    idcu_ModuleSystem system;
    idcu_module_system_init(&system);
    
    // 注册模块
    idcu_module_system_register(&system, &my_module);
    
    // 初始化和启动
    idcu_module_system_init_module(&system, "my-module");
    idcu_module_system_start_module(&system, "my-module");
    
    // 停止和销毁
    idcu_module_system_stop_module(&system, "my-module");
    idcu_module_system_destroy_module(&system, "my-module");
    
    idcu_module_system_destroy(&system);
    return 0;
}
```
