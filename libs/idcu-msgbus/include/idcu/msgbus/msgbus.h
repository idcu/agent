#ifndef IDCU_MSGBUS_MSGBUS_H
#define IDCU_MSGBUS_MSGBUS_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/msgbus/types.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_msgbus_init(idcu_MsgBus** bus);
void idcu_msgbus_destroy(idcu_MsgBus* bus);

int idcu_msgbus_publish(idcu_MsgBus* bus,
                         idcu_MsgTopic topic,
                         const void* data,
                         size_t data_size,
                         idcu_MsgPriority priority);

int idcu_msgbus_subscribe(idcu_MsgBus* bus,
                           idcu_MsgTopic topic,
                           idcu_MsgHandler handler,
                           void* user_data,
                           idcu_MsgSubscriber** subscriber);

int idcu_msgbus_unsubscribe(idcu_MsgBus* bus,
                             idcu_MsgSubscriber* subscriber);

int idcu_msgbus_process(idcu_MsgBus* bus);

#ifdef __cplusplus
}
#endif

#endif
