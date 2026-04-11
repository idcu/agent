# idcu-msgbus API 文档

用于发布-订阅通信的消息总线。

## 消息总线

```c
typedef struct idcu_MsgBus idcu_MsgBus;
```

### 消息总线函数

```c
int idcu_msgbus_init(idcu_MsgBus** out_bus);
void idcu_msgbus_destroy(idcu_MsgBus* bus);

int idcu_msgbus_publish(idcu_MsgBus* bus, idcu_MsgTopic topic, const void* data, size_t data_size, idcu_MsgPriority priority);
int idcu_msgbus_process(idcu_MsgBus* bus);
int idcu_msgbus_process_one(idcu_MsgBus* bus);

int idcu_msgbus_get_pending_count(idcu_MsgBus* bus);
void idcu_msgbus_clear(idcu_MsgBus* bus);
```

## 订阅者

```c
typedef struct idcu_MsgSubscriber idcu_MsgSubscriber;
```

### 订阅函数

```c
typedef void (*idcu_MsgHandler)(idcu_MsgTopic topic, const void* data, size_t data_size, void* user_data);

int idcu_msgbus_subscribe(idcu_MsgBus* bus, idcu_MsgTopic topic, idcu_MsgHandler handler, void* user_data, idcu_MsgSubscriber** out_subscriber);
int idcu_msgbus_unsubscribe(idcu_MsgBus* bus, idcu_MsgSubscriber* subscriber);

int idcu_msgbus_subscribe_pattern(idcu_MsgBus* bus, const char* pattern, idcu_MsgHandler handler, void* user_data, idcu_MsgSubscriber** out_subscriber);
```

## 消息主题

```c
typedef uint32_t idcu_MsgTopic;
```

## 消息优先级

```c
typedef enum {
    IDCU_MSG_PRIORITY_LOW,
    IDCU_MSG_PRIORITY_NORMAL,
    IDCU_MSG_PRIORITY_HIGH,
    IDCU_MSG_PRIORITY_CRITICAL
} idcu_MsgPriority;
```

## 示例

```c
#include <idcu/msgbus/msgbus.h>
#include <stdio.h>
#include <string.h>

#define TOPIC_TEST 100

static void message_handler(idcu_MsgTopic topic, const void* data, size_t data_size, void* user_data) {
    printf("在主题 %u 上收到消息\n", topic);
    if (data