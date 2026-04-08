#ifndef IDCU_SCHEDULER_MSG_BUS_H
#define IDCU_SCHEDULER_MSG_BUS_H

#include "context.h"
#include "idcu/common/config.h"
#include "idcu/common/lock.h"

#define IDCU_MSG_BATCH_MAX          32
#define IDCU_MSG_ZEROCOPY_POOL_SIZE 256
#define IDCU_MSG_MOD_INDEX_SIZE     256
#define IDCU_MSG_MAX_RETRIES        3
#define IDCU_MSG_RETRY_DELAY_MS     100
#define IDCU_MSG_PENDING_QUEUE_SIZE 64

typedef enum
{
    IDCU_MSG_PRIO_LOW = 0,
    IDCU_MSG_PRIO_NORMAL,
    IDCU_MSG_PRIO_HIGH,
    IDCU_MSG_PRIO_REALTIME,
    IDCU_MSG_PRIO_COUNT
} idcu_MsgPriority;

typedef struct
{
    uint8_t* data;
    uint32_t size;
    uint32_t ref_count;
} idcu_ZeroCopyPayload;

typedef struct
{
    idcu_StackContext     data;
    idcu_ZeroCopyPayload* payload;
    uint32_t              target_mod_id;
    uint32_t              source_mod_id;
    idcu_MsgPriority      priority;
    uint64_t              timestamp;
    uint32_t              retry_count;
    uint64_t              next_retry_time;
    int                   is_reliable;
} idcu_Message;

typedef struct
{
    idcu_Message msgs[IDCU_MSG_BATCH_MAX];
    uint32_t     count;
} idcu_MessageBatch;

#define IDCU_MSG_QUEUE_SIZE IDCU_CONFIG_MAX_MSG

typedef struct
{
    idcu_Message queue[IDCU_MSG_QUEUE_SIZE];
    uint32_t     head;
    uint32_t     tail;
    idcu_Mutex   lock;
} idcu_PriorityQueue;

typedef void (*idcu_MessageFailureCallback)(uint32_t src_mod, uint32_t dst_mod,
                                            const idcu_Message* msg, void* user_data);

typedef struct
{
    idcu_PriorityQueue          prio_queues[IDCU_MSG_PRIO_COUNT];
    uint32_t                    subs[16];
    idcu_ZeroCopyPayload        payload_pool[IDCU_MSG_ZEROCOPY_POOL_SIZE];
    uint8_t                     payload_in_use[IDCU_MSG_ZEROCOPY_POOL_SIZE];
    uint32_t                    payload_free_head;
    idcu_Mutex                  payload_lock;
    idcu_Message                pending_retry_queue[IDCU_MSG_PENDING_QUEUE_SIZE];
    uint32_t                    pending_retry_count;
    idcu_Mutex                  retry_lock;
    idcu_MessageFailureCallback failure_callback;
    void*                       failure_callback_data;
} idcu_MessageBus;

void idcu_msg_bus_init(idcu_MessageBus* bus);
void idcu_msg_bus_destroy(idcu_MessageBus* bus);
int  idcu_msg_send(idcu_MessageBus* bus, uint32_t src_mod, uint32_t dst_mod, idcu_MsgPriority prio,
                   const idcu_StackContext* ctx);
int  idcu_msg_recv(idcu_MessageBus* bus, uint32_t mod_id, idcu_Message* msg);
int  idcu_msg_broadcast(idcu_MessageBus* bus, uint32_t src_mod, idcu_MsgPriority prio,
                        const idcu_StackContext* ctx);
uint32_t idcu_msg_get_count(idcu_MessageBus* bus);
int      idcu_msg_send_zerocopy(idcu_MessageBus* bus, uint32_t src_mod, uint32_t dst_mod,
                                idcu_MsgPriority prio, const uint8_t* data, uint32_t size);
int      idcu_msg_recv_zerocopy(idcu_MessageBus* bus, uint32_t mod_id, idcu_Message* msg);
void     idcu_msg_release_payload(idcu_MessageBus* bus, idcu_ZeroCopyPayload* payload);
int      idcu_msg_send_batch(idcu_MessageBus* bus, idcu_MessageBatch* batch);
int      idcu_msg_recv_batch(idcu_MessageBus* bus, uint32_t mod_id, idcu_MessageBatch* batch,
                             uint32_t max_count);
int      idcu_msg_send_reliable(idcu_MessageBus* bus, uint32_t src_mod, uint32_t dst_mod,
                                idcu_MsgPriority prio, const idcu_StackContext* ctx);
void     idcu_msg_process_retries(idcu_MessageBus* bus);
int      idcu_msg_set_failure_callback(idcu_MessageBus* bus, idcu_MessageFailureCallback callback,
                                       void* user_data);
int      idcu_msg_mark_for_retry(idcu_MessageBus* bus, idcu_Message* msg);

#endif  // IDCU_SCHEDULER_MSG_BUS_H
