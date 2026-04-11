# idcu-config API 文档

配置管理库。

## 快速开始

```c
#include <idcu/config/config.h>

int main() {
    idcu_config_init("config.ini");
    
    const char* value = idcu_config_get_string("database", "host", "localhost");
    printf("数据库主机: %s\n", value);
    
    idcu_config_set_int("app", "port", 8080);
    idcu_config_save("config.ini");
    
    idcu_config_shutdown();
    return 0;
}
```

## 初始化

### 初始化配置

```c
int idcu_config_init(const char* file_path);
```

初始化配置管理器，从文件加载配置。

- `file_path`: 配置文件路径

### 关闭

```c
void idcu_config_shutdown(void);
```

关闭配置管理器。

### 是否已加载

```c
int idcu_config_is_loaded(void);
```

检查配置是否已加载。

### 重新加载

```c
int idcu_config_reload(void);
```

重新加载配置文件。

### 保存

```c
int idcu_config_save(const char* file_path);
```

将配置保存到文件。

## 获取值

### 获取字符串

```c
const char* idcu_config_get_string(const char* section, const char* key, const char* default_value);
```

获取字符串值。

### 获取整数

```c
int idcu_config_get_int(const char* section, const char* key, int default_value);
```

获取整数值。

### 获取 64 位整数

```c
int64_t idcu_config_get_int64(const char* section, const char* key, int64_t default_value);
```

获取 64 位整数值。

### 获取双精度浮点数

```c
double idcu_config_get_double(const char* section, const char* key, double default_value);
```

获取双精度浮点数值。

### 获取布尔值

```c
int idcu_config_get_bool(const char* section, const char* key, int default_value);
```

获取布尔值（0 = 假，1 = 真）。

## 设置值

### 设置字符串

```c
int idcu_config_set_string(const char* section, const char* key, const char* value);
```

设置字符串值。

### 设置整数

```c
int idcu_config_set_int(const char* section, const char* key, int value);
```

设置整数值。

### 设置 64 位整数

```c
int idcu_config_set_int64(const char* section, const char* key, int64_t value);
```

设置 64 位整数值。

### 设置双精度浮点数

```c
int idcu_config_set_double(const char* section, const char* key, double value);
```

设置双精度浮点数值。

### 设置布尔值

```c
int idcu_config_set_bool(const char* section, const char* key, int value);
```

设置布尔值。

## 列表操作

### 获取列表

```c
int idcu_config_get_list(const char* section, const char* key, const char* delimiter, idcu_ConfigList* out_list);
```

获取字符串列表。

### 设置列表

```c
int idcu_config_set_list(const char* section, const char* key, const char* delimiter, const idcu_ConfigList* list);
```

设置字符串列表。

## 变更通知

### 注册变更回调

```c
int idcu_config_register_change_callback(idcu_ConfigChangeCallback callback, void* user_data);
```

注册一个回调，当配置变更时会收到通知。

### 注销变更回调

```c
int idcu_config_unregister_change_callback(idcu_ConfigChangeCallback callback);
```

注销一个变更回调。

## 环境变量

### 启用环境变量

```c
void idcu_config_enable_env_var(int enable);
```

启用/禁用环境变量支持。

## 文件监视

### 开始监视

```c
int idcu_config_watch_start(uint32_t interval_ms);
```

开始监视配置文件的变更。

### 停止监视

```c
void idcu_config_watch_stop(void);
```

停止监视配置文件。

### 监视是否正在运行

```c
int idcu_config_watch_is_running(void);
```

检查文件监视是否正在运行。

## 配置文件加载

### 加载配置文件

```c
int idcu_config_load_profile(const char* profile_name);
```

加载一个配置文件。

## 示例

### 基本用法

```c
idcu_config_init("app.ini");

// 获取带默认值的值
const char* host = idcu_config_get_string("database", "host", "localhost");
int port = idcu_config_get_int("database", "port", 5432);

printf("连接到 %s:%d\n", host, port);

idcu_config_shutdown();
```

### 设置值

```c
idcu_config_init("app.ini");

idcu_config_set_string("app", "name", "我的应用");
idcu_config_set_int("app", "version", 1);
idcu_config_set_bool("app", "debug", 1);

idcu_config_save("app.ini");
idcu_config_shutdown();
```
