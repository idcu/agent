# idcu-plugin

Plugin system library for IDCU Agent.

## Features
- Cross-platform dynamic plugin loading (Windows/Linux/macOS)
- Plugin information query
- Plugin symbol resolution
- Plugin directory scanning
- Batch plugin operations
- Thread-safe implementation
- CMake build configuration

## Usage

```c
#include <idcu/plugin/plugin.h>

idcu_Plugin_Context* ctx;
int ret = idcu_plugin_system_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Load plugin
    idcu_PluginHandle* plugin;
    ret = idcu_plugin_load(ctx, "myplugin.dll", &plugin);
    if (ret == IDCU_ERR_OK) {
        // Get plugin info
        idcu_PluginInfo info;
        idcu_plugin_get_info(plugin, &info);
        
        // Get symbol
        void (*func)(void);
        if (idcu_plugin_get_symbol(plugin, "my_function", (void**)&func) == IDCU_ERR_OK) {
            func();
        }
        
        // Unload plugin
        idcu_plugin_unload(ctx, plugin);
    }
    
    // Shutdown
    idcu_plugin_system_shutdown(ctx);
}
```

## API Reference
- `idcu_plugin_system_init()` - Initialize plugin system
- `idcu_plugin_system_shutdown()` - Shutdown plugin system
- `idcu_plugin_load()` - Load a plugin from file
- `idcu_plugin_unload()` - Unload a plugin
- `idcu_plugin_get_info()` - Get plugin information
- `idcu_plugin_get_name()` - Get plugin name
- `idcu_plugin_is_loaded()` - Check if plugin is loaded
- `idcu_plugin_scan_directory()` - Scan directory for plugins
- `idcu_plugin_load_all()` - Load all scanned plugins
- `idcu_plugin_unload_all()` - Unload all plugins
- `idcu_plugin_get_count()` - Get loaded plugin count
- `idcu_plugin_get_all_names()` - Get all loaded plugin names
- `idcu_plugin_get_symbol()` - Get symbol from plugin

## Building

```bash
cmake -B build && cmake --build build
```

## License
MIT
