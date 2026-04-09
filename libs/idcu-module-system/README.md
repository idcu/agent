# idcu-module-system

IDCU Agent 的模块系统库。

## 功能特性

- **模块定义**: 提供完整的模块定义 API
- **模块注册表**: 按名称、ID 和类别管理模块
- **生命周期管理**: 完整的模块生命周期（init/start/stop/destroy）
- **依赖解析**: 模块依赖关系解析和检查
- **动态加载**: 跨平台动态模块加载支持

## 快速开始

### 定义和注册模块

```c
#include <idcu/module/module.h>

static idcu_ErrorCode my_module_init(void)
{
    printf("My module initialized\n");
    return IDCU_SUCCESS;
}

static idcu_ErrorCode my_module_start(void)
{
    printf("My module started\n");
    return IDCU_SUCCESS;
}

static idcu_ErrorCode my_module_stop(void)
{
    printf("My module stopped\n");
    return IDCU_SUCCESS;
}

static idcu_ErrorCode my_module_destroy(void)
{
    printf("My module destroyed\n");
    return IDCU_SUCCESS;
}

static const idcu_ModuleDef my_module_def = {
    .name = "my-module",
    .version_str = "1.0.0",
    .description = "My example module",
    .category = "example",
    .init = my_module_init,
    .start = my_module_start,
    .stop = my_module_stop,
    .destroy = my_module_destroy,
    .dependencies = NULL,
    .dependency_count = 0
};

int main(void)
{
    idcu_ModuleSystem system;
    int err = idcu_module_system_init(&system);
    if (err != IDCU_SUCCESS) {
        return 1;
    }

    err = idcu_module_system_register(&system, &my_module_def);
    if (err != IDCU_SUCCESS) {
        idcu_module_system_destroy(&system);
        return 1;
    }

    err = idcu_module_system_init_all(&system);
    if (err != IDCU_SUCCESS) {
        idcu_module_system_destroy(&system);
        return 1;
    }

    err = idcu_module_system_start_all(&system);
    if (err != IDCU_SUCCESS) {
        idcu_module_system_destroy(&system);
        return 1;
    }

    idcu_module_system_stop_all(&system);
    idcu_module_system_destroy_all(&system);
    idcu_module_system_destroy(&system);

    return 0;
}
```

## API 文档

### 模块系统初始化

```c
int idcu_module_system_init(idcu_ModuleSystem* system);
void idcu_module_system_destroy(idcu_ModuleSystem* system);
```

### 模块注册

```c
int idcu_module_system_register(idcu_ModuleSystem* system, const idcu_ModuleDef* def);
int idcu_module_system_unregister(idcu_ModuleSystem* system, const char* name);
```

### 模块查询

```c
int idcu_module_system_find_by_name(idcu_ModuleSystem* system, const char* name, idcu_Module** out_module);
int idcu_module_system_find_by_id(idcu_ModuleSystem* system, uint32_t id, idcu_Module** out_module);
int idcu_module_system_get_all(idcu_ModuleSystem* system, idcu_ModuleInfo** out_infos, size_t* out_count);
```

### 生命周期管理

```c
int idcu_module_system_init_all(idcu_ModuleSystem* system);
int idcu_module_system_start_all(idcu_ModuleSystem* system);
int idcu_module_system_stop_all(idcu_ModuleSystem* system);
int idcu_module_system_destroy_all(idcu_ModuleSystem* system);

int idcu_module_system_init_module(idcu_ModuleSystem* system, const char* name);
int idcu_module_system_start_module(idcu_ModuleSystem* system, const char* name);
int idcu_module_system_stop_module(idcu_ModuleSystem* system, const char* name);
int idcu_module_system_destroy_module(idcu_ModuleSystem* system, const char* name);
```

### 模块状态

```c
const char* idcu_module_state_to_str(idcu_ModuleState state);
```

## 构建

该库使用 CMake 构建：

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## 依赖

- idcu-common
