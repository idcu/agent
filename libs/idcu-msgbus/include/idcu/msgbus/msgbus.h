#ifndef IDCU_MSGBUS_MSGBUS_H
#define IDCU_MSGBUS_MSGBUS_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/msgbus/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Thread-safe message bus initialization
 * @thread_safety Thread-safe: Can be called from any thread
 * @param bus Pointer to store the new message bus instance
 * @return Error code
 */
int idcu_msgbus_init(idcu_MsgBus** bus);

/**
 * @brief Destroy a message bus
 * @thread_safety NOT thread-safe: Must not be called concurrently with other operations
 * @param bus Message bus instance to destroy
 */
void idcu_msgbus_destroy(idcu_MsgBus* bus);

/**
 * @brief Publish a message to the bus
 * @thread_safety Thread-safe: Can be called from any thread concurrently
 * @param bus Message bus instance
 * @param topic Message topic
 * @param data Message data (can be NULL)
 * @param data_size Size of message data
 * @param priority Message priority
 * @return Error code
 */
int idcu_msgbus_publish(idcu_MsgBus* bus,
                         idcu_MsgTopic topic,
                         const void* data,
                         size_t data_size,
                         idcu_MsgPriority priority);

/**
 * @brief Subscribe to a message topic
 * @thread_safety Thread-safe: Can be called from any thread concurrently
 * @param bus Message bus instance
 * @param topic Topic to subscribe to
 * @param handler Message handler function
 * @param user_data User data passed to handler
 * @param subscriber Pointer to store the subscriber handle
 * @return Error code
 */
int idcu_msgbus_subscribe(idcu_MsgBus* bus,
                           idcu_MsgTopic topic,
                           idcu_MsgHandler handler,
                           void* user_data,
                           idcu_MsgSubscriber** subscriber);

/**
 * @brief Unsubscribe from a topic
 * @thread_safety Thread-safe: Can be called from any thread concurrently
 * @param bus Message bus instance
 * @param subscriber Subscriber handle to unsubscribe
 * @return Error code
 */
int idcu_msgbus_unsubscribe(idcu_MsgBus* bus,
                             idcu_MsgSubscriber* subscriber);

/**
 * @brief Process pending messages
 * @thread_safety Thread-safe: Can be called from multiple threads, but typically
 *               called from a dedicated message processing thread
 * @note Handlers are called with the bus mutex unlocked to allow reentrancy
 * @param bus Message bus instance
 * @return Error code
 */
int idcu_msgbus_process(idcu_MsgBus* bus);

#ifdef __cplusplus
}
#endif

#endif
