# idcu-plugin

IDCU 项目的插件生态系统库，提供动态插件加载和管理功能。

## 特性

- 动态插件加载/卸载
- 跨平台支持（Windows DLL、Linux/macOS SO）
- 插件信息查询
- 插件符号解析
- 插件目录扫描
- 插件生命周期管理

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
find_package(idcu-plugin REQUIRED)
add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE idcu::plugin)
```

## API 文档

### 系统初始化和关闭

```c
#include <idcu/plugin/plugin.h>

int idcu_plugin_system_init(void);
void idcu_plugin_system_shutdown(void);
```

### 插件加载和卸载

```c
int idcu_plugin_load(const char* path, idcu_PluginHandle** handle);
int idcu_plugin_unload(idcu_PluginHandle* handle);
```

### 插件信息查询

```c
int idcu_plugin_get_info(idcu_PluginHandle* handle, idcu_PluginInfo* info);
int idcu_plugin_get_name(idcu_PluginHandle* handle, char* buffer, size_t buffer_size);
int idcu_plugin_is_loaded(const char* name);
```

### 插件批量操作

```c
int idcu_plugin_scan_directory(const char* directory);
int idcu_plugin_load_all(void);
int idcu_plugin_unload_all(void);
```

### 插件枚举

```c
int idcu_plugin_get_count(void);
int idcu_plugin_get_all_names(char** names, size_t max_names, size_t* actual_count);
```

### 符号解析

```c
int idcu_plugin_get_symbol(idcu_PluginHandle* handle, const char* symbol_name, void** symbol);
```

## 测试

```bash
cd build
ctest
```

## 许可证

详见项目根目录的 LICENSE 文件。
