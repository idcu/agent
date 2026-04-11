# idcu-config

配置管理库 for IDCU Agent.

## 功能特性

- **配置格式支持**: 支持INI风格配置文件（支持YAML/JSON解析预留）
- **类型安全访问**: 支持string/int/int64/double/bool/list类型
- **配置读写**: 支持读取、写入和保存配置
- **热重载**: 支持配置重新加载
- **变更通知**: 配置变更回调通知
- **环境变量**: 支持环境变量覆盖配置
- **多环境**: 支持配置profile
- **线程安全**: 使用互斥锁保护配置访问

## 配置文件格式

支持INI风格的配置文件：

```ini
[default]
name=MyApp
version=1.0.0
debug=true

[database]
host=localhost
port=5432
username=admin
password=secret

[logging]
level=info
file=/var/log/myapp.log
max_files=5
```

## 使用方法

```c
#include <idcu/config/config.h>

// 初始化配置
int ret = idcu_config_init("config.ini");
if (ret != IDCU_ERR_OK) {
    // 处理错误
}

// 读取配置
const char* name = idcu_config_get_string("default", "name", "default");
int port = idcu_config_get_int("database", "port", 3306);
bool debug = idcu_config_get_bool("default", "debug", false);

// 写入配置
idcu_config_set_string("default", "name", "NewApp");
idcu_config_set_int("database", "port", 5433);

// 保存配置
idcu_config_save(NULL); // 保存到原文件

// 清理
idcu_config_shutdown();
```

## 类型安全访问

### 字符串
```c
const char* val = idcu_config_get_string("section", "key", "default");
idcu_config_set_string("section", "key", "value");
```

### 整数
```c
int val = idcu_config_get_int("section", "key", 0);
idcu_config_set_int("section", "key", 42);

int64_t val64 = idcu_config_get_int64("section", "key", 0);
idcu_config_set_int64("section", "key", 1234567890LL);
```

### 浮点数
```c
double val = idcu_config_get_double("section", "key", 0.0);
idcu_config_set_double("section", "key", 3.14);
```

### 布尔值
```c
bool val = idcu_config_get_bool("section", "key", false);
idcu_config_set_bool("section", "key", true);
```

### 列表
```c
idcu_ConfigList list;
idcu_config_get_list("section", "key", ",", &list);
for (int i = 0; i < list.count; i++) {
    printf("Item: %s\n", list.items[i]);
}
```

## 环境变量支持

```c
// 启用环境变量覆盖
idcu_config_enable_env_var(1);

// 环境变量格式: SECTION_KEY
// 例如: DATABASE_PORT=5432
int port = idcu_config_get_int("database", "port", 3306);
```

## 变更通知

```c
void on_config_change(const char* section, const char* key,
                     const char* old_value, const char* new_value,
                     void* user_data) {
    printf("Config changed: [%s] %s: %s -> %s\n",
           section, key, old_value, new_value);
}

// 注册回调
idcu_config_register_change_callback(on_config_change, NULL);

// 取消注册
idcu_config_unregister_change_callback(on_config_change);
```

## 热重载

```c
// 修改配置文件后重新加载
idcu_config_reload();
```

## API参考

### 核心API
- `idcu_config_init()` - 初始化配置
- `idcu_config_shutdown()` - 关闭配置
- `idcu_config_is_loaded()` - 检查配置是否已加载
- `idcu_config_reload()` - 重新加载配置
- `idcu_config_save()` - 保存配置

### 类型访问
- `idcu_config_get_string()` / `idcu_config_set_string()`
- `idcu_config_get_int()` / `idcu_config_set_int()`
- `idcu_config_get_int64()` / `idcu_config_set_int64()`
- `idcu_config_get_double()` / `idcu_config_set_double()`
- `idcu_config_get_bool()` / `idcu_config_set_bool()`

### 列表支持
- `idcu_config_get_list()` / `idcu_config_set_list()`

### 变更通知
- `idcu_config_register_change_callback()`
- `idcu_config_unregister_change_callback()`

### 环境变量
- `idcu_config_enable_env_var()`
- `idcu_config_load_profile()`

## 构建

```bash
cmake -B build && cmake --build build
```

## 依赖

- idcu-common
- idcu-json (预留)
- idcu-yaml (预留)

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
