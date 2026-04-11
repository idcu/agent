# idcu-config API Documentation

Configuration management library.

## Quick Start

```c
#include <idcu/config/config.h>

int main() {
    idcu_config_init("config.ini");
    
    const char* value = idcu_config_get_string("database", "host", "localhost");
    printf("Database host: %s\n", value);
    
    idcu_config_set_int("app", "port", 8080);
    idcu_config_save("config.ini");
    
    idcu_config_shutdown();
    return 0;
}
```

## Initialization

### Initialize Config

```c
int idcu_config_init(const char* file_path);
```

Initialize a configuration manager, loading from a file.

- `file_path`: Path to config file

### Shutdown

```c
void idcu_config_shutdown(void);
```

Shutdown the configuration manager.

### Is Loaded

```c
int idcu_config_is_loaded(void);
```

Check if config is loaded.

### Reload

```c
int idcu_config_reload(void);
```

Reload the config file.

### Save

```c
int idcu_config_save(const char* file_path);
```

Save configuration to a file.

## Getting Values

### Get String

```c
const char* idcu_config_get_string(const char* section, const char* key, const char* default_value);
```

Get a string value.

### Get Integer

```c
int idcu_config_get_int(const char* section, const char* key, int default_value);
```

Get an integer value.

### Get Int64

```c
int64_t idcu_config_get_int64(const char* section, const char* key, int64_t default_value);
```

Get a 64-bit integer value.

### Get Double

```c
double idcu_config_get_double(const char* section, const char* key, double default_value);
```

Get a double value.

### Get Boolean

```c
int idcu_config_get_bool(const char* section, const char* key, int default_value);
```

Get a boolean value (0 = false, 1 = true).

## Setting Values

### Set String

```c
int idcu_config_set_string(const char* section, const char* key, const char* value);
```

Set a string value.

### Set Integer

```c
int idcu_config_set_int(const char* section, const char* key, int value);
```

Set an integer value.

### Set Int64

```c
int idcu_config_set_int64(const char* section, const char* key, int64_t value);
```

Set a 64-bit integer value.

### Set Double

```c
int idcu_config_set_double(const char* section, const char* key, double value);
```

Set a double value.

### Set Boolean

```c
int idcu_config_set_bool(const char* section, const char* key, int value);
```

Set a boolean value.

## List Operations

### Get List

```c
int idcu_config_get_list(const char* section, const char* key, const char* delimiter, idcu_ConfigList* out_list);
```

Get a list of strings.

### Set List

```c
int idcu_config_set_list(const char* section, const char* key, const char* delimiter, const idcu_ConfigList* list);
```

Set a list of strings.

## Change Notifications

### Register Change Callback

```c
int idcu_config_register_change_callback(idcu_ConfigChangeCallback callback, void* user_data);
```

Register a callback to be notified when config changes.

### Unregister Change Callback

```c
int idcu_config_unregister_change_callback(idcu_ConfigChangeCallback callback);
```

Unregister a change callback.

## Environment Variables

### Enable Environment Variables

```c
void idcu_config_enable_env_var(int enable);
```

Enable/disable environment variable support.

## File Watch

### Start Watch

```c
int idcu_config_watch_start(uint32_t interval_ms);
```

Start watching the config file for changes.

### Stop Watch

```c
void idcu_config_watch_stop(void);
```

Stop watching the config file.

### Is Watch Running

```c
int idcu_config_watch_is_running(void);
```

Check if file watch is running.

## Profile Loading

### Load Profile

```c
int idcu_config_load_profile(const char* profile_name);
```

Load a configuration profile.

## Examples

### Basic Usage

```c
idcu_config_init("app.ini");

// Get values with defaults
const char* host = idcu_config_get_string("database", "host", "localhost");
int port = idcu_config_get_int("database", "port", 5432);

printf("Connecting to %s:%d\n", host, port);

idcu_config_shutdown();
```

### Setting Values

```c
idcu_config_init("app.ini");

idcu_config_set_string("app", "name", "MyApp");
idcu_config_set_int("app", "version", 1);
idcu_config_set_bool("app", "debug", 1);

idcu_config_save("app.ini");
idcu_config_shutdown();
```
