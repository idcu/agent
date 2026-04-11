# 模块系统设计

本文档描述了 IDCU Agent 模块系统的设计。

## 概述

模块系统为动态组件加载和生命周期管理提供了一个框架。

## 核心概念

### 模块定义

一个模块由 `idcu_ModuleDef` 结构定义：

```c
typedef struct idcu_ModuleDef {
    const char* name;           // 唯一模块标识符
    const char* version_str;    // 语义版本（例如，"1.0.0"）
    const char* description;    // 人类可读的描述
    const char* category;       // 模块类别

    // 生命周期回调
    idcu_ErrorCode (*init)(void);
    idcu_ErrorCode (*start)(void);
    idcu_ErrorCode (*stop)(void);
    idcu_ErrorCode (*destroy)(void);

    // 依赖项
    const char** dependencies;
    size_t dependency_count;
} idcu_ModuleDef;
```

### 模块生命周期

```
未注册
    ↓ (注册)
已注册
    ↓ (初始化)
已初始化
    ↓ (启动)
已启动 ←──┐
    ↓ (停止)  │
已停止 ─────┘
    ↓ (销毁)
已销毁
```

### 状态转换

| 从状态       | 到状态       | 触发器           |
|--------------|--------------|-------------------|
| 未注册       | 已注册       | `register()`      |
| 已注册       | 已初始化     | `init_module()`   |
| 已初始化     | 已启动       | `start_module()`  |
| 已启动       | 已停止       | `stop_module()`   |
| 已停止       | 已启动       | `start_module()`  |
| 已停止       | 已销毁       | `destroy_module()`|

## 依赖关系

### 依赖声明

模块可以声明对其他模块的依赖：

```c
static const char* my_deps[] = {"module-a", "module-b"};

static const idcu_ModuleDef my_module = {
    .name = "my-module",
    .version_str = "1.0.0",
    .dependencies = my_deps,
    .dependency_count = 2,
    // ...
};
```

### 依赖解析

模块系统确保：
1. 依赖项在依赖模块之前初始化
2. 依赖项在依赖模块之前启动
3. 依赖模块在依赖项之前停止
4. 依赖模块在依赖项之前销毁

### 循环依赖

循环依赖在注册期间被检测并拒绝。

## 模块系统 API

### 注册

```c
// 注册一个模块
int idcu_module_system_register(idcu_ModuleSystem* system,
                                 const idcu_ModuleDef* module_def);

// 注销一个模块
int idcu_module_system_unregister(idcu_ModuleSystem* system,
                                   const char* name);
```

### 生命周期管理

```c
// 单个模块操作
int idcu_module_system_init_module(idcu_ModuleSystem* system,
                                    const char* name);
int idcu_module_system_start_module(idcu_ModuleSystem* system,
                                     const char* name);
int idcu_module_system_stop_module(idcu_ModuleSystem* system,
                                    const char* name);
int idcu_module_system_destroy_module(idcu_ModuleSystem* system,
                                       const char* name);

// 批量操作
int idcu_module_system_init_all(idcu_ModuleSystem* system);
int idcu_module_system_start_all(idcu_ModuleSystem* system);
int idcu_module_system_stop_all(idcu_ModuleSystem* system);
int idcu_module_system_destroy_all(idcu_ModuleSystem* system);
```

### 查询

```c
// 按名称查找模块
int idcu_module_system_find_by_name(idcu_ModuleSystem* system,
                                     const char* name,
                                     idcu_Module** out_module);

// 获取所有模块
int idcu_module_system_get_all(idcu_ModuleSystem* system,
                                idcu_ModuleInfo** out_infos,
                                size_t* out_count);

// 按类别获取模块
int idcu_module_system_get_by_category(idcu_ModuleSystem* system,
                                         const char* category,
                                         idcu_ModuleInfo** out_infos,
                                         size_t* out_count);
```

## 模块类别

模块应按类别进行组织：

- `core` - 核心系统模块
- `network` - 网络相关模块
- `storage` - 存储和持久化模块
- `monitoring` - 指标和健康检查模块
- `api` - API 和接口模块
- `business` - 业务逻辑模块
- `integration` - 集成模块

## 示例模块

```c
#include <idcu/module/module.h>
#include <idcu/log/log.h>

static idcu_ErrorCode my_module_init(void) {
    IDCU_LOG_INFO("我的模块正在初始化...");
    return IDCU_ERR_OK;
}

static idcu_ErrorCode my_module_start(void) {
    IDCU_LOG_INFO("我的模块正在启动...");
    return IDCU_ERR_OK;
}

static idcu_ErrorCode my_module_stop(void) {
    IDCU_LOG_INFO("我的模块正在停止...");
    return IDCU_ERR_OK;
}

static idcu_ErrorCode my_module_destroy(void) {
    IDCU_LOG_INFO("我的模块正在销毁...");
    return IDCU_ERR_OK;
}

static const char* dependencies[] = {"core-module"};

const idcu_ModuleDef my_module = {
    .name = "my-module",
    .version_str = "1.0.0",
    .description = "我的示例模块",
    .category = "business",
    .init = my_module_init,
    .start = my_module_start,
    .stop = my_module_stop,
    .destroy = my_module_destroy,
    .dependencies = dependencies,
    .dependency_count = 1
};
```

## 最佳实践

1. **保持模块小而专注** - 单一职责原则
2. **声明所有依赖项** - 显式优于隐式
3. **优雅地处理错误** - 返回适当的错误码
4. **记录状态转换** - 有助于调试
5. **在销毁时清理** - 释放所有资源
6. **保持幂等** - 处理重复的 init/destroy 调用
