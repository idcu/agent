#ifndef MSG_BUS_H
#define MSG_BUS_H

#include "scheduler/context.h"
#include "common/config.h"
#include "common/lock.h"

#define MSG_BATCH_MAX 32
#define MSG_ZEROCOPY_POOL_SIZE 256

typedef enum {
    MSG_PRIO_LOW = 0,
    MSG_PRIO_NORMAL,
    MSG_PRIO_HIGH,
    MSG_PRIO_REALTIME,
    MSG_PRIO_COUNT
} MsgPriority;

typedef struct {
    uint8_t *data;
    uint32_t size;
    uint32_t ref_count;
} ZeroCopyPayload;

typedef struct {
    StackContext data;
    ZeroCopyPayload *payload;
    uint32_t target_mod_id;
    uint32_t source_mod_id;
    MsgPriority priority;
    uint64_t timestamp;
    uint32_t retry_count;
} Message;

typedef struct {
    Message msgs[MSG_BATCH_MAX];
    uint32_t count;
} MessageBatch;

#define MSG_QUEUE_SIZE CONFIG_MAX_MSG

typedef struct {
    Message queue[MSG_PRIO_COUNT][MSG_QUEUE_SIZE];
    uint32_t head[MSG_PRIO_COUNT];
    uint32_t tail[MSG_PRIO_COUNT];
    uint32_t subs[16];
    Mutex lock;
    ZeroCopyPayload payload_pool[MSG_ZEROCOPY_POOL_SIZE];
    uint8_t payload_in_use[MSG_ZEROCOPY_POOL_SIZE];
    Mutex payload_lock;
} MessageBus;

void msg_bus_init(MessageBus *bus);
void msg_bus_destroy(MessageBus *bus);
int msg_send(MessageBus *bus, uint32_t src_mod, uint32_t dst_mod, MsgPriority prio, const StackContext *ctx);
int msg_recv(MessageBus *bus, uint32_t mod_id, Message *msg);
int msg_broadcast(MessageBus *bus, uint32_t src_mod, MsgPriority prio, const StackContext *ctx);
uint32_t msg_get_count(MessageBus *bus);
int msg_send_zerocopy(MessageBus *bus, uint32_t src_mod, uint32_t dst_mod, MsgPriority prio, const uint8_t *data, uint32_t size);
int msg_recv_zerocopy(MessageBus *bus, uint32_t mod_id, Message *msg);
void msg_release_payload(MessageBus *bus, ZeroCopyPayload *payload);
int msg_send_batch(MessageBus *bus, MessageBatch *batch);
int msg_recv_batch(MessageBus *bus, uint32_t mod_id, MessageBatch *batch, uint32_t max_count);

#endif
