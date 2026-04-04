# 模块化积木式架构设计指南

## 概述

IDCU Agent 2.0 采用**配置驱动的模块化积木式架构**，核心思想是：

1. **核心极简** - 微内核只包含最基本的调度和通信功能
2. **积木式组装** - 所有功能通过模块实现，像搭积木一样组合
3. **分类分级** - 模块按功能类别和优先级分级管理
4. **配置驱动** - 通过配置文件启用/禁用模块，无需修改代码
5. **易于扩展** - 新增模块只需添加文件和配置，不影响现有代码

---

## 架构分层

```
┌─────────────────────────────────────────────────────────┐
│  Level 4: 扩展层 (Extension)       可选扩展功能        │
├─────────────────────────────────────────────────────────┤
│  Level 3: 业务层 (Business)        核心业务功能        │
├─────────────────────────────────────────────────────────┤
│  Level 2: 服务层 (Service)         监控/网络/安全      │
├─────────────────────────────────────────────────────────┤
│  Level 1: 基础层 (Foundation)      基础服务组件        │
├─────────────────────────────────────────────────────────┤
│  Level 0: 核心层 (Core)            核心基础模块        │
├─────────────────────────────────────────────────────────┤
│  微内核 (Micro Kernel)         调度/通信/模块管理      │
└─────────────────────────────────────────────────────────┘
```

### 模块分类说明

| 层级 | 分类 | 说明 | 示例 |
|------|------|------|------|
| Level 0 | core | 核心基础模块，系统启动必需 | base_log, core_module |
| Level 1 | foundation | 基础服务模块，提供通用能力 | config_module, log_module |
| Level 2 | service | 服务层模块，提供特定服务 | monitoring, networking, security |
| Level 3 | business | 业务功能模块，实现核心业务 | biz_collect, heartbeat_module |
| Level 4 | extension | 扩展功能模块，可选增强功能 | custom_plugin, third_party |

---

## 配置文件详解

配置文件位于 `config/agent.cfg`，采用 INI 格式。

### 1. 模块分类配置

在 `[modules]`  section 下按分类配置模块：

```ini
[modules]
# Level 0: 核心模块
core = base_log, core_module

# Level 1: 基础服务模块
foundation = config_module

# Level 2: 服务层模块
monitoring = healthcheck_module, metrics_module
networking = http_server_module
security = sandbox_module

# Level 3: 业务模块
business = biz_collect, heartbeat_module, alert_module

# Level 4: 扩展模块
extension = custom_extension
```

### 2. 模块启用/禁用

通过 `enable_<模块名>` 配置项控制模块是否启用：

```ini
# 启用核心模块
enable_base_log = true
enable_core_module = true

# 可选业务模块
enable_biz_collect = true
enable_heartbeat_module = true
enable_alert_module = false  # 禁用告警模块
```

### 3. 模块优先级配置

通过 `<模块名>.priority` 配置模块优先级：

```ini
base_log.priority = high
core_module.priority = realtime
biz_collect.priority = normal
heartbeat_module.priority = high
```

优先级选项：
- `low` - 低优先级
- `normal` - 普通优先级（默认）
- `high` - 高优先级
- `realtime` - 实时优先级

### 4. 模块特定配置

每个模块可以有自己的配置 section，格式为 `[module.<模块名>]`：

```ini
[module.heartbeat_module]
interval_ms = 5000
timeout_ms = 15000

[module.metrics_module]
collection_interval_ms = 5000
export_enabled = true
```

在模块代码中通过配置前缀读取：

```c
static int heartbeat_module_init()
{
    const char* config_prefix = "module.heartbeat_module";
    int interval = idcu_config_get_int(config_prefix, "interval_ms", 5000);
    // ...
}
```

---

## 开发新模块指南

### 步骤 1: 创建模块文件

在 `modules/` 目录下对应的分类文件夹中创建模块文件：

```
modules/
├── base/          # 核心基础模块
├── biz/           # 业务功能模块
├── dynamic/       # 动态加载模块
└── sdk_examples/  # SDK示例
```

### 步骤 2: 实现模块接口

模块必须实现 `idcu_ModuleInterface` 接口：

```c
#include "module/module_def.h"
#include "utils/config_manager.h"
#include "utils/log.h"

// 模块内部状态
static int g_my_counter = 0;

// 初始化函数
static int my_module_init()
{
    // 读取模块配置
    const char* config_prefix = "module.my_module";
    int config_value = idcu_config_get_int(config_prefix, "my_config", 100);
    
    IDCU_LOG_INFO("my_module initialized (config: %d)", config_value);
    return IDCU_ERR_SUCCESS;
}

// 运行函数（主循环中调用）
static int my_module_run()
{
    g_my_counter++;
    if (g_my_counter % 1000000 == 0) {
        IDCU_LOG_INFO("my_module running (counter: %d)", g_my_counter);
    }
    return IDCU_ERR_SUCCESS;
}

// 停止函数
static int my_module_stop()
{
    IDCU_LOG_INFO("my_module stopped (counter: %d)", g_my_counter);
    return IDCU_ERR_SUCCESS;
}

// 注册模块
IDCU_REGISTER_MODULE(my_module, IDCU_MODULE_VERSION(1, 0, 0), 
                     my_module_init, my_module_run, my_module_stop);
```

### 步骤 3: 在配置文件中添加模块

在 `config/agent.cfg` 中：

1. 在对应分类下添加模块名
2. 配置启用状态
3. （可选）配置优先级
4. （可选）添加模块特定配置

```ini
[modules]
business = ..., my_module

enable_my_module = true
my_module.priority = normal

[module.my_module]
my_config = 200
```

### 步骤 4: 更新模块发现（仅 Windows/MSVC）

在 `src/module/module_registry.c` 中添加模块声明：

```c
extern const idcu_ModuleInterface __idcu_module_my_module;
```

在 `idcu_module_registry_discover_modules()` 函数中添加注册：

```c
ret = idcu_module_registry_register(registry, &__idcu_module_my_module, IDCU_MOD_PRIO_NORMAL);
```

**注意**：在 GCC/Linux 下通过 section 自动发现，无需此步骤。

---

## 核心组件说明

### 1. 模块分类管理器 (`module_category.h/c`)

负责：
- 管理模块分类和层级
- 加载和解析模块配置
- 提供模块启用状态查询
- 按层级获取模块列表

主要 API：
```c
// 初始化分类管理器
int idcu_module_category_manager_init(idcu_ModuleCategoryManager* mgr);

// 从配置文件加载模块配置
int idcu_module_category_load_config(idcu_ModuleCategoryManager* mgr, const char* config_file);

// 检查模块是否启用
int idcu_module_category_is_module_enabled(idcu_ModuleCategoryManager* mgr, const char* module_name);

// 获取模块优先级
idcu_ModulePrio idcu_module_category_get_module_priority(idcu_ModuleCategoryManager* mgr, const char* module_name);
```

### 2. 模块注册表 (`module_registry.h/c`)

负责：
- 模块的注册和管理
- 依赖关系图构建
- 拓扑排序（按依赖关系启动）
- 配置应用

主要 API：
```c
// 加载配置文件
int idcu_module_registry_load_config(idcu_ModuleRegistry* registry, const char* config_file);

// 应用配置到已注册模块
int idcu_module_registry_apply_config(idcu_ModuleRegistry* registry);

// 初始化所有启用的模块
int idcu_module_registry_init_all(idcu_ModuleRegistry* registry);

// 运行所有启用的模块
int idcu_module_registry_run_all(idcu_ModuleRegistry* registry);
```

### 3. 微内核 (`micro_kernel.h/c`)

负责：
- 协程调度
- 消息总线
- 沙箱安全
- 模块生命周期管理

新增 API：
```c
// 设置配置文件路径
int idcu_kernel_set_config_file(idcu_MicroKernel *k, const char* config_file);

// 加载并应用配置
int idcu_kernel_load_config(idcu_MicroKernel *k);
```

---

## 最佳实践

### 1. 模块设计原则

- **单一职责** - 每个模块只负责一个功能
- **松耦合** - 模块间通过消息总线通信，不直接依赖
- **可配置** - 关键参数通过配置文件设置
- **无状态** - 尽量避免全局状态，或通过配置管理

### 2. 配置管理

- 为模块提供合理的默认值
- 使用 `module.<模块名>` 作为配置前缀
- 在模块初始化时读取配置
- 重要配置变更需要重启模块

### 3. 错误处理

- 初始化失败返回错误码，内核会记录日志
- 运行时错误尽量恢复，不要轻易崩溃
- 使用 `IDCU_LOG_ERROR/WARN/INFO` 记录日志

### 4. 依赖管理

- 尽量减少模块间依赖
- 如必须依赖，在 `IDCU_REGISTER_MODULE_WITH_DEPS` 中声明
- 依赖的模块必须在更高优先级层级

---

## 迁移指南（从 1.0 到 2.0）

### 1. 现有模块兼容

现有模块无需修改即可继续工作，默认全部启用。

### 2. 迁移步骤

1. 更新配置文件到新格式
2. 在分类配置中添加现有模块
3. 按需要设置 `enable_*` 和优先级
4. （可选）添加模块特定配置

### 3. 配置示例

```ini
# 旧配置（兼容）
enable_base_log = true
enable_core_module = true

# 新配置（推荐）
[modules]
core = base_log, core_module
business = biz_collect

base_log.priority = high
core_module.priority = realtime
```

---

## 常见问题

### Q: 新增模块需要重新编译吗？

A: 是的，静态模块需要重新编译。如果需要动态加载，可以使用动态模块功能。

### Q: 如何在运行时启用/禁用模块？

A: 当前版本需要修改配置文件并重启程序。未来版本计划支持热插拔。

### Q: 模块间如何通信？

A: 通过消息总线 (`msg_bus.h`) 发送和接收消息。

### Q: 如何调试模块？

A: 查看日志文件，或使用 `IDCU_LOG_DEBUG` 输出调试信息。

---

## 总结

新的模块化积木式架构提供了：

✅ **配置驱动** - 无需修改代码即可调整模块组合  
✅ **分类分级** - 清晰的模块层级和职责划分  
✅ **易于扩展** - 新增模块只需添加文件和配置  
✅ **向后兼容** - 现有模块无需修改即可使用  
✅ **灵活组合** - 像搭积木一样组装功能  

通过这种架构，你可以：
- 快速原型开发 - 只启用需要的模块
- 生产环境部署 - 按需组合功能模块
- 功能定制 - 通过配置调整系统行为
- 团队协作 - 不同模块可独立开发和测试
