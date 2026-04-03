#include "scheduler/msg_bus.h"
#include "common/error_code.h"
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

static uint64_t get_timestamp_ms(void)
{
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

void msg_bus_init(MessageBus *bus)
{
    if (!bus) {
        return;
    }

    memset(bus, 0, sizeof(MessageBus));
    mutex_init(&bus->lock);
    mutex_init(&bus->payload_lock);
}

void msg_bus_destroy(MessageBus *bus)
{
    if (!bus) {
        return;
    }
    
    int ret = mutex_lock(&bus->payload_lock);
    if (ret == ERR_SUCCESS) {
        for (uint32_t i = 0; i < MSG_ZEROCOPY_POOL_SIZE; i++) {
            if (bus->payload_in_use[i] && bus->payload_pool[i].data) {
                free(bus->payload_pool[i].data);
                bus->payload_pool[i].data = NULL;
            }
        }
        mutex_unlock(&bus->payload_lock);
    }
    
    mutex_destroy(&bus->payload_lock);
    mutex_destroy(&bus->lock);
}

static int is_queue_full(MessageBus *bus, MsgPriority prio)
{
    return ((bus->tail[prio] + 1) % MSG_QUEUE_SIZE) == bus->head[prio];
}

static int is_queue_empty(MessageBus *bus, MsgPriority prio)
{
    return bus->head[prio] == bus->tail[prio];
}

int msg_send(MessageBus *bus, uint32_t src_mod, uint32_t dst_mod, MsgPriority prio, const StackContext *ctx)
{
    if (!bus || !ctx) {
        return ERR_INVALID_PARAM;
    }

    if (prio >= MSG_PRIO_COUNT) {
        return ERR_INVALID_PARAM;
    }

    int ret = mutex_lock(&bus->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }

    if (is_queue_full(bus, prio)) {
        mutex_unlock(&bus->lock);
        return ERR_QUEUE_FULL;
    }

    Message *msg = &bus->queue[prio][bus->tail[prio]];
    memset(msg, 0, sizeof(Message));
    msg->data = *ctx;
    msg->source_mod_id = src_mod;
    msg->target_mod_id = dst_mod;
    msg->priority = prio;
    msg->timestamp = get_timestamp_ms();
    msg->retry_count = 0;
    msg->payload = NULL;

    bus->tail[prio] = (bus->tail[prio] + 1) % MSG_QUEUE_SIZE;

    mutex_unlock(&bus->lock);
    return ERR_SUCCESS;
}

int msg_recv(MessageBus *bus, uint32_t mod_id, Message *msg)
{
    if (!bus || !msg) {
        return ERR_INVALID_PARAM;
    }

    int ret = mutex_lock(&bus->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }

    for (int p = MSG_PRIO_COUNT - 1; p >= 0; p--) {
        MsgPriority prio = (MsgPriority)p;
        
        if (!is_queue_empty(bus, prio)) {
            Message *queue_msg = &bus->queue[prio][bus->head[prio]];
            
            if (queue_msg->target_mod_id == mod_id || queue_msg->target_mod_id == 0) {
                *msg = *queue_msg;
                bus->head[prio] = (bus->head[prio] + 1) % MSG_QUEUE_SIZE;
                mutex_unlock(&bus->lock);
                return ERR_SUCCESS;
            }
        }
    }

    mutex_unlock(&bus->lock);
    return ERR_QUEUE_EMPTY;
}

int msg_broadcast(MessageBus *bus, uint32_t src_mod, MsgPriority prio, const StackContext *ctx)
{
    return msg_send(bus, src_mod, 0, prio, ctx);
}

uint32_t msg_get_count(MessageBus *bus)
{
    if (!bus) {
        return 0;
    }

    int ret = mutex_lock(&bus->lock);
    if (ret != ERR_SUCCESS) {
        return 0;
    }

    uint32_t count = 0;
    for (int p = 0; p < MSG_PRIO_COUNT; p++) {
        if (bus->tail[p] >= bus->head[p]) {
            count += bus->tail[p] - bus->head[p];
        } else {
            count += MSG_QUEUE_SIZE - bus->head[p] + bus->tail[p];
        }
    }

    mutex_unlock(&bus->lock);
    return count;
}

static ZeroCopyPayload* allocate_payload(MessageBus *bus, uint32_t size)
{
    int ret = mutex_lock(&bus->payload_lock);
    if (ret != ERR_SUCCESS) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < MSG_ZEROCOPY_POOL_SIZE; i++) {
        if (!bus->payload_in_use[i]) {
            ZeroCopyPayload *payload = &bus->payload_pool[i];
            payload->data = (uint8_t*)malloc(size);
            if (!payload->data) {
                mutex_unlock(&bus->payload_lock);
                return NULL;
            }
            payload->size = size;
            payload->ref_count = 1;
            bus->payload_in_use[i] = 1;
            mutex_unlock(&bus->payload_lock);
            return payload;
        }
    }
    
    mutex_unlock(&bus->payload_lock);
    return NULL;
}

int msg_send_zerocopy(MessageBus *bus, uint32_t src_mod, uint32_t dst_mod, MsgPriority prio, const uint8_t *data, uint32_t size)
{
    if (!bus || !data || size == 0) {
        return ERR_INVALID_PARAM;
    }

    if (prio >= MSG_PRIO_COUNT) {
        return ERR_INVALID_PARAM;
    }

    int ret = mutex_lock(&bus->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }

    if (is_queue_full(bus, prio)) {
        mutex_unlock(&bus->lock);
        return ERR_QUEUE_FULL;
    }

    ZeroCopyPayload *payload = allocate_payload(bus, size);
    if (!payload) {
        mutex_unlock(&bus->lock);
        return ERR_NO_MEMORY;
    }

    memcpy(payload->data, data, size);

    Message *msg = &bus->queue[prio][bus->tail[prio]];
    memset(msg, 0, sizeof(Message));
    msg->source_mod_id = src_mod;
    msg->target_mod_id = dst_mod;
    msg->priority = prio;
    msg->timestamp = get_timestamp_ms();
    msg->retry_count = 0;
    msg->payload = payload;

    bus->tail[prio] = (bus->tail[prio] + 1) % MSG_QUEUE_SIZE;

    mutex_unlock(&bus->lock);
    return ERR_SUCCESS;
}

int msg_recv_zerocopy(MessageBus *bus, uint32_t mod_id, Message *msg)
{
    int ret = msg_recv(bus, mod_id, msg);
    if (ret == ERR_SUCCESS && msg->payload) {
        int lock_ret = mutex_lock(&bus->payload_lock);
        if (lock_ret == ERR_SUCCESS) {
            msg->payload->ref_count++;
            mutex_unlock(&bus->payload_lock);
        }
    }
    return ret;
}

void msg_release_payload(MessageBus *bus, ZeroCopyPayload *payload)
{
    if (!bus || !payload) {
        return;
    }

    int ret = mutex_lock(&bus->payload_lock);
    if (ret != ERR_SUCCESS) {
        return;
    }

    payload->ref_count--;
    if (payload->ref_count == 0) {
        if (payload->data) {
            free(payload->data);
            payload->data = NULL;
        }
        
        for (uint32_t i = 0; i < MSG_ZEROCOPY_POOL_SIZE; i++) {
            if (&bus->payload_pool[i] == payload) {
                bus->payload_in_use[i] = 0;
                break;
            }
        }
    }

    mutex_unlock(&bus->payload_lock);
}

int msg_send_batch(MessageBus *bus, MessageBatch *batch)
{
    if (!bus || !batch || batch->count == 0) {
        return ERR_INVALID_PARAM;
    }

    int ret = mutex_lock(&bus->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }

    for (uint32_t i = 0; i < batch->count; i++) {
        Message *src_msg = &batch->msgs[i];
        MsgPriority prio = src_msg->priority;
        
        if (prio >= MSG_PRIO_COUNT || is_queue_full(bus, prio)) {
            mutex_unlock(&bus->lock);
            return i > 0 ? (int)i : ERR_QUEUE_FULL;
        }

        Message *dst_msg = &bus->queue[prio][bus->tail[prio]];
        *dst_msg = *src_msg;
        dst_msg->timestamp = get_timestamp_ms();
        bus->tail[prio] = (bus->tail[prio] + 1) % MSG_QUEUE_SIZE;
    }

    mutex_unlock(&bus->lock);
    return (int)batch->count;
}

int msg_recv_batch(MessageBus *bus, uint32_t mod_id, MessageBatch *batch, uint32_t max_count)
{
    if (!bus || !batch) {
        return ERR_INVALID_PARAM;
    }

    if (max_count > MSG_BATCH_MAX) {
        max_count = MSG_BATCH_MAX;
    }

    int ret = mutex_lock(&bus->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }

    batch->count = 0;
    for (uint32_t i = 0; i < max_count; i++) {
        int found = 0;
        for (int p = MSG_PRIO_COUNT - 1; p >= 0; p--) {
            MsgPriority prio = (MsgPriority)p;
            
            if (!is_queue_empty(bus, prio)) {
                Message *queue_msg = &bus->queue[prio][bus->head[prio]];
                
                if (queue_msg->target_mod_id == mod_id || queue_msg->target_mod_id == 0) {
                    batch->msgs[i] = *queue_msg;
                    bus->head[prio] = (bus->head[prio] + 1) % MSG_QUEUE_SIZE;
                    batch->count++;
                    found = 1;
                    break;
                }
            }
        }
        if (!found) {
            break;
        }
    }

    mutex_unlock(&bus->lock);
    return (int)batch->count;
}
