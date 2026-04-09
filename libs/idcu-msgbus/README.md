# idcu-msgbus

Message bus library for IDCU Agent, providing publish-subscribe messaging pattern with priority queues.

## Features

- Publish-Subscribe messaging pattern
- Priority-based message queuing (LOW, NORMAL, HIGH, CRITICAL)
- Topic-based message routing (up to 64 topics)
- Thread-safe operations

## Usage

```c
#include <idcu/msgbus/msgbus.h>

void my_handler(idcu_MsgTopic topic, const void* data, size_t data_size, void* user_data) {
    // Handle message
}

int main() {
    idcu_MsgBus* bus = NULL;
    idcu_msgbus_init(&bus);

    idcu_MsgSubscriber* sub = NULL;
    idcu_msgbus_subscribe(bus, 1, my_handler, NULL, &sub);

    const char* msg = "Hello!";
    idcu_msgbus_publish(bus, 1, msg, strlen(msg), IDCU_MSG_PRIORITY_NORMAL);

    idcu_msgbus_process(bus);

    idcu_msgbus_unsubscribe(bus, sub);
    idcu_msgbus_destroy(bus);
    return 0;
}
```

## License

MIT
