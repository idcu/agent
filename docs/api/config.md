# idcu-config API Documentation

Configuration management library.

## Quick Start

```c
#include <idcu/config/config.h>

int main() {
    idcu_Config* config = NULL;
    idcu_config_init(&config, "config.ini");
    
    const char* value = idcu_config_get_string(config, "database", "host", "localhost");
    printf("Database host: %s\n", value);
    
    idcu_config_set_int(config, "app", "port", 8080);
    idcu_config_save(config, "config.ini");
    
    idcu_config_destroy(config);
    return 0;
}
```

## Configuration Structure

### Config Type

```c
typedef struct idcu_Config idcu_Config;
```

## Initialization

### Initialize Config

```c
idcu_ErrorCode idcu_config_init(idcu_Config** config, const char* file_path);
```

Initialize a configuration manager, optionally loading from a file.

- `config`: Output pointer to config instance
- `file_path`: Path to config file (NULL for empty config)

### Load from File

```c
idcu_ErrorCode idcu_config_load(idcu_Config* config, const char* file_path);
```

Load configuration from a file.

### Save to File

```c
idcu_ErrorCode idcu_config_save(idcu_Config* config, const char* file_path);
```

Save configuration to a file.

### Destroy

```c
void idcu_config_destroy(idcu_Config* config);
```

Destroy the configuration manager.

## Getting Values

### Get String

```c
const char* idcu_config_get_string(idcu_Config* config, const char* section, const char* key, const char* default_value);
```

Get a string value.

### Get Integer

```c
int idcu_config_get_int(idcu_Config* config, const char* section, const char* key, int default_value);
```

Get an integer value.

### Get Int64

```c
int64_t idcu_config_get_int64(idcu_Config* config, const char* section, const char* key, int64_t default_value);
```

Get a 64-bit integer value.

### Get Double

```c
double idcu_config_get_double(idcu_Config* config, const char* section, const char* key, double default_value);
```

Get a double value.

### Get Boolean

```c
int idcu_config_get_bool(idcu_Config* config, const char* section, const char* key, int default_value);
```

Get a boolean value (0 = false, 1 = true).

## Setting Values

### Set String

```c
idcu_ErrorCode idcu_config_set_string(idcu_Config* config, const char* section, const char* key, const char* value);
```

Set a string value.

### Set Integer

```c
idcu_ErrorCode idcu_config_set_int(idcu_Config* config, const char* section, const char* key, int value);
```

Set an integer value.

### Set Int64

```c
idcu_ErrorCode idcu_config_set_int64(idcu_Config* config, const char* section, const char* key, int64_t value);
```

Set a 64-bit integer value.

### Set Double

```c
idcu_ErrorCode idcu_config_set_double(idcu_Config* config, const char* section, const char* key, double value);
```

Set a double value.

### Set Boolean

```c
idcu_ErrorCode idcu_config_set_bool(idcu_Config* config, const char* section, const char* key, int value);
```

Set a boolean value.

## List Operations

### Get List

```c
idcu_ErrorCode idcu_config_get_list(idcu_Config* config, const char* section, const char* key, char*** out_values, size_t* out_count);
```

Get a list of strings.

### Set List

```c
idcu_ErrorCode idcu_config_set_list(idcu_Config* config, const char* section, const char* key, const char** values, size_t count);
```

Set a list of strings.

### Free List

```c
void idcu_config_free_list(char** values, size_t count);
```

Free the memory allocated by `idcu_config_get_list`.

## Change Notifications

### Register Change Callback

```c
idcu_ErrorCode idcu_config_register_change_callback(idcu_Config* config, idcu_ConfigChangeCallback callback, void* user_data);
```

Register a callback to be notified when config changes.

### Unregister Change Callback

```c
void idcu_config_unregister_change_callback(idcu_Config* config, idcu_ConfigChangeCallback callback);
```

Unregister a change callback.

## Environment Variables

### Load from Environment

```c
idcu_ErrorCode idcu_config_load_from_env(idcu_Config* config, const char* prefix);
```

Load configuration from environment variables.

## Hot Reload

### Enable Auto Reload

```c
idcu_ErrorCode idcu_config_enable_auto_reload(idcu_Config* config, const char* file_path, int check_interval_ms);
```

Enable automatic reloading when the config file changes.

### Disable Auto Reload

```c
void idcu_config_disable_auto_reload(idcu_Config* config);
```

Disable automatic reloading.

### Reload Manually

```c
idcu_ErrorCode idcu_config_reload(idcu_Config* config);
```

Reload the config file manually.

## Profile Loading

### Load Profile

```c
idcu_ErrorCode idcu_config_load_profile(idcu_Config* config, const char* profile_name);
```

Load a configuration profile.

## Examples

### Basic Usage

```c
idcu_Config* config = NULL;
idcu_config_init(&config, "app.ini");

// Get values with defaults
const char* host = idcu_config_get_string(config, "database", "host", "localhost");
int port = idcu_config_get_int(config, "database", "port", 5432);

printf("Connecting to %s:%d\n", host, port);

idcu_config_destroy(config);
```

### Setting Values

```c
idcu_Config* config = NULL;
idcu_config_init(&config, NULL);

idcu_config_set_string(config, "app", "name", "MyApp");
idcu_config_set_int(config, "app", "version", 1);
idcu_config_set_bool(config, "app", "debug", 1);

idcu_config_save(config, "app.ini");
idcu_config_destroy(config);
```

### Working with Lists

```c
idcu_Config* config = NULL;
idcu_config_init(&config, NULL);

const char* servers[] = {"server1", "server2", "server3"};
idcu_config_set_list(config, "cluster", "servers", servers, 3);

char** values = NULL;
size_t count = 0;
idcu_config_get_list(config, "cluster", "servers", &values, &count);

for (size_t i = 0; i < count; i++) {
    printf("Server: %s\n", values[i]);
}

idcu_config_free_list(values, count);
idcu_config_destroy(config);
```

### Change Notifications

```c
void on_config_change(const char* section, const char* key, void* user_data) {
    printf("Config changed: %s.%s\n", section, key);
}

idcu_Config* config = NULL;
idcu_config_init(&config, "app.ini");
idcu_config_register_change_callback(config, on_config_change, NULL);

// ... any change will trigger the callback

idcu_config_destroy(config);
```

### Environment Variables

```c
idcu_Config* config = NULL;
idcu_config_init(&config, NULL);

// Load variables like MYAPP_DATABASE_HOST
idcu_config_load_from_env(config, "MYAPP_");

const char* host = idcu_config_get_string(config, "database", "host", "localhost");
idcu_config_destroy(config);
```

### Hot Reload

```c
idcu_Config* config = NULL;
idcu_config_init(&config, "app.ini");

// Check for changes every 5 seconds
idcu_config_enable_auto_reload(config, "app.ini", 5000);

// ... config will reload automatically when file changes

idcu_config_disable_auto_reload(config);
idcu_config_destroy(config);
```
