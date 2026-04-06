# 消息总线跨模块通信示例

## 功能说明

本示例演示如何使用IDCU的消息总线功能实现模块间的通信，包括点对点消息发送、广播消息、消息接收和处理等功能。

### 主要功能
- 点对点消息发送
- 广播消息
- 消息接收和处理
- 消息优先级管理
- 通信统计
- SDK消息总线API集成

## 编译方式

### 作为独立模块编译到IDCU Agent

1. 将 `msgbus_communication_example.c` 复制到 `modules/business/msgbus_comm/src/` 目录
2. 创建对应的 `CMakeLists.txt`
3. 在根目录的 `CMakeLists.txt` 中添加该模块
4. 运行编译脚本

### 作为独立程序运行（演示用）

```bash
gcc -o msgbus_communication_example msgbus_communication_example.c -I./modules/core/sdk/include -I./libs/idcu-log/include -I./libs/idcu-msgbus/include
./msgbus_communication_example
```

## 运行步骤

### 创建两个通信模块

为了演示模块间通信，建议创建两个实例：

1. **sender_module**: 负责发送消息
2. **receiver_module**: 负责接收消息

### 配置文件设置

在 `config/agent.cfg` 中启用两个模块：

```ini
[modules]
business = ..., msgbus_sender, msgbus_receiver

enable_msgbus_sender = true
msgbus_sender.priority = normal

enable_msgbus_receiver = true
msgbus_receiver.priority = normal
```

### 运行IDCU Agent

```bash
# Windows
out/idcu_agent.exe

# Linux
out/idcu_agent
```

## 核心代码解读

### 1. 消息数据结构

```c
typedef struct {
    uint32_t msg_type;
    int32_t int_value;
    char str_value[64];
} MessageData;
```

定义了消息的格式，包含：
- 消息类型（数据、命令、响应）
- 整数值
- 字符串值

### 2. 消息处理函数

```c
static void message_handler(idcu_SdkContext* ctx, const idcu_SdkMessage* msg, void* user_data) {
    CommModuleData* data = (CommModuleData*)user_data;
    // 验证消息有效性
    // 解析消息内容
    // 记录日志
    // 更新统计数据
}
```

- 接收来自消息总线的消息
- 解析消息内容
- 更新接收计数
- 记录详细日志

### 3. 点对点消息发送

```c
static int send_test_message(idcu_SdkContext* ctx, CommModuleData* data, const char* target_module) {
    MessageData msg_data;
    // 填充消息数据
    int ret = idcu_sdk_send_message(
        ctx,
        target_module,
        IDCU_MSG_PRIO_NORMAL,
        (const uint8_t*)&msg_data,
        sizeof(msg_data)
    );
    // 处理返回结果
}
```

- 使用 `idcu_sdk_send_message` 发送点对点消息
- 指定目标模块名称
- 设置消息优先级
- 传递序列化的数据

### 4. 广播消息

```c
static int broadcast_test_message(idcu_SdkContext* ctx, CommModuleData* data) {
    MessageData msg_data;
    // 填充消息数据
    int ret = idcu_sdk_broadcast_message(
        ctx,
        IDCU_MSG_PRIO_HIGH,
        (const uint8_t*)&msg_data,
        sizeof(msg_data)
    );
    // 处理返回结果
}
```

- 使用 `idcu_sdk_broadcast_message` 发送广播
- 所有注册的模块都能收到
- 适合系统通知和事件广播

### 5. 模块初始化与消息处理器注册

```c
static int comm_module_init(idcu_SdkContext* ctx) {
    CommModuleData* data = (CommModuleData*)malloc(sizeof(CommModuleData));
    // 初始化数据
    int ret = idcu_sdk_register_message_handler(ctx, message_handler, data);
    // 注册消息处理器
    idcu_sdk_set_user_data(ctx, data);
    return IDCU_ERR_OK;
}
```

- 分配模块数据
- 注册消息处理回调函数
- 设置用户数据供回调使用

## 消息优先级

IDCU消息总线支持以下优先级：

| 优先级 | 用途 | 处理顺序 |
|--------|------|----------|
| `IDCU_MSG_PRIO_LOW` | 非紧急数据 | 最后处理 |
| `IDCU_MSG_PRIO_NORMAL` | 普通消息 | 正常顺序 |
| `IDCU_MSG_PRIO_HIGH` | 重要数据 | 优先处理 |
| `IDCU_MSG_PRIO_CRITICAL` | 紧急事件 | 最优先处理 |

## 扩展建议

1. **消息序列化**: 使用JSON或Protocol Buffers进行更复杂的消息序列化
2. **请求-响应模式**: 实现请求ID匹配，支持异步请求-响应
3. **消息过滤**: 添加消息主题或类型过滤机制
4. **消息持久化**: 支持将重要消息持久化到磁盘
5. **性能监控**: 添加消息吞吐量和延迟监控

## 依赖关系

- `idcu-core-sdk`: SDK核心功能
- `idcu-msgbus`: 消息总线库
- `idcu-log`: 日志系统
- `idcu-common`: 通用工具和错误码
