# API 参考文档 (API Reference)

本文档提供了 IDCU Agent 核心库和 SDK 的 API 参考，包括参数说明、返回值和异常场景。

---

## 目录
1. [生成 API 文档](#生成-api-文档)
2. [核心库概览](#核心库概览)
3. [SDK 接口](#sdk-接口)
4. [模块系统接口](#模块系统接口)

---

## 生成 API 文档

项目已配置 Doxygen 用于自动生成 API 文档。

### 使用 Doxygen 生成文档

#### 前置条件
安装 Doxygen：
- **Windows:** 从 https://www.doxygen.nl/download.html 下载安装
- **Linux (Ubuntu/Debian):**
  ```bash
  sudo apt-get install doxygen
  ```
- **Linux (CentOS/Fedora):**
  ```bash
  sudo yum install doxygen
  ```

#### 生成文档
在项目根目录运行：
```bash
doxygen Doxyfile
```

生成的 HTML 文档将位于 `docs/html/` 目录，打开 `index.html` 即可查看。

---

## 核心库概览

### libs/idcu-common - 通用基础库
提供项目通用的基础工具组件：
- **原子操作** (`atomic.h`) - 原子变量、原子计数器
- **锁机制** (`lock.h`) - 互斥锁、读写锁、条件变量
- **错误码** (`error_code.h`) - 统一错误码定义
- **数据结构** - 向量、链表、哈希表、字符串缓冲区

### libs/idcu-log - 日志系统
- **日志系统** (`log.h`) - 多级别日志、日志格式化、输出管理

### libs/idcu-config - 配置管理
- **配置管理** (`config.h`) - INI 格式配置读取、配置项查询

### libs/idcu-msgbus - 消息总线
- **消息总线** (`msg_bus.h`) - 模块间通信、消息队列

### libs/idcu-coroutine - 协程调度
- **协程** (`coroutine.h`) - 轻量级协程调度、多优先级

### libs/idcu-sandbox - 沙箱安全
- **沙箱** (`sandbox.h`) - 权限控制、资源限制

---

## SDK 接口

### 头文件
```c
#include "sdk.h"
```

### 函数列表

#### idcu_sdk_log_xxx() - 日志函数
```c
void idcu_sdk_log_debug(const char* fmt, ...);
void idcu_sdk_log_info(const char* fmt, ...);
void idcu_sdk_log_warn(const char* fmt, ...);
void idcu_sdk_log_error(const char* fmt, ...);
```
**参数：**
- `fmt` - 格式化字符串
- `...` - 可变参数

**返回值：** 无

---

#### idcu_sdk_config_get_int() - 读取整数配置
```c
int idcu_sdk_config_get_int(const char* section, const char* key, int default_value);
```
**参数：**
- `section` - 配置节名
- `key` - 配置项名
- `default_value` - 默认值（配置不存在时返回）

**返回值：** 配置的整数值，或默认值

---

#### idcu_sdk_config_get_string() - 读取字符串配置
```c
const char* idcu_sdk_config_get_string(const char* section, const char* key, const char* default_value);
```
**参数：**
- `section` - 配置节名
- `key` - 配置项名
- `default_value` - 默认值

**返回值：** 配置的字符串指针，或默认值

---

#### idcu_sdk_send_message() - 发送消息
```c
int idcu_sdk_send_message(const char* target_module, const void* data, size_t data_len);
```
**参数：**
- `target_module` - 目标模块名
- `data` - 消息数据指针
- `data_len` - 数据长度

**返回值：**
- `0` - 成功
- 非零 - 失败

---

#### idcu_sdk_recv_message() - 接收消息
```c
int idcu_sdk_recv_message(idcu_Message* msg);
```
**参数：**
- `msg` - 消息结构体指针，用于存储接收到的消息

**返回值：**
- `0` - 成功接收到消息
- 非零 - 无消息或失败

---

## 模块系统接口

### 头文件
```c
#include "module_def.h"
```

### 模块注册宏
```c
IDCU_REGISTER_MODULE(
    name,           // 模块名
    version,        // 版本号，使用 IDCU_MODULE_VERSION(major, minor, patch)
    init_func,      // 初始化函数
    run_func,       // 运行函数
    stop_func       // 停止函数
);
```

### 模块函数签名
```c
// 初始化函数
typedef int (*idcu_module_init_func)(void);

// 运行函数
typedef int (*idcu_module_run_func)(void);

// 停止函数
typedef int (*idcu_module_stop_func)(void);
```

**返回值约定：**
- `0` - 成功
- 非零 - 失败（初始化失败时模块不会被加载）

---

## 异常场景处理

### 初始化失败
如果模块的 `init()` 函数返回非零值，模块将不会被加载，系统会记录错误日志并继续加载其他模块。

### 消息发送失败
`idcu_msg_bus_send()` 可能失败的情况：
- 目标模块不存在
- 消息队列已满
- 消息数据过长

### 内存分配失败
系统使用 `malloc()` 分配内存时如果失败，会记录错误日志并返回错误码。模块应优雅处理内存分配失败的情况。

---

## 更多信息

有关完整的 API 文档，请使用 Doxygen 生成 HTML 文档，或直接查看各头文件中的注释。
