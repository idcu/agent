#include <idcu/msgbus/msgbus.h>
#include <idcu/common/vector.h>
#include <idcu/common/lock.h>
#include <stdlib.h>
#include <string.h>

typedef struct idcu_Msg {
    idcu_MsgTopic topic;
    void* data;
    size_t data_size;
    idcu_MsgPriority priority;
} idcu_Msg;

typedef struct idcu_SubscriberList {
    idcu_Vector* subscribers;
} idcu_SubscriberList;

struct idcu_MsgBus {
    idcu_Mutex mutex;
    idcu_Vector* topic_queues[IDCU_MSG_MAX_TOPICS];
    idcu_SubscriberList topic_subscribers[IDCU_MSG_MAX_TOPICS];
    idcu_Vector* all_subscribers;
};

struct idcu_MsgSubscriber {
    idcu_MsgBus* bus;
    idcu_MsgTopic topic;
    idcu_MsgHandler handler;
    void* user_data;
    int active;
};

static void msg_dtor(void* element) {
    idcu_Msg* msg = *(idcu_Msg**)element;
    if (msg) {
        if (msg->data) {
            free(msg->data);
        }
        free(msg);
    }
}

static void insert_sorted_by_priority(idcu_Vector* vec, idcu_Msg* msg) {
    size_t n = idcu_vector_size(vec);
    if (n == 0) {
        idcu_vector_push_back(vec, &msg);
        return;
    }

    size_t insert_pos = 0;
    for (; insert_pos < n; insert_pos++) {
        idcu_Msg** current = (idcu_Msg**)idcu_vector_get(vec, insert_pos);
        if (*current && (*current)->priority < msg->priority) {
            break;
        }
    }

    idcu_vector_insert(vec, insert_pos, &msg);
}

int idcu_msgbus_init(idcu_MsgBus** bus) {
    if (!bus) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_MsgBus* b = (idcu_MsgBus*)malloc(sizeof(idcu_MsgBus));
    if (!b) {
        return IDCU_ERR_MEMORY;
    }

    int ret = idcu_mutex_init(&b->mutex);
    if (ret != IDCU_ERR_OK) {
        free(b);
        return ret;
    }

    for (int i = 0; i < IDCU_MSG_MAX_TOPICS; i++) {
        b->topic_queues[i] = NULL;
        b->topic_subscribers[i].subscribers = NULL;
    }

    b->all_subscribers = (idcu_Vector*)malloc(sizeof(idcu_Vector));
    if (!b->all_subscribers) {
        idcu_mutex_destroy(&b->mutex);
        free(b);
        return IDCU_ERR_MEMORY;
    }

    ret = idcu_vector_init(b->all_subscribers, sizeof(idcu_MsgSubscriber*), 8);
    if (ret != IDCU_ERR_OK) {
        free(b->all_subscribers);
        idcu_mutex_destroy(&b->mutex);
        free(b);
        return ret;
    }

    *bus = b;
    return IDCU_ERR_OK;
}

void idcu_msgbus_destroy(idcu_MsgBus* bus) {
    if (!bus) {
        return;
    }

    idcu_mutex_lock(&bus->mutex);

    for (int i = 0; i < IDCU_MSG_MAX_TOPICS; i++) {
        if (bus->topic_queues[i]) {
            for (size_t j = 0; j < idcu_vector_size(bus->topic_queues[i]); j++) {
                idcu_Msg** msg_ptr = (idcu_Msg**)idcu_vector_get(bus->topic_queues[i], j);
                if (msg_ptr && *msg_ptr) {
                    if ((*msg_ptr)->data) {
                        free((*msg_ptr)->data);
                    }
                    free(*msg_ptr);
                }
            }
            idcu_vector_destroy(bus->topic_queues[i]);
            free(bus->topic_queues[i]);
        }

        if (bus->topic_subscribers[i].subscribers) {
            idcu_vector_destroy(bus->topic_subscribers[i].subscribers);
            free(bus->topic_subscribers[i].subscribers);
        }
    }

    for (size_t i = 0; i < idcu_vector_size(bus->all_subscribers); i++) {
        idcu_MsgSubscriber** sub_ptr = (idcu_MsgSubscriber**)idcu_vector_get(bus->all_subscribers, i);
        if (sub_ptr && *sub_ptr) {
            free(*sub_ptr);
        }
    }
    idcu_vector_destroy(bus->all_subscribers);
    free(bus->all_subscribers);

    idcu_mutex_unlock(&bus->mutex);
    idcu_mutex_destroy(&bus->mutex);
    free(bus);
}

int idcu_msgbus_publish(idcu_MsgBus* bus,
                         idcu_MsgTopic topic,
                         const void* data,
                         size_t data_size,
                         idcu_MsgPriority priority) {
    if (!bus) {
        return IDCU_ERR_INVALID_ARG;
    }

    if (topic >= IDCU_MSG_MAX_TOPICS) {
        return IDCU_ERR_INVALID_ARG;
    }

    if (data_size > IDCU_MSG_MAX_SIZE) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_Msg* msg = (idcu_Msg*)malloc(sizeof(idcu_Msg));
    if (!msg) {
        return IDCU_ERR_MEMORY;
    }

    msg->topic = topic;
    msg->priority = priority;
    msg->data_size = data_size;

    if (data && data_size > 0) {
        msg->data = malloc(data_size);
        if (!msg->data) {
            free(msg);
            return IDCU_ERR_MEMORY;
        }
        memcpy(msg->data, data, data_size);
    } else {
        msg->data = NULL;
    }

    idcu_mutex_lock(&bus->mutex);

    if (!bus->topic_queues[topic]) {
        bus->topic_queues[topic] = (idcu_Vector*)malloc(sizeof(idcu_Vector));
        if (!bus->topic_queues[topic]) {
            idcu_mutex_unlock(&bus->mutex);
            if (msg->data) {
                free(msg->data);
            }
            free(msg);
            return IDCU_ERR_MEMORY;
        }
        int ret = idcu_vector_init(bus->topic_queues[topic], sizeof(idcu_Msg*), 8);
        if (ret != IDCU_ERR_OK) {
            idcu_mutex_unlock(&bus->mutex);
            free(bus->topic_queues[topic]);
            if (msg->data) {
                free(msg->data);
            }
            free(msg);
            return ret;
        }
    }

    insert_sorted_by_priority(bus->topic_queues[topic], msg);

    idcu_mutex_unlock(&bus->mutex);

    return IDCU_ERR_OK;
}

int idcu_msgbus_subscribe(idcu_MsgBus* bus,
                           idcu_MsgTopic topic,
                           idcu_MsgHandler handler,
                           void* user_data,
                           idcu_MsgSubscriber** subscriber) {
    if (!bus || !handler || !subscriber) {
        return IDCU_ERR_INVALID_ARG;
    }

    if (topic >= IDCU_MSG_MAX_TOPICS) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_MsgSubscriber* sub = (idcu_MsgSubscriber*)malloc(sizeof(idcu_MsgSubscriber));
    if (!sub) {
        return IDCU_ERR_MEMORY;
    }

    sub->bus = bus;
    sub->topic = topic;
    sub->handler = handler;
    sub->user_data = user_data;
    sub->active = 1;

    idcu_mutex_lock(&bus->mutex);

    if (!bus->topic_subscribers[topic].subscribers) {
        bus->topic_subscribers[topic].subscribers = (idcu_Vector*)malloc(sizeof(idcu_Vector));
        if (!bus->topic_subscribers[topic].subscribers) {
            idcu_mutex_unlock(&bus->mutex);
            free(sub);
            return IDCU_ERR_MEMORY;
        }
        int ret = idcu_vector_init(bus->topic_subscribers[topic].subscribers,
                                    sizeof(idcu_MsgSubscriber*), 8);
        if (ret != IDCU_ERR_OK) {
            idcu_mutex_unlock(&bus->mutex);
            free(bus->topic_subscribers[topic].subscribers);
            free(sub);
            return ret;
        }
    }

    idcu_vector_push_back(bus->topic_subscribers[topic].subscribers, &sub);
    idcu_vector_push_back(bus->all_subscribers, &sub);

    idcu_mutex_unlock(&bus->mutex);

    *subscriber = sub;
    return IDCU_ERR_OK;
}

int idcu_msgbus_unsubscribe(idcu_MsgBus* bus,
                             idcu_MsgSubscriber* subscriber) {
    if (!bus || !subscriber) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_MsgTopic topic = subscriber->topic;
    if (topic >= IDCU_MSG_MAX_TOPICS) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&bus->mutex);

    subscriber->active = 0;

    // Remove from topic subscribers list
    if (bus->topic_subscribers[topic].subscribers) {
        idcu_Vector* subs = bus->topic_subscribers[topic].subscribers;
        for (size_t i = 0; i < idcu_vector_size(subs); i++) {
            idcu_MsgSubscriber** sub_ptr = (idcu_MsgSubscriber**)idcu_vector_get(subs, i);
            if (sub_ptr && *sub_ptr == subscriber) {
                idcu_vector_remove(subs, i);
                break;
            }
        }
    }

    // Remove from all subscribers list
    if (bus->all_subscribers) {
        idcu_Vector* all_subs = bus->all_subscribers;
        for (size_t i = 0; i < idcu_vector_size(all_subs); i++) {
            idcu_MsgSubscriber** sub_ptr = (idcu_MsgSubscriber**)idcu_vector_get(all_subs, i);
            if (sub_ptr && *sub_ptr == subscriber) {
                idcu_vector_remove(all_subs, i);
                break;
            }
        }
    }

    idcu_mutex_unlock(&bus->mutex);

    // Free the subscriber
    free(subscriber);

    return IDCU_ERR_OK;
}

int idcu_msgbus_process(idcu_MsgBus* bus) {
    if (!bus) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&bus->mutex);

    for (int topic = 0; topic < IDCU_MSG_MAX_TOPICS; topic++) {
        if (!bus->topic_queues[topic]) {
            continue;
        }

        while (idcu_vector_size(bus->topic_queues[topic]) > 0) {
            idcu_Msg** msg_ptr = (idcu_Msg**)idcu_vector_get(bus->topic_queues[topic], 0);
            idcu_Msg* msg = NULL;
            if (msg_ptr) {
                msg = *msg_ptr;
            }
            idcu_vector_remove(bus->topic_queues[topic], 0);

            if (!bus->topic_subscribers[topic].subscribers || !msg) {
                if (msg) {
                    if (msg->data) {
                        free(msg->data);
                    }
                    free(msg);
                }
                continue;
            }

            idcu_Vector* subs = bus->topic_subscribers[topic].subscribers;
            size_t sub_count = idcu_vector_size(subs);
            idcu_MsgSubscriber** local_subs = (idcu_MsgSubscriber**)malloc(sizeof(idcu_MsgSubscriber*) * sub_count);
            if (!local_subs) {
                if (msg) {
                    if (msg->data) {
                        free(msg->data);
                    }
                    free(msg);
                }
                continue;
            }

            for (size_t i = 0; i < sub_count; i++) {
                idcu_MsgSubscriber** sub_ptr = (idcu_MsgSubscriber**)idcu_vector_get(subs, i);
                if (sub_ptr) {
                    local_subs[i] = *sub_ptr;
                } else {
                    local_subs[i] = NULL;
                }
            }

            idcu_MsgTopic msg_topic = msg->topic;
            const void* msg_data = msg->data;
            size_t msg_dsize = msg->data_size;

            idcu_mutex_unlock(&bus->mutex);

            for (size_t i = 0; i < sub_count; i++) {
                idcu_MsgSubscriber* sub = local_subs[i];
                if (sub && sub->active && sub->handler) {
                    sub->handler(msg_topic, msg_data, msg_dsize, sub->user_data);
                }
            }

            idcu_mutex_lock(&bus->mutex);

            free(local_subs);

            if (msg) {
                if (msg->data) {
                    free(msg->data);
                }
                free(msg);
            }
        }
    }

    idcu_mutex_unlock(&bus->mutex);

    return IDCU_ERR_OK;
}
