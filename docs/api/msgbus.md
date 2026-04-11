# idcu-msgbus API Documentation

Message bus for publish-subscribe communication.

## Message Bus

```c
typedef struct idcu_MsgBus idcu_MsgBus;
```

### Message Bus Functions

```c
int idcu_msgbus_init(idcu_MsgBus** out_bus);
void idcu_msgbus_destroy(idcu_MsgBus* bus);

int idcu_msgbus_publish(idcu_MsgBus* bus, idcu_MsgTopic topic, const void* data, size_t data_size, idcu_MsgPriority priority);
int idcu_msgbus_process(idcu_MsgBus* bus);
int idcu_msgbus_process_one(idcu_MsgBus* bus);

int idcu_msgbus_get_pending_count(idcu_MsgBus* bus);
void idcu_msgbus_clear(idcu_MsgBus* bus);
```

## Subscriber

```c
typedef struct idcu_MsgSubscriber idcu_MsgSubscriber;
```

### Subscription Functions

```c
typedef void (*idcu_MsgHandler)(idcu_MsgTopic topic, const void* data, size_t data_size, void* user_data);

int idcu_msgbus_subscribe(idcu_MsgBus* bus, idcu_MsgTopic topic, idcu_MsgHandler handler, void* user_data, idcu_MsgSubscriber** out_subscriber);
int idcu_msgbus_unsubscribe(idcu_MsgBus* bus, idcu_MsgSubscriber* subscriber);

int idcu_msgbus_subscribe_pattern(idcu_MsgBus* bus, const char* pattern, idcu_MsgHandler handler, void* user_data, idcu_MsgSubscriber** out_subscriber);
```

## Message Topics

```c
typedef uint32_t idcu_MsgTopic;
```

## Message Priorities

```c
typedef enum {
    IDCU_MSG_PRIORITY_LOW,
    IDCU_MSG_PRIORITY_NORMAL,
    IDCU_MSG_PRIORITY_HIGH,
    IDCU_MSG_PRIORITY_CRITICAL
} idcu_MsgPriority;
```

## Example

```c
#include <idcu/msgbus/msgbus.h>
#include <stdio.h>
#include <string.h>

#define TOPIC_TEST 100

static void message_handler(idcu_MsgTopic topic, const void* data, size_t data_size, void* user_data) {
    printf("Received message on topic %u\n", topic);
    if (data && data_size > 0) {
        printf("Data: %s\n", (const char*)data);
    }
}

int main(void) {
    idcu_MsgBus* bus = NULL;
    idcu_msgbus_init(&bus);
    
    // Subscribe
    idcu_MsgSubscriber* subscriber = NULL;
    idcu_msgbus_subscribe(bus, TOPIC_TEST, message_handler, NULL, &subscriber);
    
    // Publish a message
    const char* message = "Hello, Message Bus!";
    idcu_msgbus_publish(bus, TOPIC_TEST, message, strlen(message) + 1, IDCU_MSG_PRIORITY_NORMAL);
    
    // Process messages
    idcu_msgbus_process(bus);
    
    // Unsubscribe
    idcu_msgbus_unsubscribe(bus, subscriber);
    
    idcu_msgbus_destroy(bus);
    return 0;
}
```
