# 任务 2.3: 模块系统

## 目标

创建完整的模块系统，包括：
- 模块定义和注册
- 模块生命周期管理
- 依赖解析和加载
- 模块版本管理
- 模块分类系统

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-module-system/include/idcu/module
mkdir -p libs/idcu-module-system/src/idcu/module
mkdir -p libs/idcu-module-system/tests
mkdir -p libs/idcu-module-system/examples
```

### 2. 创建模块定义头文件 (module_def.h)

创建 `libs/idcu-module-system/include/idcu/module/module_def.h`：

```c
#ifndef IDCU_MODULE_MODULE_DEF_H
#define IDCU_MODULE_MODULE_DEF_H

#include "idcu/common/error_code.h"
#include "idcu/common/config.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_MODULE_STATE_UNLOADED = 0,
    IDCU_MODULE_STATE_LOADED,
    IDCU_MODULE_STATE_INITIALIZED,
    IDCU_MODULE_STATE_STARTING,
    IDCU_MODULE_STATE_RUNNING,
    IDCU_MODULE_STATE_STOPPING,
    IDCU_MODULE_STATE_STOPPED,
    IDCU_MODULE_STATE_ERROR
} idcu_ModuleState;

typedef enum
{
    IDCU_MODULE_CATEGORY_CORE = 0,
    IDCU_MODULE_CATEGORY_INTEGRATION,
    IDCU_MODULE_CATEGORY_BUSINESS,
    IDCU_MODULE_CATEGORY_SERVICE,
    IDCU_MODULE_CATEGORY_CUSTOM
} idcu_ModuleCategory;

typedef struct idcu_ModuleVersion
{
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    const char* pre_release;
    const char* build_metadata;
} idcu_ModuleVersion;

typedef struct idcu_ModuleDependency
{
    const char* name;
    idcu_ModuleVersion min_version;
    idcu_ModuleVersion max_version;
    bool optional;
} idcu_ModuleDependency;

typedef struct idcu_ModuleHandle idcu_ModuleHandle;

typedef int (*idcu_ModuleInitFunc)(idcu_ModuleHandle* handle);
typedef int (*idcu_ModuleStartFunc)(idcu_ModuleHandle* handle);
typedef int (*idcu_ModuleRunFunc)(idcu_ModuleHandle* handle);
typedef int (*idcu_ModuleStopFunc)(idcu_ModuleHandle* handle);
typedef void (*idcu_ModuleDestroyFunc)(idcu_ModuleHandle* handle);

typedef struct
{
    const char* name;
    const char* version_str;
    const char* description;
    const char* author;
    const char* license;
    idcu_ModuleCategory category;
    idcu_ModuleInitFunc init;
    idcu_ModuleStartFunc start;
    idcu_ModuleRunFunc run;
    idcu_ModuleStopFunc stop;
    idcu_ModuleDestroyFunc destroy;
    const idcu_ModuleDependency* dependencies;
    size_t dependency_count;
    uint32_t permissions;
} idcu_ModuleDef;

struct idcu_ModuleHandle
{
    const idcu_ModuleDef* def;
    idcu_ModuleState state;
    idcu_ModuleVersion version;
    uint32_t module_id;
    void* user_data;
    uint64_t start_time;
    uint64_t last_run_time;
    uint64_t run_count;
};

int idcu_module_def_init(idcu_ModuleDef* def, const char* name, const char* version, const char* description);
void idcu_module_def_destroy(idcu_ModuleDef* def);

int idcu_module_version_parse(idcu_ModuleVersion* ver, const char* version_str);
int idcu_module_version_compare(const idcu_ModuleVersion* a, const idcu_ModuleVersion* b);
bool idcu_module_version_satisfies(const idcu_ModuleVersion* ver, const idcu_ModuleDependency* dep);
void idcu_module_version_to_string(const idcu_ModuleVersion* ver, char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#define IDCU_MODULE_DEFINE(name, ver, desc, init_fn, start_fn, stop_fn, destroy_fn) \
    static idcu_ModuleDef g_##name##_module_def = { \
        .name = #name, \
        .version_str = ver, \
        .description = desc, \
        .init = init_fn, \
        .start = start_fn, \
        .stop = stop_fn, \
        .destroy = destroy_fn, \
    }; \
    const idcu_ModuleDef* idcu_get_##name##_module(void) { return &g_##name##_module_def; }

#endif
```

### 3. 创建模块注册表头文件 (module_registry.h)

创建 `libs/idcu-module-system/include/idcu/module/module_registry.h`：

```c
#ifndef IDCU_MODULE_MODULE_REGISTRY_H
#define IDCU_MODULE_MODULE_REGISTRY_H

#include "module_def.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    idcu_HashMap modules_by_name;
    idcu_HashMap modules_by_id;
    idcu_Vector modules;
    idcu_Mutex lock;
    uint32_t next_module_id;
    bool initialized;
} idcu_ModuleRegistry;

int idcu_module_registry_init(idcu_ModuleRegistry* registry);
void idcu_module_registry_destroy(idcu_ModuleRegistry* registry);

int idcu_module_registry_register(idcu_ModuleRegistry* registry, const idcu_ModuleDef* def);
int idcu_module_registry_unregister(idcu_ModuleRegistry* registry, const char* name);

const idcu_ModuleDef* idcu_module_registry_find_by_name(idcu_ModuleRegistry* registry, const char* name);
const idcu_ModuleDef* idcu_module_registry_find_by_id(idcu_ModuleRegistry* registry, uint32_t id);

size_t idcu_module_registry_count(idcu_ModuleRegistry* registry);
const idcu_ModuleDef* idcu_module_registry_get_by_index(idcu_ModuleRegistry* registry, size_t index);

int idcu_module_registry_get_by_category(idcu_ModuleRegistry* registry, idcu_ModuleCategory category,
                                           idcu_Vector* out_modules);

#ifdef __cplusplus
}
#endif

#endif
```

### 4. 创建模块加载器头文件 (dynamic_loader.h)

创建 `libs/idcu-module-system/include/idcu/module/dynamic_loader.h`：

```c
#ifndef IDCU_MODULE_DYNAMIC_LOADER_H
#define IDCU_MODULE_DYNAMIC_LOADER_H

#include "module_def.h"
#include "idcu/common/error_code.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct idcu_ModuleLoader idcu_ModuleLoader;

struct idcu_ModuleLoader
{
    void* handle;
    char path[512];
    const idcu_ModuleDef* (*get_module)(void);
    bool loaded;
};

int idcu_module_loader_init(idcu_ModuleLoader* loader);
void idcu_module_loader_destroy(idcu_ModuleLoader* loader);

int idcu_module_loader_load(idcu_ModuleLoader* loader, const char* path);
int idcu_module_loader_unload(idcu_ModuleLoader* loader);

const idcu_ModuleDef* idcu_module_loader_get_module(idcu_ModuleLoader* loader);

int idcu_module_loader_scan_directory(const char* dir_path, idcu_Vector* out_loaders);

#ifdef __cplusplus
}
#endif

#endif
```

### 5. 创建模块系统头文件 (module_system.h)

创建 `libs/idcu-module-system/include/idcu/module/module_system.h`：

```c
#ifndef IDCU_MODULE_MODULE_SYSTEM_H
#define IDCU_MODULE_MODULE_SYSTEM_H

#include "module_def.h"
#include "module_registry.h"
#include "idcu/common/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    idcu_ModuleRegistry registry;
    idcu_Vector handles;
    idcu_Vector load_order;
    bool initialized;
} idcu_ModuleSystem;

int idcu_module_system_init(idcu_ModuleSystem* system);
void idcu_module_system_destroy(idcu_ModuleSystem* system);

int idcu_module_system_register(idcu_ModuleSystem* system, const idcu_ModuleDef* def);
int idcu_module_system_load_directory(idcu_ModuleSystem* system, const char* dir_path);

int idcu_module_system_resolve_dependencies(idcu_ModuleSystem* system);
int idcu_module_system_sort_by_dependencies(idcu_ModuleSystem* system);

int idcu_module_system_init_all(idcu_ModuleSystem* system);
int idcu_module_system_start_all(idcu_ModuleSystem* system);
int idcu_module_system_run_all(idcu_ModuleSystem* system);
int idcu_module_system_stop_all(idcu_ModuleSystem* system);
int idcu_module_system_destroy_all(idcu_ModuleSystem* system);

idcu_ModuleHandle* idcu_module_system_get_handle(idcu_ModuleSystem* system, const char* name);
idcu_ModuleState idcu_module_system_get_state(idcu_ModuleSystem* system, const char* name);

#ifdef __cplusplus
}
#endif

#endif
```

### 6. 创建 CMakeLists.txt

创建 `libs/idcu-module-system/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-module-system VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-module-system STATIC
    src/idcu/module/module_def.c
    src/idcu/module/module_registry.c
    src/idcu/module/dynamic_loader.c
    src/idcu/module/module_system.c
    src/idcu/module/module_version.c
    src/idcu/module/module_category.c
)

target_include_directories(idcu-module-system PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-module-system PRIVATE
    idcu::common
)

if(WIN32)
else()
    target_link_libraries(idcu-module-system PRIVATE dl)
endif()

add_library(idcu::module-system ALIAS idcu-module-system)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 7. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-module-system/module.yaml`：

```yaml
name: idcu-module-system
version: 1.0.0
description: Module system for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common

build:
  type: cmake
  targets:
    - idcu-module-system

headers:
  - idcu/module/module_def.h
  - idcu/module/module_registry.h
  - idcu/module/dynamic_loader.h
  - idcu/module/module_system.h

features:
  - module_def: Module definition and lifecycle
  - module_registry: Module registry and lookup
  - dynamic_loader: Dynamic module loading
  - module_system: Complete module management system
  - dependency_resolution: Dependency resolution and ordering

testing:
  enabled: true
  framework: internal
```

### 8. 创建 README.md

创建 `libs/idcu-module-system/README.md`：

```markdown
# idcu-module-system

IDCU Agent 的模块系统。

## 功能特性

- **模块定义**: 完整的模块定义结构
- **生命周期管理**: 初始化、启动、运行、停止、销毁
- **依赖解析**: 自动解析和排序模块依赖
- **动态加载**: 支持动态加载模块
- **注册表**: 模块注册和查询

## 快速开始

### 定义模块

```c
#include "idcu/module/module_def.h"

static int my_module_init(idcu_ModuleHandle* handle) {
    idcu_module_set_user_data(handle, NULL);
    return IDCU_ERR_OK;
}

static int my_module_start(idcu_ModuleHandle* handle) {
    return IDCU_ERR_OK;
}

static int my_module_stop(idcu_ModuleHandle* handle) {
    return IDCU_ERR_OK;
}

static void my_module_destroy(idcu_ModuleHandle* handle) {
}

IDCU_MODULE_DEFINE(
    my_module,
    "1.0.0",
    "My example module",
    my_module_init,
    my_module_start,
    my_module_stop,
    my_module_destroy
);
```

### 使用模块系统

```c
#include "idcu/module/module_system.h"

idcu_ModuleSystem system;
idcu_module_system_init(&system);

idcu_module_system_register(&system, idcu_get_my_module_module());
idcu_module_system_resolve_dependencies(&system);
idcu_module_system_init_all(&system);
idcu_module_system_start_all(&system);

idcu_module_system_run_all(&system);

idcu_module_system_stop_all(&system);
idcu_module_system_destroy_all(&system);
idcu_module_system_destroy(&system);
```

## API 文档

详见 [include/idcu/module/](include/idcu/module/)
```

## 验证检查清单

- [ ] 所有头文件已创建
- [ ] 所有源文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以定义和注册模块
- [ ] 可以管理模块生命周期
- [ ] 依赖解析功能正常

## Git 提交

```bash
git add libs/idcu-module-system/
git commit -m "feat: add module system

- Add module definitions and lifecycle management
- Add module registry
- Add dynamic module loader
- Add dependency resolution
- Add complete module system"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 依赖循环 | 模块相互依赖 | 重新设计模块依赖关系 |
| 动态加载失败 | 找不到模块符号 | 确保模块导出了正确的符号 |
| 版本不兼容 | 版本要求不满足 | 检查模块版本依赖 |
