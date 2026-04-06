# 热加载模块示例

## 功能说明

本示例演示如何使用IDCU的动态模块加载功能，实现模块的热加载、卸载和重新加载，无需重启整个应用程序。

### 主要功能
- 动态模块加载/卸载
- 模块热重载（更新代码无需重启）
- 热插拔支持（运行时检测新模块）
- 模块状态管理
- 引用计数
- 跨平台支持（Windows DLL / Linux SO）

## 编译方式

### 编译示例程序

```bash
# 使用CMake编译
mkdir -p build && cd build
cmake ..
make hotload_module_example

# 或直接使用gcc
gcc -o hotload_module_example hotload_module_example.c \
    -I./modules/core/module-system/include \
    -I./libs/idcu-log/include \
    -I./libs/idcu-common/include \
    -L./build/modules -lidcu-module-system
```

### 编译动态模块示例

创建一个简单的可动态加载模块：

```c
// sample_module.c
#include "module_def.h"
#include <stdio.h>

static int sample_init() {
    printf("[sample_module] Initialized\n");
    return 0;
}

static int sample_run() {
    return 0;
}

static int sample_stop() {
    printf("[sample_module] Stopped\n");
    return 0;
}

IDCU_REGISTER_MODULE(sample_module, IDCU_MODULE_VERSION(1, 0, 0), 
                     sample_init, sample_run, sample_stop);
```

编译为动态库：

```bash
# Linux
gcc -shared -fPIC -o sample_module.so sample_module.c \
    -I./modules/core/module-system/include

# Windows
gcc -shared -o sample_module.dll sample_module.c \
    -I./modules/core/module-system/include
```

## 运行步骤

### 1. 准备模块目录

```bash
mkdir -p modules
cp sample_module.so modules/  # Linux
# 或
copy sample_module.dll modules\  # Windows
```

### 2. 运行示例程序

```bash
# Windows
hotload_module_example.exe

# Linux
./hotload_module_example
```

程序会演示完整的热加载流程。

### 3. 实际应用中的使用

在IDCU Agent中使用热加载：

```c
#include "dynamic_module.h"

idcu_DynamicLoader loader;
idcu_dynamic_loader_init(&loader, "./modules");

// 加载模块
idcu_dynamic_loader_load_module(&loader, "my_module", "./modules/my_module.so");

// 查找并初始化模块
idcu_DynamicModule* mod = idcu_dynamic_loader_find_module(&loader, "my_module");
if (mod) {
    idcu_dynamic_module_init(mod);
    idcu_dynamic_module_run(mod);
}

// 热重载模块（更新版本）
idcu_dynamic_module_reload(&loader, "my_module", "./modules/my_module_v2.so");

// 卸载模块
idcu_dynamic_loader_unload_module(&loader, "my_module");

idcu_dynamic_loader_destroy(&loader);
```

## 核心代码解读

### 1. 动态加载器初始化

```c
idcu_DynamicLoader loader;
int ret = idcu_dynamic_loader_init(&loader, MODULES_DIR);
```

- 创建动态加载器实例
- 指定模块搜索目录
- 初始化内部数据结构和锁

### 2. 加载模块

```c
int ret = idcu_dynamic_loader_load_module(&loader, "sample_module", "./sample_module.so");
```

- 加载动态库文件（.so或.dll）
- 查找模块导出符号
- 验证模块接口
- 初始化模块状态为LOADED

### 3. 模块生命周期管理

```c
idcu_DynamicModule* mod = idcu_dynamic_loader_find_module(&loader, "sample_module");
if (mod) {
    idcu_dynamic_module_init(mod);   // 调用模块init函数
    idcu_dynamic_module_run(mod);    // 调用模块run函数
    idcu_dynamic_module_stop(mod);   // 调用模块stop函数
}
```

- 按顺序调用模块的生命周期函数
- 跟踪模块状态转换

### 4. 模块状态枚举

模块有以下状态：

| 状态 | 描述 |
|------|------|
| `UNLOADED` | 未加载 |
| `LOADED` | 已加载，未初始化 |
| `INITIALIZED` | 已初始化，未运行 |
| `RUNNING` | 正在运行 |
| `STOPPED` | 已停止 |

### 5. 热重载模块

```c
int ret = idcu_dynamic_module_reload(&loader, "sample_module", "./sample_module_v2.so");
```

- 停止旧模块
- 卸载旧模块
- 加载新模块
- 保持模块名称和引用
- 自动初始化新模块

### 6. 热插拔操作

```c
// 热插拔加载（运行时检测新模块）
idcu_dynamic_loader_hotplug_load(&loader, "new_module", "./new_module.so");

// 热插拔卸载（运行时安全移除）
idcu_dynamic_loader_hotplug_unload(&loader, "old_module");
```

- 支持运行时动态添加/删除模块
- 确保线程安全
- 处理引用计数

## 模块导出要求

动态模块必须导出以下符号：

```c
// 必须使用这个宏来注册模块
IDCU_REGISTER_MODULE(
    module_name,
    IDCU_MODULE_VERSION(major, minor, patch),
    init_function,
    run_function,
    stop_function
);
```

这会导出：
- `module_name_interface`: 模块接口结构
- 生命周期函数指针

## 热加载最佳实践

1. **版本管理**: 始终使用版本号命名模块文件
2. **状态保存**: 模块卸载前保存状态，重载后恢复
3. **依赖管理**: 注意模块间的依赖关系
4. **错误处理**: 热加载失败时应有回退方案
5. **线程安全**: 确保模块代码是线程安全的
6. **资源清理**: 模块的stop函数应彻底清理资源

## 扩展建议

1. **模块监控**: 添加模块健康检查和性能监控
2. **自动发现**: 定期扫描模块目录，自动加载新模块
3. **模块沙箱**: 为动态模块提供安全沙箱环境
4. **远程部署**: 支持从远程服务器下载和加载模块
5. **回滚机制**: 热加载失败时自动回滚到之前版本

## 依赖关系

- `idcu-module-system`: 模块系统核心
- `idcu-common`: 通用工具和错误码
- `idcu-log`: 日志系统
- 平台动态库加载API（dlopen / LoadLibrary）
