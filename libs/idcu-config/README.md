# idcu-config

IDCU 项目的独立配置管理库，提供简单易用的配置文件解析和管理功能。

## 特性

- 跨平台支持（Windows、Linux、macOS）
- 支持 INI 格式配置文件
- 支持多种数据类型（字符串、整数、浮点数、布尔值）
- 支持环境变量扩展
- 线程安全
- **配置热更新（支持运行时重载配置）**
- **配置文件监控和自动重载**
- **配置变更回调支持**
- 支持配置文件保存

## 快速开始

### 构建

```bash
mkdir build &amp;&amp; cd build
cmake ..
cmake --build .
```

### 安装

```bash
cmake --install .
```

### 在其他项目中使用

```cmake
find_package(idcu-common REQUIRED)
find_package(idcu-config REQUIRED)
add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE idcu::config)
```

## API 文档

### 初始化和关闭

```c
#include &lt;idcu/config/config.h&gt;

// 初始化配置管理器
int ret = idcu_config_init("config.ini");
if (ret == IDCU_ERR_SUCCESS) {
    // 使用配置...
    idcu_config_shutdown();
}
```

### 读取配置

```c
// 读取字符串
const char* value = idcu_config_get_string("section", "key", "default");

// 读取整数
int int_val = idcu_config_get_int("section", "key", 0);

// 读取 64 位整数
int64_t int64_val = idcu_config_get_int64("section", "key", 0);

// 读取浮点数
double double_val = idcu_config_get_double("section", "key", 0.0);

// 读取布尔值
bool bool_val = idcu_config_get_bool("section", "key", false);
```

### 写入配置

```c
// 写入字符串
idcu_config_set_string("section", "key", "value");

// 写入整数
idcu_config_set_int("section", "key", 42);

// 写入布尔值
idcu_config_set_bool("section", "key", true);
```

### 保存配置

```c
idcu_config_save("config.ini");
```

### 配置变更回调

```c
void on_config_change(const char* section, const char* key, 
                      const char* old_value, const char* new_value, 
                      void* user_data) {
    // 处理配置变更
}

// 注册回调
idcu_config_register_change_callback(on_config_change, NULL);

// 注销回调
idcu_config_unregister_change_callback(on_config_change);
```

### 配置热更新

```c
// 手动重载配置
int ret = idcu_config_reload();
if (ret == IDCU_ERR_SUCCESS) {
    printf("Config reloaded successfully!\n");
}

// 启动配置文件监控（自动重载）
ret = idcu_config_watch_start();
if (ret == IDCU_ERR_SUCCESS) {
    printf("Config watcher started!\n");
}

// 停止配置文件监控
idcu_config_watch_stop();
```

## 配置文件格式

配置文件使用标准 INI 格式：

```ini
[general]
log_level = info
port = 8080
enabled = true

[database]
host = localhost
port = 5432
name = mydb
```

## 测试

```bash
cd build
ctest
```

## 示例

库提供了以下示例代码：

- [简单配置示例](examples/example_basic.c) - 演示基本配置读写
- [配置保存示例](examples/example_save.c) - 演示如何保存配置
- **[配置热更新示例](examples/example_hot_reload.c) - 演示配置文件监控和自动重载**

### 编译和运行示例

```bash
cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build .

./example_basic
./example_save
./example_hot_reload
```

## 配置热更新使用指南

### 基本概念

idcu-config 库支持两种配置热更新方式：

1. **手动重载**：调用 `idcu_config_reload()` 函数手动重新加载配置文件
2. **文件监控自动重载**：使用 `idcu_config_watch_start()` 启动文件监控，配置文件变更时自动重载

### 工作流程

1. 初始化配置管理器
2. 注册配置变更回调（可选但推荐）
3. 启动配置文件监控（如需自动重载）
4. 运行时配置变更会自动触发回调
5. 模块根据回调更新自身行为
6. 关闭时停止监控并清理资源

### 多环境配置

配合项目的多环境配置目录使用：

```
config/
├── dev/           # 开发环境配置
│   └── agent.cfg
├── test/          # 测试环境配置
│   └── agent.cfg
└── prod/          # 生产环境配置
    └── agent.cfg
```

在不同环境启动时，使用对应的配置文件：

```bash
# 开发环境
./agent --config=config/dev/agent.cfg

# 生产环境
./agent --config=config/prod/agent.cfg
```

## 依赖

- idcu-common - IDCU 基础通用组件库

## 许可证

详见项目根目录的 LICENSE 文件。
