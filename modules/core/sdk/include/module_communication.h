#ifndef IDCU_SDK_MODULE_COMMUNICATION_H
#define IDCU_SDK_MODULE_COMMUNICATION_H

#include "idcu/common/error_code.h"
#include "msg_bus.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define IDCU_MSG_PAYLOAD_MAX 4096
#define IDCU_MSG_TOPIC_MAX   256

    typedef enum
    {
        IDCU_MSG_TYPE_REQUEST      = 0,
        IDCU_MSG_TYPE_RESPONSE     = 1,
        IDCU_MSG_TYPE_NOTIFICATION = 2,
        IDCU_MSG_TYPE_EVENT        = 3
    } idcu_MessageType;

    typedef struct
    {
        uint64_t         request_id;
        const char*      topic;
        idcu_MessageType type;
        uint32_t         source_module_id;
        uint32_t         target_module_id;
        const char*      source_module_name;
        const char*      target_module_name;
        idcu_MsgPriority priority;
        const uint8_t*   payload;
        uint32_t         payload_size;
        int64_t          timestamp;
    } idcu_ModuleMessage;

    typedef struct idcu_module_comm_context idcu_ModuleCommContext;

    typedef void (*idcu_ModuleMessageHandler)(idcu_ModuleCommContext*   ctx,
                                              const idcu_ModuleMessage* msg, void* user_data);

    typedef void (*idcu_ModuleResponseCallback)(idcu_ModuleCommContext*   ctx,
                                                const idcu_ModuleMessage* response, int error_code,
                                                void* user_data);

    idcu_ModuleCommContext* idcu_module_comm_create_context(void);
    void                    idcu_module_comm_destroy_context(idcu_ModuleCommContext* ctx);

    void* idcu_module_comm_get_user_data(idcu_ModuleCommContext* ctx);
    void  idcu_module_comm_set_user_data(idcu_ModuleCommContext* ctx, void* user_data);

    void idcu_module_comm_set_message_bus(idcu_ModuleCommContext* ctx, idcu_MessageBus* bus);
    void idcu_module_comm_set_module_info(idcu_ModuleCommContext* ctx, uint32_t module_id,
                                          const char* module_name);

    uint32_t    idcu_module_comm_get_module_id(idcu_ModuleCommContext* ctx);
    const char* idcu_module_comm_get_module_name(idcu_ModuleCommContext* ctx);

    int idcu_module_comm_send_request(idcu_ModuleCommContext* ctx, const char* target_module,
                                      const char* topic, idcu_MsgPriority priority,
                                      const uint8_t* payload, uint32_t payload_size,
                                      idcu_ModuleResponseCallback callback,
                                      void* callback_user_data, uint64_t* out_request_id);

    int idcu_module_comm_send_response(idcu_ModuleCommContext*   ctx,
                                       const idcu_ModuleMessage* request, int error_code,
                                       const uint8_t* payload, uint32_t payload_size);

    int idcu_module_comm_send_notification(idcu_ModuleCommContext* ctx, const char* target_module,
                                           const char* topic, idcu_MsgPriority priority,
                                           const uint8_t* payload, uint32_t payload_size);

    int idcu_module_comm_broadcast_event(idcu_ModuleCommContext* ctx, const char* topic,
                                         idcu_MsgPriority priority, const uint8_t* payload,
                                         uint32_t payload_size);

    int idcu_module_comm_register_topic_handler(idcu_ModuleCommContext* ctx, const char* topic,
                                                idcu_ModuleMessageHandler handler, void* user_data);

    int idcu_module_comm_unregister_topic_handler(idcu_ModuleCommContext* ctx, const char* topic);

    int idcu_module_comm_cancel_request(idcu_ModuleCommContext* ctx, uint64_t request_id);

    void idcu_module_message_free(idcu_ModuleMessage* msg);

#ifdef __cplusplus
}
#endif

#endif
