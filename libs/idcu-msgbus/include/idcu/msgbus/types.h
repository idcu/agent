#ifndef IDCU_MSGBUS_TYPES_H
#define IDCU_MSGBUS_TYPES_H

#include <idcu/common/config.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IDCU_MSG_PRIORITY_LOW = 0,
    IDCU_MSG_PRIORITY_NORMAL = 1,
    IDCU_MSG_PRIORITY_HIGH = 2,
    IDCU_MSG_PRIORITY_CRITICAL = 3
} idcu_MsgPriority;

typedef unsigned int idcu_MsgTopic;

typedef struct idcu_MsgBus idcu_MsgBus;
typedef struct idcu_MsgSubscriber idcu_MsgSubscriber;

typedef void (*idcu_MsgHandler)(idcu_MsgTopic topic,
                                  const void* data,
                                  size_t data_size,
                                  void* user_data);

#define IDCU_MSG_MAX_TOPICS 64
#define IDCU_MSG_MAX_SIZE 4096
#define IDCU_MSG_PRIORITY_COUNT 4

#ifdef __cplusplus
}
#endif

#endif
