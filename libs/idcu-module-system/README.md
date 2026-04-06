# idcu-module-system

IDCU 项目的模块管理系统库，提供动态模块加载、启动、停止等功能。

## 特性

- 模块注册与管理
- 动态加载/卸载模块
- 模块启动/停止/重启
- 模块生命周期管理
- 模块系统状态保存/恢复
- 跨平台支持（Windows、Linux、macOS）

## 快速开始

### 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 安装

```bash
cmake --install .
```

### 在其他项目中使用

```cmake
find_package(idcu-module-system REQUIRED)
add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE idcu::module-system)
```

## API 文档

### 系统初始化和关闭

```c
#include <idcu/module_system/module_system.h>

int idcu_module_system_init(void);
void idcu_module_system_shutdown(void);
```

### 模块注册和卸载

```c
int idcu_module_register(const char* name, const idcu_ModuleInfo* info,
                         idcu_ModuleInitFunc init, idcu_ModuleCleanupFunc cleanup,
                         idcu_ModuleUpdateFunc update);
int idcu_module_unregister(const char* name);
```

### 模块加载和卸载

```c
int idcu_module_load(const char* name, idcu_ModuleHandle** handle);
int idcu_module_unload(idcu_ModuleHandle* handle);
```

### 模块控制

```c
int idcu_module_start(idcu_ModuleHandle* handle);
int idcu_module_stop(idcu_ModuleHandle* handle);
int idcu_module_restart(idcu_ModuleHandle* handle);
```

### 模块查询

```c
int idcu_module_is_loaded(const char* name);
int idcu_module_is_running(const char* name);
int idcu_module_get_info(const char* name, idcu_ModuleInfo* info);
int idcu_module_get_count(void);
int idcu_module_get_all_names(char** names, size_t max_names, size_t* actual_count);
```

### 系统操作

```c
int idcu_module_system_update(uint64_t delta_ms);
int idcu_module_system_save_state(const char* file_path);
int idcu_module_system_load_state(const char* file_path);
```

## 测试

```bash
cd build
ctest
```

## 许可证

详见项目根目录的 LICENSE 文件。
