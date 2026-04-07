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

## 使用说明

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

### 运行示例程序

```bash
# Windows
hotload_module_example.exe

# Linux
./hotload_module_example
```

程序会演示完整的热加载流程。

## 模块状态

模块有以下状态：

| 状态 | 描述 |
|------|------|
| `UNLOADED` | 未加载 |
| `LOADED` | 已加载，未初始化 |
| `INITIALIZED` | 已初始化，未运行 |
| `RUNNING` | 正在运行 |
| `STOPPED` | 已停止 |

## 热加载最佳实践

1. **版本管理**: 始终使用版本号命名模块文件
2. **状态保存**: 模块卸载前保存状态，重载后恢复
3. **依赖管理**: 注意模块间的依赖关系
4. **错误处理**: 热加载失败时应有回退方案
5. **线程安全**: 确保模块代码是线程安全的
6. **资源清理**: 模块的stop函数应彻底清理资源

## 依赖关系

- `idcu-module-system`: 模块系统核心
- `idcu-common`: 通用工具和错误码
- `idcu-log`: 日志系统
- 平台动态库加载API（dlopen / LoadLibrary）
