# 模块开发模板

本目录包含 IDCU Agent 模块开发的标准模板，帮助开发者快速创建新模块。

## 目录结构

```
templates/module/
├── README.md.template          # 模块 README 模板
├── CMakeLists.txt.template     # CMake 构建配置模板
├── src/
│   └── module.c.template       # 模块源代码模板
├── include/
│   └── idcu/
│       └── module/
│           └── module.h.template  # 模块头文件模板
└── tests/
    └── CMakeLists.txt.template     # 测试配置模板
    └── test_module.c.template      # 单元测试模板
```

## 快速开始

### 1. 使用 idcu-module-build 工具生成

推荐使用 `idcu-module-build` 工具自动生成模块框架：

```bash
cd libs/idcu-module-build/scripts
python generate_template.py --name my_module --output ../../modules/business/my_module
```

### 2. 手动复制模板

如果需要手动创建，可以按照以下步骤：

```bash
# 创建模块目录
mkdir -p modules/business/my_module/src
mkdir -p modules/business/my_module/include/idcu/my_module
mkdir -p modules/business/my_module/tests

# 复制模板文件
cp templates/module/README.md.template modules/business/my_module/README.md
cp templates/module/CMakeLists.txt.template modules/business/my_module/CMakeLists.txt
cp templates/module/src/module.c.template modules/business/my_module/src/my_module.c
cp templates/module/include/idcu/module/module.h.template modules/business/my_module/include/idcu/my_module/my_module.h
cp templates/module/tests/CMakeLists.txt.template modules/business/my_module/tests/CMakeLists.txt
cp templates/module/tests/test_module.c.template modules/business/my_module/tests/test_my_module.c
```

## 模板文件说明

### README.md.template

模块的说明文档模板，包含：
- 模块功能简介
- 依赖关系说明
- 配置选项
- 使用示例
- API 文档

### CMakeLists.txt.template

CMake 构建配置模板，包含：
- 模块名称设置
- 源文件列表
- 依赖库配置
- 模块安装规则

### src/module.c.template

模块源代码模板，包含：
- 模块初始化函数
- 模块运行函数
- 模块停止函数
- 模块注册宏

### include/idcu/module/module.h.template

模块头文件模板，包含：
- 公共 API 声明
- 数据结构定义
- 错误码定义

### tests/CMakeLists.txt.template

测试 CMake 配置模板。

### tests/test_module.c.template

单元测试模板，使用 idcu-testframework 编写测试用例。

## 模块命名规范

### 目录和文件命名

- 使用小写字母和下划线
- 避免使用特殊字符
- 示例：`my_module`、`data_collector`

### 函数命名

- 使用 `idcu_` 前缀
- 使用小写字母和下划线
- 示例：`idcu_my_module_init()`、`idcu_my_module_do_something()`

### 类型命名

- 使用 `idcu_` 前缀
- 使用大驼峰命名法
- 示例：`idcu_MyModuleContext`、`idcu_MyModuleConfig`

### 宏命名

- 全大写字母
- 使用下划线分隔
- 示例：`IDCU_MY_MODULE_VERSION`、`IDCU_MY_MODULE_MAX_SIZE`

## 模块生命周期

每个模块都有三个核心生命周期函数：

```c
// 1. 初始化 - 模块加载时调用
static int my_module_init(void) {
    // 初始化资源、配置等
    return IDCU_ERR_OK;
}

// 2. 运行 - 模块主循环，协程调度
static int my_module_run(void) {
    // 模块主要逻辑
    return IDCU_ERR_OK;
}

// 3. 停止 - 模块卸载时调用
static int my_module_stop(void) {
    // 清理资源
    return IDCU_ERR_OK;
}

// 注册模块
IDCU_REGISTER_MODULE(my_module, IDCU_MODULE_VERSION(1, 0, 0),
                      my_module_init, my_module_run, my_module_stop);
```

## 配置模块

在模块中读取配置：

```c
#include "idcu/config/config.h"

// 读取配置
const char* log_level = idcu_config_get_string("general", "log_level", "info");
int interval = idcu_config_get_int("module.my_module", "interval_ms", 1000);

// 监听配置变更
void on_config_change(const char* section, const char* key,
                      const char* old_value, const char* new_value,
                      void* user_data) {
    if (strcmp(section, "module.my_module") == 0) {
        // 处理配置变更
    }
}

idcu_config_register_change_callback(on_config_change, NULL);
```

## 模块间通信

使用消息总线进行模块间通信：

```c
#include "idcu/msgbus/msg_bus.h"

// 发送消息
idcu_Message msg;
msg.type = IDCU_MSG_TYPE_CUSTOM;
msg.sender = "my_module";
msg.recipient = "other_module";
msg.data = my_data;
msg.data_size = sizeof(my_data);

idcu_msgbus_send(&msg);

// 接收消息
idcu_Message* received_msg = idcu_msgbus_recv("my_module", 1000);
if (received_msg) {
    // 处理消息
    idcu_msgbus_free(received_msg);
}
```

## 日志记录

使用统一的日志系统：

```c
#include "idcu/log/log.h"

IDCU_LOG_DEBUG("Debug message: %d", value);
IDCU_LOG_INFO("Info message");
IDCU_LOG_WARN("Warning message");
IDCU_LOG_ERROR("Error message: %s", error_str);
```

## 测试模块

编写单元测试：

```c
#include "idcu/testframework/test_framework.h"

TEST_CASE(my_module_test_basic) {
    // 测试初始化
    int ret = idcu_my_module_init();
    TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    // 测试功能
    int result = idcu_my_module_do_something();
    TEST_ASSERT_TRUE(result > 0);
    
    // 清理
    idcu_my_module_shutdown();
}

TEST_SUITE(my_module_tests) {
    RUN_TEST_CASE(my_module_test_basic);
}
```

## 构建和编译

```bash
# 在项目根目录
mkdir -p build && cd build
cmake ..
make my_module
```

## 更多资源

- [模块开发教程](../../docs/tutorial_02_module_development.md)
- [SDK 指南](../../docs/sdk_guide.md)
- [架构设计文档](../../docs/architecture.md)
- 查看现有模块示例：`modules/business/examples/`
