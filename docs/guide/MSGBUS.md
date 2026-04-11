# Message Bus Design

This document describes the design of the IDCU Agent message bus.

## Overview

The message bus provides a publish-subscribe communication system for inter-module communication.

## Core Concepts

### Topics

Messages are organized by topics. Topics are 32-bit integers:

```c
typedef uint32_t idcu_MsgTopic;
```

Topic conventions:
- Use higher bits for category
- Use lower bits for specific message types

Example topic definitions:

```c
#define TOPIC_SYSTEM_BASE     0x00010000
#define TOPIC_SYSTEM_STARTUP  (TOPIC_SYSTEM_BASE | 0x0001)
#define TOPIC_SYSTEM_SHUTDOWN (TOPIC_SYSTEM_BASE | 0x0002)

#define TOPIC_METRICS_BASE    0x00020000
#define TOPIC_METRICS_UPDATE  (TOPIC_METRICS_BASE | 0x0001)

#define TOPIC_LOG_BASE        0x00030000
#define TOPIC_LOG_ENTRY       (TOPIC_LOG_BASE | 0x0001)
```

### Message Priorities

Messages can have priority levels:

```c
typedef enum {
    IDCU_MSG_PRIORITY_LOW,      // Low priority (batch processing)
    IDCU_MSG_PRIORITY_NORMAL,    // Normal priority (default)
    IDCU_MSG_PRIORITY_HIGH,      // High priority (time-sensitive)
    IDCU_MSG_PRIORITY_CRITICAL   // Critical priority (urgent)
} idcu_MsgPriority;
```

Higher priority messages are delivered before lower priority ones.

### Message Handler

```c
typedef void (*idcu_MsgHandler)(idcu_MsgTopic topic,
                                 const void* data,
                                 size_t data_size,
                                 void* user_data);
```

## Message Bus API

### Initialization

```c
// Create message bus
int idcu_msgbus_init(idcu_MsgBus** out_bus);

// Destroy message bus
void idcu_msgbus_destroy(idcu_MsgBus* bus);
```

### Publish

```c
// Publish a message
int idcu_msgbus_publish(idcu_MsgBus* bus,
                         idcu_MsgTopic topic,
                         const void* data,
                         size_t data_size,
                         idcu_MsgPriority priority);
```

### Subscribe

```c
// Subscribe to a topic
int idcu_msgbus_subscribe(idcu_MsgBus* bus,
                           idcu_MsgTopic topic,
                           idcu_MsgHandler handler,
                           void* user_data,
                           idcu_MsgSubscriber** out_subscriber);

// Subscribe with pattern matching
int idcu_msgbus_subscribe_pattern(idcu_MsgBus* bus,
                                    const char* pattern,
                                    idcu_MsgHandler handler,
                                    void* user_data,
                                    idcu_MsgSubscriber** out_subscriber);

// Unsubscribe
int idcu_msgbus_unsubscribe(idcu_MsgBus* bus,
                             idcu_MsgSubscriber* subscriber);
```

### Message Processing

```c
// Process all pending messages
int idcu_msgbus_process(idcu_MsgBus* bus);

// Process one message
int idcu_msgbus_process_one(idcu_MsgBus* bus);

// Get pending count
int idcu_msgbus_get_pending_count(idcu_MsgBus* bus);

// Clear all messages
void idcu_msgbus_clear(idcu_MsgBus* bus);
```

## Architecture

### Message Flow

```
Publisher
    ↓
[Message Queue (priority sorted)]
    ↓
[Dispatch]
    ↓
Subscriber 1 ── Handler 1
    ↓
Subscriber 2 ── Handler 2
    ↓
    ...
```

### Thread Safety

The message bus is thread-safe:
- Multiple threads can publish concurrently
- Multiple threads can subscribe concurrently
- Message dispatch is single-threaded (process() must be called)

## Usage Patterns

### Basic Publish-Subscribe

```c
#include <idcu/msgbus/msgbus.h>

#define TOPIC_EXAMPLE 0x1000

void my_handler(idcu_MsgTopic topic, const void* data, 
                size_t data_size, void* user_data) {
    printf("Received message on topic %u\n", topic);
    if (data && data_size > 0) {
        printf("Data: %s\n", (const char*)data);
    }
}

int main(void) {
    idcu_MsgBus* bus = NULL;
    idcu_msgbus_init(&bus);
    
    // Subscribe
    idcu_MsgSubscriber* sub = NULL;
    idcu_msgbus_subscribe(bus, TOPIC_EXAMPLE, my_handler, NULL, &sub);
    
    // Publish
    const char* msg = "Hello, Bus!";
    idcu_msgbus_publish(bus, TOPIC_EXAMPLE, msg, strlen(msg) + 1, 
                         IDCU_MSG_PRIORITY_NORMAL);
    
    // Process
    idcu_msgbus_process(bus);
    
    // Cleanup
    idcu_msgbus_unsubscribe(bus, sub);
    idcu_msgbus_destroy(bus);
    return 0;
}
```

### Request-Response Pattern

```c
typedef struct {
    idcu_MsgTopic reply_topic;
    int request_id;
} RequestHeader;

typedef struct {
    int request_id;
    int result;
} ResponseHeader;

#define TOPIC_REQUEST  0x2000
#define TOPIC_RESPONSE 0x2001

void request_handler(idcu_MsgTopic topic, const void* data, 
                     size_t data_size, void* user_data) {
    idcu_MsgBus* bus = (idcu_MsgBus*)user_data;
    const RequestHeader* header = (const RequestHeader*)data;
    
    // Process request...
    ResponseHeader resp = {
        .request_id = header->request_id,
        .result = 42
    };
    
    // Send response
    idcu_msgbus_publish(bus, header->reply_topic, &resp, 
                         sizeof(resp), IDCU_MSG_PRIORITY_HIGH);
}
```

### Pattern Matching

Pattern matching allows subscribing to multiple topics:

```c
// Subscribe to all topics in 0x1000-0x1FFF range
idcu_msgbus_subscribe_pattern(bus, "0x1*", handler, NULL, &sub);
```

## Best Practices

1. **Define clear topic conventions** - Document topic ranges and meanings
2. **Keep messages small** - Avoid large payloads
3. **Use priorities appropriately** - Reserve CRITICAL for emergencies
4. **Process regularly** - Call process() from main loop
5. **Handle NULL data** - Check data and data_size
6. **Don't block in handlers** - Keep handlers fast and async
7. **Use user_data for context** - Pass necessary state
