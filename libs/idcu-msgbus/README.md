# idcu-msgbus

IDCU Agent 的消息总线库，提供带优先级队列的发布-订阅消息模式。

## 功能特性

- 发布-订阅消息模式
- 基于优先级的消息队列（LOW、NORMAL、HIGH、CRITICAL）
- 基于主题的消息路由（最多 64 个主题）
- 线程安全操作

## 使用方法

```c
#include <idcu/msgbus/msgbus.h>

void my_handler(idcu_MsgTopic topic, const void* data, size_t data_size, void* user_data) {
    // 处理消息
}

int main() {
    idcu_MsgBus* bus = NULL;
    idcu_msgbus_init(&bus);

    idcu_MsgSubscriber* sub = NULL;
    idcu_msgbus_subscribe(bus, 1, my_handler, NULL, &sub);

    const char* msg = "你好!";
    idcu_msgbus_publish(bus, 1, msg, strlen(msg), IDCU_MSG_PRIORITY_NORMAL);

    idcu_msgbus_process(bus);

    idcu_msgbus_unsubscribe(bus, sub);
    idcu_msgbus_destroy(bus);
    return 0;
}
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
