# idcu-plugin

IDCU Agent 的插件系统库。

## 功能特性
- 跨平台动态插件加载（Windows/Linux/macOS）
- 插件信息查询
- 插件符号解析
- 插件目录扫描
- 批量插件操作
- 线程安全实现
- CMake 构建配置

## 使用方法

```c
#include <idcu/plugin/plugin.h>

idcu_PluginSystem* ctx;
int ret = idcu_plugin_system_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // 加载插件
    idcu_PluginHandle* plugin;
    ret = idcu_plugin_load(ctx, "myplugin.dll", &plugin);
    if (ret == IDCU_ERR_OK) {
        // 获取插件信息
        idcu_PluginInfo info;
        idcu_plugin_get_info(plugin, &info);
        
        // 获取符号
        void (*func)(void);
        if (idcu_plugin_get_symbol(plugin, "my_function", (void**)&func) == IDCU_ERR_OK) {
            func();
        }
        
        // 卸载插件
        idcu_plugin_unload(ctx, plugin);
    }
    
    // 关闭
    idcu_plugin_system_shutdown(ctx);
}
```

## API 参考
- `idcu_plugin_system_init()` - 初始化插件系统
- `idcu_plugin_system_shutdown()` - 关闭插件系统
- `idcu_plugin_load()` - 从文件加载插件
- `idcu_plugin_unload()` - 卸载插件
- `idcu_plugin_get_info()` - 获取插件信息
- `idcu_plugin_get_name()` - 获取插件名称
- `idcu_plugin_is_loaded()` - 检查插件是否已加载
- `idcu_plugin_scan_directory()` - 扫描目录查找插件
- `idcu_plugin_load_all()` - 加载所有扫描到的插件
- `idcu_plugin_unload_all()` - 卸载所有插件
- `idcu_plugin_get_count()` - 获取已加载插件数量
- `idcu_plugin_get_all_names()` - 获取所有已加载插件名称
- `idcu_plugin_get_symbol()` - 从插件获取符号

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
