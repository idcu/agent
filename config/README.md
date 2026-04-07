# IDCU Agent 配置管理

## 目录结构

```
config/
├── README.md              # 本文件 - 配置说明文档
├── agent.cfg              # 默认配置文件（开发环境默认）
├── development.cfg        # 开发环境配置
├── testing.cfg            # 测试环境配置
└── production.cfg         # 生产环境配置
```

## 环境配置说明

### 选择合适的配置

根据运行环境选择对应的配置文件：

| 环境 | 配置文件 | 说明 |
|------|---------|------|
| 本地开发 | `development.cfg` | 详细日志、禁用外部通知、配置热重载 |
| 测试/CI | `testing.cfg` | 中等日志、启用完整功能、严格验证 |
| 生产环境 | `production.cfg` | 最小日志、完整告警、资源限制严格 |

### 如何使用

#### 方式 1：指定配置文件启动

```bash
# Windows
idcu_agent.exe --config=config/production.cfg

# Linux
./idcu_agent --config=config/production.cfg
```

#### 方式 2：通过环境变量选择

```bash
# Windows
set IDCU_ENV=production
idcu_agent.exe

# Linux
export IDCU_ENV=production
./idcu_agent
```

程序会自动加载 `config/{IDCU_ENV}.cfg`

#### 方式 3：复制为默认配置

```bash
# 复制生产环境配置为默认配置
cp config/production.cfg config/agent.cfg
```

## 配置热更新

### 启用配置热更新

在配置文件中设置：

```ini
[general]
config_hot_reload = true
```

### 热更新工作原理

1. 程序定期检查配置文件的修改时间（默认每 5 秒）
2. 检测到配置文件修改后，自动重新加载配置
3. 通过配置变更回调通知相关模块
4. 模块根据新配置调整行为

### 支持热更新的配置项

| 配置项 | 热更新支持 | 说明 |
|--------|-----------|------|
| `log_level` | ✓ | 日志级别实时变更 |
| 模块 `enabled` | ✓ | 模块启停 |
| 模块参数配置 | ✓ | 大部分模块参数支持热更新 |
| `max_modules` | ✗ | 需要重启 |
| `max_messages` | ✗ | 需要重启 |

## 配置文件格式

### 基本语法

配置文件使用 INI 格式：

```ini
[section_name]
key = value
```

### 环境变量引用

可以在配置中引用环境变量：

```ini
[module.alert_module]
channels.sms.access_key_id = ${SMS_ACCESS_KEY_ID}
channels.sms.access_key_secret = ${SMS_ACCESS_KEY_SECRET}
```

程序启动时会自动替换为实际的环境变量值。

### 配置验证

默认启用配置验证：

```ini
[general]
config_validate = true
```

配置验证会检查：
- 必需配置项是否存在
- 配置值类型是否正确
- 配置值范围是否合理

## 多环境配置最佳实践

### 1. 环境变量管理敏感信息

生产环境中，敏感信息（密码、密钥）应通过环境变量传递，
不要直接写在配置文件中。

### 2. 配置文件版本控制

- `agent.cfg`、`development.cfg`、`testing.cfg` 可以提交到版本控制
- `production.cfg` 建议只保留模板，实际配置通过部署系统管理
- 可以创建 `production.cfg.example` 作为模板

### 3. 配置继承

可以通过 `profile` 段实现配置继承：

```ini
[profile.dev-override]
log_level = debug

[general]
# 基础配置...
```

在代码中调用 `idcu_config_load_profile("dev-override")` 应用覆盖配置。

## 配置 API 使用

### 初始化配置

```c
#include "idcu/config/config.h"

// 初始化配置管理器
idcu_config_init("config/agent.cfg");
```

### 读取配置

```c
// 读取字符串
const char* log_level = idcu_config_get_string("general", "log_level", "info");

// 读取整数
int max_modules = idcu_config_get_int("general", "max_modules", 32);

// 读取布尔值
int hot_reload = idcu_config_get_bool("general", "config_hot_reload", 0);
```

### 监听配置变更

```c
void on_config_change(const char* section, const char* key, 
                      const char* old_value, const char* new_value,
                      void* user_data) {
    printf("Config changed: [%s] %s: %s -> %s\n",
           section, key, old_value ? old_value : "(none)",
           new_value ? new_value : "(none)");
}

// 注册变更回调
idcu_config_register_change_callback(on_config_change, NULL);
```

### 手动重新加载

```c
// 强制重新加载配置
idcu_config_reload();
```
