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

void idcu_msg_bus_init(idcu_MessageBus *bus)
{
    if (!bus) {
        return;
    }

    memset(bus, 0, sizeof(idcu_MessageBus));
    idcu_mutex_init(&bus->lock);
    idcu_mutex_init(&bus->payload_lock);
}

void idcu_msg_bus_destroy(idcu_MessageBus *bus)
{
    if (!bus) {
        return;
    }
    
    int ret = idcu_mutex_lock(&bus->payload_lock);
    if (ret == IDCU_ERR_SUCCESS) {
        for (uint32_t i = 0; i < IDCU_MSG_ZEROCOPY_POOL_SIZE; i++) {
            if (bus->payload_in_use[i] && bus->payload_pool[i].data) {
                free(bus->payload_pool[i].data);
                bus->payload_pool[i].data = NULL;
            }
        }
        idcu_mutex_unlock(&bus->payload_lock);
    }
    
    idcu_mutex_destroy(&bus->payload_lock);
    idcu_mutex_destroy(&bus->lock);
}

static int is_queue_full(idcu_MessageBus *bus, idcu_MsgPriority prio)
{
    return ((bus->tail[prio] + 1) % IDCU_MSG_QUEUE_SIZE) == bus->head[prio];
}

static int is_queue_empty(idcu_MessageBus *bus, idcu_MsgPriority prio)
{
    return bus->head[prio] == bus->tail[prio];
}

int idcu_msg_send(idcu_MessageBus *bus, uint32_t src_mod, uint32_t dst_mod, idcu_MsgPriority prio, const idcu_StackContext *ctx)
{
    if (!bus || !ctx) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (prio >= IDCU_MSG_PRIO_COUNT) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&bus->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    if (is_queue_full(bus, prio)) {
        idcu_mutex_unlock(&bus->lock);
        return IDCU_ERR_QUEUE_FULL;
    }

    idcu_Message *msg = &bus->queue[prio][bus->tail[prio]];
    memset(msg, 0, sizeof(idcu_Message));
    msg->data = *ctx;
    msg->source_mod_id = src_mod;
    msg->target_mod_id = dst_mod;
    msg->priority = prio;
    msg->timestamp = get_timestamp_ms();
    msg->retry_count = 0;
    msg->payload = NULL;

    bus->tail[prio] = (bus->tail[prio] + 1) % IDCU_MSG_QUEUE_SIZE;

    idcu_mutex_unlock(&bus->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_msg_recv(idcu_MessageBus *bus, uint32_t mod_id, idcu_Message *msg)
{
    if (!bus || !msg) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&bus->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    for (int p = IDCU_MSG_PRIO_COUNT - 1; p >= 0; p--) {
        idcu_MsgPriority prio = (idcu_MsgPriority)p;
        
        if (!is_queue_empty(bus, prio)) {
            idcu_Message *queue_msg = &bus->queue[prio][bus->head[prio]];
            
            if (queue_msg->target_mod_id == mod_id || queue_msg->target_mod_id == 0) {
                *msg = *queue_msg;
                bus->head[prio] = (bus->head[prio] + 1) % IDCU_MSG_QUEUE_SIZE;
                idcu_mutex_unlock(&bus->lock);
                return IDCU_ERR_SUCCESS;
            }
        }
    }

    idcu_mutex_unlock(&bus->lock);
    return IDCU_ERR_QUEUE_EMPTY;
}

int idcu_msg_broadcast(idcu_MessageBus *bus, uint32_t src_mod, idcu_MsgPriority prio, const idcu_StackContext *ctx)
{
    return idcu_msg_send(bus, src_mod, 0, prio, ctx);
}

uint32_t idcu_msg_get_count(idcu_MessageBus *bus)
{
    if (!bus) {
        return 0;
    }

    int ret = idcu_mutex_lock(&bus->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return 0;
    }

    uint32_t count = 0;
    for (int p = 0; p < IDCU_MSG_PRIO_COUNT; p++) {
        if (bus->tail[p] >= bus->head[p]) {
            count += bus->tail[p] - bus->head[p];
        } else {
            count += IDCU_MSG_QUEUE_SIZE - bus->head[p] + bus->tail[p];
        }
    }

    idcu_mutex_unlock(&bus->lock);
    return count;
}

static idcu_ZeroCopyPayload* allocate_payload(idcu_MessageBus *bus, uint32_t size)
{
    int ret = idcu_mutex_lock(&bus->payload_lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < IDCU_MSG_ZEROCOPY_POOL_SIZE; i++) {
        if (!bus->payload_in_use[i]) {
            idcu_ZeroCopyPayload *payload = &bus->payload_pool[i];
            payload->data = (uint8_t*)malloc(size);
            if (!payload->data) {
                idcu_mutex_unlock(&bus->payload_lock);
                return NULL;
            }
            payload->size = size;
            payload->ref_count = 1;
            bus->payload_in_use[i] = 1;
            idcu_mutex_unlock(&bus->payload_lock);
            return payload;
        }
    }
    
    idcu_mutex_unlock(&bus->payload_lock);
    return NULL;
}

int idcu_msg_send_zerocopy(idcu_MessageBus *bus, uint32_t src_mod, uint32_t dst_mod, idcu_MsgPriority prio, const uint8_t *data, uint32_t size)
{
    if (!bus || !data || size == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (prio >= IDCU_MSG_PRIO_COUNT) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&bus->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    if (is_queue_full(bus, prio)) {
        idcu_mutex_unlock(&bus->lock);
        return IDCU_ERR_QUEUE_FULL;
    }

    idcu_ZeroCopyPayload *payload = allocate_payload(bus, size);
    if (!payload) {
        idcu_mutex_unlock(&bus->lock);
        return IDCU_ERR_NO_MEMORY;
    }

    memcpy(payload->data, data, size);

    idcu_Message *msg = &bus->queue[prio][bus->tail[prio]];
    memset(msg, 0, sizeof(idcu_Message));
    msg->source_mod_id = src_mod;
    msg->target_mod_id = dst_mod;
    msg->priority = prio;
    msg->timestamp = get_timestamp_ms();
    msg->retry_count = 0;
    msg->payload = payload;

    bus->tail[prio] = (bus->tail[prio] + 1) % IDCU_MSG_QUEUE_SIZE;

    idcu_mutex_unlock(&bus->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_msg_recv_zerocopy(idcu_MessageBus *bus, uint32_t mod_id, idcu_Message *msg)
{
    int ret = idcu_msg_recv(bus, mod_id, msg);
    if (ret == IDCU_ERR_SUCCESS && msg->payload) {
        int lock_ret = idcu_mutex_lock(&bus->payload_lock);
        if (lock_ret == IDCU_ERR_SUCCESS) {
            msg->payload->ref_count++;
            idcu_mutex_unlock(&bus->payload_lock);
        }
    }
    return ret;
}

void idcu_msg_release_payload(idcu_MessageBus *bus, idcu_ZeroCopyPayload *payload)
{
    if (!bus || !payload) {
        return;
    }

    int ret = idcu_mutex_lock(&bus->payload_lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return;
    }

    payload->ref_count--;
    if (payload->ref_count == 0) {
        if (payload->data) {
            free(payload->data);
            payload->data = NULL;
        }
        
        for (uint32_t i = 0; i < IDCU_MSG_ZEROCOPY_POOL_SIZE; i++) {
            if (&bus->payload_pool[i] == payload) {
                bus->payload_in_use[i] = 0;
                break;
            }
        }
    }

    idcu_mutex_unlock(&bus->payload_lock);
}

int idcu_msg_send_batch(idcu_MessageBus *bus, idcu_MessageBatch *batch)
{
    if (!bus || !batch || batch->count == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&bus->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    for (uint32_t i = 0; i < batch->count; i++) {
        idcu_Message *src_msg = &batch->msgs[i];
        idcu_MsgPriority prio = src_msg->priority;
        
        if (prio >= IDCU_MSG_PRIO_COUNT || is_queue_full(bus, prio)) {
            idcu_mutex_unlock(&bus->lock);
            return i > 0 ? (int)i : IDCU_ERR_QUEUE_FULL;
        }

        idcu_Message *dst_msg = &bus->queue[prio][bus->tail[prio]];
        *dst_msg = *src_msg;
        dst_msg->timestamp = get_timestamp_ms();
        bus->tail[prio] = (bus->tail[prio] + 1) % IDCU_MSG_QUEUE_SIZE;
    }

    idcu_mutex_unlock(&bus->lock);
    return (int)batch->count;
}

int idcu_msg_recv_batch(idcu_MessageBus *bus, uint32_t mod_id, idcu_MessageBatch *batch, uint32_t max_count)
{
    if (!bus || !batch) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (max_count > IDCU_MSG_BATCH_MAX) {
        max_count = IDCU_MSG_BATCH_MAX;
    }

    int ret = idcu_mutex_lock(&bus->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    batch->count = 0;
    for (uint32_t i = 0; i < max_count; i++) {
        int found = 0;
        for (int p = IDCU_MSG_PRIO_COUNT - 1; p >= 0; p--) {
            idcu_MsgPriority prio = (idcu_MsgPriority)p;
            
            if (!is_queue_empty(bus, prio)) {
                idcu_Message *queue_msg = &bus->queue[prio][bus->head[prio]];
                
                if (queue_msg->target_mod_id == mod_id || queue_msg->target_mod_id == 0) {
                    batch->msgs[i] = *queue_msg;
                    bus->head[prio] = (bus->head[prio] + 1) % IDCU_MSG_QUEUE_SIZE;
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

    idcu_mutex_unlock(&bus->lock);
    return (int)batch->count;
}
