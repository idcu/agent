#include "scheduler/msg_bus.h"
#include "common/error_code.h"
#include "monitor/metrics.h"
#include "utils/log.h"
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
    
    for (int p = 0; p < IDCU_MSG_PRIO_COUNT; p++) {
        idcu_mutex_init(&bus->prio_queues[p].lock);
    }
    
    idcu_mutex_init(&bus->payload_lock);
    
    for (uint32_t i = 0; i < IDCU_MSG_ZEROCOPY_POOL_SIZE - 1; i++) {
        bus->payload_in_use[i] = i + 1;
    }
    bus->payload_in_use[IDCU_MSG_ZEROCOPY_POOL_SIZE - 1] = 0xFF;
    bus->payload_free_head = 0;
}

void idcu_msg_bus_destroy(idcu_MessageBus *bus)
{
    if (!bus) {
        return;
    }
    
    int ret = idcu_mutex_lock(&bus->payload_lock);
    if (ret == IDCU_ERR_SUCCESS) {
        for (uint32_t i = 0; i < IDCU_MSG_ZEROCOPY_POOL_SIZE; i++) {
            if (bus->payload_pool[i].data) {
                free(bus->payload_pool[i].data);
                bus->payload_pool[i].data = NULL;
            }
        }
        idcu_mutex_unlock(&bus->payload_lock);
    }
    
    idcu_mutex_destroy(&bus->payload_lock);
    
    for (int p = 0; p < IDCU_MSG_PRIO_COUNT; p++) {
        idcu_mutex_destroy(&bus->prio_queues[p].lock);
    }
}

static int is_queue_full(idcu_PriorityQueue *q)
{
    return ((q->tail + 1) % IDCU_MSG_QUEUE_SIZE) == q->head;
}

static int is_queue_empty(idcu_PriorityQueue *q)
{
    return q->head == q->tail;
}

int idcu_msg_send(idcu_MessageBus *bus, uint32_t src_mod, uint32_t dst_mod, idcu_MsgPriority prio, const idcu_StackContext *ctx)
{
    if (!bus || !ctx) {
        IDCU_LOG_ERROR("msg_send failed: invalid parameters (bus=%p, ctx=%p)", (void*)bus, (void*)ctx);
        return IDCU_ERR_INVALID_PARAM;
    }

    if (prio >= IDCU_MSG_PRIO_COUNT) {
        IDCU_LOG_ERROR("msg_send failed: invalid priority %d", prio);
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_PriorityQueue *q = &bus->prio_queues[prio];
    int ret = idcu_mutex_lock(&q->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("msg_send failed: lock error, code=%d (%s)", ret, idcu_err_to_str(ret));
        return ret;
    }

    if (is_queue_full(q)) {
        idcu_mutex_unlock(&q->lock);
        IDCU_LOG_WARN("msg_send failed: queue full (prio=%d)", prio);
        return IDCU_ERR_QUEUE_FULL;
    }

    idcu_Message *msg = &q->queue[q->tail];
    memset(msg, 0, sizeof(idcu_Message));
    msg->data = *ctx;
    msg->source_mod_id = src_mod;
    msg->target_mod_id = dst_mod;
    msg->priority = prio;
    msg->timestamp = get_timestamp_ms();
    msg->retry_count = 0;
    msg->payload = NULL;

    q->tail = (q->tail + 1) % IDCU_MSG_QUEUE_SIZE;

    idcu_global_metrics_inc(IDCU_METRIC_MSG_SENT, 1);

    idcu_mutex_unlock(&q->lock);
    IDCU_LOG_DEBUG("msg_send succeeded: src=%u, dst=%u, prio=%d", src_mod, dst_mod, prio);
    return IDCU_ERR_SUCCESS;
}

int idcu_msg_recv(idcu_MessageBus *bus, uint32_t mod_id, idcu_Message *msg)
{
    if (!bus || !msg) {
        IDCU_LOG_ERROR("msg_recv failed: invalid parameters (bus=%p, msg=%p)", (void*)bus, (void*)msg);
        return IDCU_ERR_INVALID_PARAM;
    }

    for (int p = IDCU_MSG_PRIO_COUNT - 1; p >= 0; p--) {
        idcu_MsgPriority prio = (idcu_MsgPriority)p;
        idcu_PriorityQueue *q = &bus->prio_queues[prio];
        
        int ret = idcu_mutex_lock(&q->lock);
        if (ret != IDCU_ERR_SUCCESS) {
            continue;
        }
        
        if (!is_queue_empty(q)) {
            idcu_Message *queue_msg = &q->queue[q->head];
            
            if (queue_msg->target_mod_id == mod_id || queue_msg->target_mod_id == 0) {
                *msg = *queue_msg;
                q->head = (q->head + 1) % IDCU_MSG_QUEUE_SIZE;

                idcu_global_metrics_inc(IDCU_METRIC_MSG_RECEIVED, 1);

                idcu_mutex_unlock(&q->lock);
                IDCU_LOG_DEBUG("msg_recv succeeded: mod=%u, src=%u, prio=%d", mod_id, queue_msg->source_mod_id, prio);
                return IDCU_ERR_SUCCESS;
            }
        }
        
        idcu_mutex_unlock(&q->lock);
    }

    return IDCU_ERR_QUEUE_EMPTY;
}

int idcu_msg_broadcast(idcu_MessageBus *bus, uint32_t src_mod, idcu_MsgPriority prio, const idcu_StackContext *ctx)
{
    int ret = idcu_msg_send(bus, src_mod, 0, prio, ctx);
    if (ret == IDCU_ERR_SUCCESS) {
        idcu_global_metrics_inc(IDCU_METRIC_MSG_BROADCAST, 1);
    }
    return ret;
}

uint32_t idcu_msg_get_count(idcu_MessageBus *bus)
{
    if (!bus) {
        return 0;
    }

    uint32_t count = 0;
    for (int p = 0; p < IDCU_MSG_PRIO_COUNT; p++) {
        idcu_PriorityQueue *q = &bus->prio_queues[p];
        int ret = idcu_mutex_lock(&q->lock);
        if (ret != IDCU_ERR_SUCCESS) {
            continue;
        }
        
        if (q->tail >= q->head) {
            count += q->tail - q->head;
        } else {
            count += IDCU_MSG_QUEUE_SIZE - q->head + q->tail;
        }
        
        idcu_mutex_unlock(&q->lock);
    }

    return count;
}

static idcu_ZeroCopyPayload* allocate_payload(idcu_MessageBus *bus, uint32_t size)
{
    int ret = idcu_mutex_lock(&bus->payload_lock);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("allocate_payload failed: lock error, code=%d (%s)", ret, idcu_err_to_str(ret));
        return NULL;
    }
    
    if (bus->payload_free_head == 0xFF) {
        idcu_mutex_unlock(&bus->payload_lock);
        IDCU_LOG_WARN("allocate_payload failed: no available slots in payload pool");
        return NULL;
    }
    
    uint32_t slot = bus->payload_free_head;
    bus->payload_free_head = bus->payload_in_use[slot];
    
    idcu_ZeroCopyPayload *payload = &bus->payload_pool[slot];
    payload->data = (uint8_t*)malloc(size);
    if (!payload->data) {
        bus->payload_in_use[slot] = (uint8_t)bus->payload_free_head;
        bus->payload_free_head = slot;
        idcu_mutex_unlock(&bus->payload_lock);
        IDCU_LOG_ERROR("allocate_payload failed: out of memory (size=%u)", size);
        return NULL;
    }
    payload->size = size;
    payload->ref_count = 1;
    
    idcu_mutex_unlock(&bus->payload_lock);
    IDCU_LOG_DEBUG("allocate_payload succeeded: slot=%u, size=%u", slot, size);
    return payload;
}

int idcu_msg_send_zerocopy(idcu_MessageBus *bus, uint32_t src_mod, uint32_t dst_mod, idcu_MsgPriority prio, const uint8_t *data, uint32_t size)
{
    if (!bus || !data || size == 0) {
        IDCU_LOG_ERROR("msg_send_zerocopy failed: invalid parameters (bus=%p, data=%p, size=%u)", (void*)bus, (void*)data, size);
        return IDCU_ERR_INVALID_PARAM;
    }

    if (prio >= IDCU_MSG_PRIO_COUNT) {
        IDCU_LOG_ERROR("msg_send_zerocopy failed: invalid priority %d", prio);
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_ZeroCopyPayload *payload = allocate_payload(bus, size);
    if (!payload) {
        IDCU_LOG_ERROR("msg_send_zerocopy failed: allocate payload failed");
        return IDCU_ERR_NO_MEMORY;
    }

    memcpy(payload->data, data, size);

    idcu_PriorityQueue *q = &bus->prio_queues[prio];
    int ret = idcu_mutex_lock(&q->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        idcu_msg_release_payload(bus, payload);
        IDCU_LOG_ERROR("msg_send_zerocopy failed: lock error, code=%d (%s)", ret, idcu_err_to_str(ret));
        return ret;
    }

    if (is_queue_full(q)) {
        idcu_mutex_unlock(&q->lock);
        idcu_msg_release_payload(bus, payload);
        IDCU_LOG_WARN("msg_send_zerocopy failed: queue full (prio=%d)", prio);
        return IDCU_ERR_QUEUE_FULL;
    }

    idcu_Message *msg = &q->queue[q->tail];
    memset(msg, 0, sizeof(idcu_Message));
    msg->source_mod_id = src_mod;
    msg->target_mod_id = dst_mod;
    msg->priority = prio;
    msg->timestamp = get_timestamp_ms();
    msg->retry_count = 0;
    msg->payload = payload;

    q->tail = (q->tail + 1) % IDCU_MSG_QUEUE_SIZE;

    idcu_global_metrics_inc(IDCU_METRIC_MSG_SENT, 1);
    idcu_global_metrics_inc(IDCU_METRIC_MSG_ZEROCOPY_SENT, 1);

    idcu_mutex_unlock(&q->lock);
    IDCU_LOG_DEBUG("msg_send_zerocopy succeeded: src=%u, dst=%u, prio=%d, size=%u", src_mod, dst_mod, prio, size);
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
        
        uint32_t slot = (uint32_t)(payload - bus->payload_pool);
        bus->payload_in_use[slot] = (uint8_t)bus->payload_free_head;
        bus->payload_free_head = slot;
    }

    idcu_mutex_unlock(&bus->payload_lock);
}

int idcu_msg_send_batch(idcu_MessageBus *bus, idcu_MessageBatch *batch)
{
    if (!bus || !batch || batch->count == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    uint32_t sent = 0;
    idcu_MsgPriority last_prio = IDCU_MSG_PRIO_COUNT;
    idcu_PriorityQueue *current_q = NULL;
    int lock_held = 0;

    for (uint32_t i = 0; i < batch->count; i++) {
        idcu_Message *src_msg = &batch->msgs[i];
        idcu_MsgPriority prio = src_msg->priority;
        
        if (prio >= IDCU_MSG_PRIO_COUNT) {
            if (lock_held) {
                idcu_mutex_unlock(&current_q->lock);
                lock_held = 0;
            }
            return sent > 0 ? (int)sent : IDCU_ERR_INVALID_PARAM;
        }
        
        if (prio != last_prio) {
            if (lock_held) {
                idcu_mutex_unlock(&current_q->lock);
                lock_held = 0;
            }
            current_q = &bus->prio_queues[prio];
            int ret = idcu_mutex_lock(&current_q->lock);
            if (ret != IDCU_ERR_SUCCESS) {
                return sent > 0 ? (int)sent : ret;
            }
            lock_held = 1;
            last_prio = prio;
        }
        
        if (is_queue_full(current_q)) {
            idcu_mutex_unlock(&current_q->lock);
            idcu_global_metrics_inc(IDCU_METRIC_MSG_SENT, sent);
            return sent > 0 ? (int)sent : IDCU_ERR_QUEUE_FULL;
        }

        idcu_Message *dst_msg = &current_q->queue[current_q->tail];
        *dst_msg = *src_msg;
        dst_msg->timestamp = get_timestamp_ms();
        current_q->tail = (current_q->tail + 1) % IDCU_MSG_QUEUE_SIZE;
        sent++;
    }
    
    if (lock_held) {
        idcu_mutex_unlock(&current_q->lock);
    }

    idcu_global_metrics_inc(IDCU_METRIC_MSG_SENT, sent);
    idcu_global_metrics_inc(IDCU_METRIC_MSG_BATCH_SENT, 1);
    return (int)sent;
}

int idcu_msg_recv_batch(idcu_MessageBus *bus, uint32_t mod_id, idcu_MessageBatch *batch, uint32_t max_count)
{
    if (!bus || !batch) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (max_count > IDCU_MSG_BATCH_MAX) {
        max_count = IDCU_MSG_BATCH_MAX;
    }

    batch->count = 0;
    for (uint32_t i = 0; i < max_count; i++) {
        int found = 0;
        for (int p = IDCU_MSG_PRIO_COUNT - 1; p >= 0; p--) {
            idcu_MsgPriority prio = (idcu_MsgPriority)p;
            idcu_PriorityQueue *q = &bus->prio_queues[prio];
            
            int ret = idcu_mutex_lock(&q->lock);
            if (ret != IDCU_ERR_SUCCESS) {
                continue;
            }
            
            if (!is_queue_empty(q)) {
                idcu_Message *queue_msg = &q->queue[q->head];
                
                if (queue_msg->target_mod_id == mod_id || queue_msg->target_mod_id == 0) {
                    batch->msgs[i] = *queue_msg;
                    q->head = (q->head + 1) % IDCU_MSG_QUEUE_SIZE;
                    batch->count++;
                    found = 1;
                    idcu_mutex_unlock(&q->lock);
                    break;
                }
            }
            
            idcu_mutex_unlock(&q->lock);
        }
        if (!found) {
            break;
        }
    }

    if (batch->count > 0) {
        idcu_global_metrics_inc(IDCU_METRIC_MSG_RECEIVED, batch->count);
        idcu_global_metrics_inc(IDCU_METRIC_MSG_BATCH_RECEIVED, 1);
    }

    return (int)batch->count;
}
