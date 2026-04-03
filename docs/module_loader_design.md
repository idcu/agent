# 模块加载机制重新设计

## 概述

本文档描述了 idcu 项目中模块加载机制的重新设计。新的设计提供了更完善的模块生命周期管理、依赖关系处理和事件通知机制。

---

## 设计目标

1. **更完善的生命周期管理** - 提供完整的模块初始化、运行、停止和卸载流程
2. **依赖关系管理** - 支持模块之间的依赖关系声明和检查
3. **版本管理** - 支持模块版本号和版本兼容性检查
4. **事件通知** - 提供模块生命周期事件的回调机制
5. **错误处理** - 更好的错误追踪和报告机制
6. **优先级调度** - 基于优先级的模块启动顺序

---

## 新模块管理器架构

### 核心组件

#### 1. idcu_ModuleInfo - 模块信息

```c
typedef struct {
    char name[IDCU_MODULE_NAME_MAX];
    char version[IDCU_MODULE_VERSION_MAX];
    char description[IDCU_MODULE_DESCRIPTION_MAX];
    idcu_ModuleInterface* iface;
    idcu_ModuleState state;
    idcu_ModulePrio priority;
    uint32_t module_id;
    uint32_t ref_count;
    uint64_t created_time;
    uint64_t last_error_time;
    int last_error_code;
    char last_error_message[256];
    idcu_ModuleDependency dependencies[IDCU_MAX_DEPENDENCIES];
    int dependency_count;
    idcu_ModuleEventHandler event_handler;
    void* event_handler_user_data;
    void* user_data;
} idcu_ModuleInfo;
```

#### 2. idcu_ModuleManager - 模块管理器

```c
typedef struct {
    idcu_ModuleInfo modules[IDCU_MAX_MODULES];
    uint32_t count;
    uint32_t next_id;
    idcu_Mutex lock;
    int initialized;
    int running;
} idcu_ModuleManager;
```

---

## 模块生命周期

### 状态定义

```c
typedef enum {
    IDCU_MOD_STATE_UNINIT   = 0,  // 未初始化
    IDCU_MOD_STATE_INITED   = 1,  // 已初始化
    IDCU_MOD_STATE_RUNNING  = 2,  // 运行中
    IDCU_MOD_STATE_STOPPED  = 3,  // 已停止
    IDCU_MOD_STATE_ERROR    = 4   // 错误状态
} idcu_ModuleState;
```

### 生命周期流程

1. **注册** - `idcu_module_register()`
2. **初始化** - `idcu_module_init()`
3. **运行** - `idcu_module_run()`
4. **停止** - `idcu_module_stop()`
5. **重启** - `idcu_module_restart()`（可选）
6. **卸载** - `idcu_module_unregister()`

---

## 事件通知机制

### 事件类型

```c
typedef enum {
    IDCU_MODULE_EVENT_INIT_START = 0,    // 初始化开始
    IDCU_MODULE_EVENT_INIT_DONE,           // 初始化完成
    IDCU_MODULE_EVENT_INIT_FAIL,           // 初始化失败
    IDCU_MODULE_EVENT_START,                // 模块启动
    IDCU_MODULE_EVENT_STOP,                 // 模块停止
    IDCU_MODULE_EVENT_ERROR,                // 模块错误
    IDCU_MODULE_EVENT_UNLOAD                // 模块卸载
} idcu_ModuleEvent;
```

### 事件处理器

```c
typedef void (*idcu_ModuleEventHandler)(uint32_t module_id, idcu_ModuleEvent event, void* user_data);
```

使用示例：

```c
void my_event_handler(uint32_t module_id, idcu_ModuleEvent event, void* user_data) {
    switch (event) {
        case IDCU_MODULE_EVENT_INIT_DONE:
            printf("Module %d initialized\n", module_id);
            break;
        case IDCU_MODULE_EVENT_ERROR:
            printf("Module %d error occurred\n", module_id);
            break;
        default:
            break;
    }
}

idcu_module_set_event_handler(manager, module_id, my_event_handler, NULL);
```

---

## 依赖关系管理

### 依赖声明

```c
typedef struct {
    char dependency_name[IDCU_MODULE_NAME_MAX];
    char required_version[IDCU_MODULE_VERSION_MAX];
} idcu_ModuleDependency;
```

### 注册时指定依赖

```c
idcu_ModuleDependency deps[] = {
    {"base_module", "1.0.0"},
    {"utils_module", "2.1.0"}
};

idcu_module_register_with_deps(manager, "my_module", "1.0.0", "My module", 
                              &iface, IDCU_MOD_PRIO_NORMAL, deps, 2);
```

### 运行时添加依赖

```c
idcu_module_add_dependency(manager, my_module_id, "dependency_module", "1.5.0");
```

### 依赖检查

```c
int result = idcu_module_check_dependencies(manager, module_id);
if (result != IDCU_ERR_SUCCESS) {
    printf("Dependencies not satisfied\n");
}
```

---

## 版本管理

### 版本格式

使用语义化版本格式：`major.minor.patch`

例如：`1.0.0`, `2.1.3`

### 版本解析

```c
uint32_t major, minor, patch;
idcu_module_version_parse("1.2.3", &major, &minor, &patch);
// major=1, minor=2, patch=3
```

### 版本比较

```c
int result = idcu_module_version_compare("1.2.0", "1.1.5");
// result > 0 表示 "1.2.0" > "1.1.5"
```

### 版本兼容性检查

```c
if (idcu_module_version_satisfies(">=1.0.0", "1.2.3")) {
    // 版本兼容
}
```

---

## API 参考

### 管理器操作

| 函数 | 描述 |
|------|------|
| `idcu_module_manager_init()` | 初始化模块管理器 |
| `idcu_module_manager_destroy()` | 销毁模块管理器 |

### 模块注册

| 函数 | 描述 |
|------|------|
| `idcu_module_register()` | 注册模块 |
| `idcu_module_register_with_deps()` | 注册模块并指定依赖 |
| `idcu_module_unregister()` | 注销模块（按ID） |
| `idcu_module_unregister_by_name()` | 注销模块（按名称） |

### 模块查询

| 函数 | 描述 |
|------|------|
| `idcu_module_find_by_id()` | 按ID查找模块 |
| `idcu_module_find_by_name()` | 按名称查找模块 |
| `idcu_module_get_count()` | 获取模块数量 |
| `idcu_module_get_at()` | 按索引获取模块 |

### 模块生命周期

| 函数 | 描述 |
|------|------|
| `idcu_module_init()` | 初始化单个模块 |
| `idcu_module_run()` | 运行单个模块 |
| `idcu_module_stop()` | 停止单个模块 |
| `idcu_module_restart()` | 重启单个模块 |
| `idcu_module_init_all()` | 初始化所有模块 |
| `idcu_module_run_all()` | 运行所有模块 |
| `idcu_module_stop_all()` | 停止所有模块 |
| `idcu_module_shutdown()` | 完全关闭 |

### 依赖管理

| 函数 | 描述 |
|------|------|
| `idcu_module_add_dependency()` | 添加依赖 |
| `idcu_module_check_dependencies()` | 检查依赖 |

### 事件处理

| 函数 | 描述 |
|------|------|
| `idcu_module_set_event_handler()` | 设置事件处理器 |

### 状态和错误

| 函数 | 描述 |
|------|------|
| `idcu_module_get_state()` | 获取模块状态 |
| `idcu_module_get_error()` | 获取错误信息 |
| `idcu_module_clear_error()` | 清除错误 |

### 版本工具

| 函数 | 描述 |
|------|------|
| `idcu_module_version_parse()` | 解析版本号 |
| `idcu_module_version_compare()` | 比较版本号 |
| `idcu_module_version_satisfies()` | 检查版本兼容性 |

---

## 使用示例

### 基本用法

```c
#include "module/module_manager.h"

// 1. 初始化管理器
idcu_ModuleManager manager;
idcu_module_manager_init(&manager);

// 2. 定义模块接口
idcu_ModuleInterface my_module_iface = {
    .name = "my_module",
    .init = my_module_init,
    .run = my_module_run,
    .stop = my_module_stop
};

// 3. 注册模块
uint32_t module_id;
idcu_module_register(&manager, "my_module", "1.0.0", 
                     "My awesome module", &my_module_iface, 
                     IDCU_MOD_PRIO_NORMAL);

// 4. 初始化并运行所有模块
idcu_module_init_all(&manager);
idcu_module_run_all(&manager);

// 5. 停止并关闭
idcu_module_stop_all(&manager);
idcu_module_shutdown(&manager);
idcu_module_manager_destroy(&manager);
```

### 带依赖的用法

```c
// 定义依赖
idcu_ModuleDependency deps[] = {
    {"core", "1.0.0"},
    {"network", "2.0.0"}
};

// 注册带依赖的模块
idcu_module_register_with_deps(&manager, "my_app", "1.0.0", 
                               "My application", &app_iface, 
                               IDCU_MOD_PRIO_NORMAL, deps, 2);
```

---

## 与旧系统的对比

### 旧系统特点

- 简单的模块注册机制
- 基本的生命周期管理
- 无依赖关系管理
- 无事件通知
- 基本的错误处理

### 新系统特点

- ✅ 完善的模块生命周期管理
- ✅ 依赖关系管理
- ✅ 版本管理和兼容性检查
- ✅ 事件通知机制
- ✅ 详细的错误追踪
- ✅ 优先级调度
- ✅ 更好的线程安全

---

## 迁移指南

### 从旧系统迁移

1. **包含新头文件**
   ```c
   #include "module/module_manager.h"
   ```

2. **使用新的注册函数**
   ```c
   // 旧
   IDCU_REGISTER_MODULE(name, init, run, stop);
   
   // 新
   idcu_module_register(&manager, name, "1.0.0", "Description", &iface, IDCU_MOD_PRIO_NORMAL);
   ```

3. **使用新的生命周期函数**
   ```c
   // 旧
   module_registry_init_all(registry);
   
   // 新
   idcu_module_init_all(&manager);
   ```

---

## 总结

新的模块加载机制提供了：

1. **更完善的功能** - 依赖管理、版本控制、事件通知
2. **更好的可靠性** - 详细的错误追踪和状态管理
3. **更强的扩展性** - 易于添加新功能
4. **更好的开发体验** - 清晰的 API 和事件驱动设计

该设计保留了旧系统的核心功能，同时提供了更强大和灵活的模块管理能力。
