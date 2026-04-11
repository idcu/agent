#include <idcu/discovery/discovery.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

static uint64_t get_time_ms(void) {
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER li;
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    return (li.QuadPart - 116444736000000000ULL) / 10000;
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

const char* idcu_node_status_to_string(idcu_NodeStatus status) {
    switch (status) {
        case IDCU_NODE_STATUS_ONLINE: return "online";
        case IDCU_NODE_STATUS_OFFLINE: return "offline";
        case IDCU_NODE_STATUS_UNHEALTHY: return "unhealthy";
        case IDCU_NODE_STATUS_UNKNOWN: return "unknown";
        default: return "unknown";
    }
}

static void node_dtor(void* element) {
    (void)element;
}

int idcu_discovery_init(idcu_Discovery* discovery, uint64_t discovery_interval_ms, uint64_t node_timeout_ms) {
    if (!discovery) {
        return IDCU_ERR_INVALID_ARG;
    }
    memset(discovery, 0, sizeof(idcu_Discovery));
    int ret = idcu_vector_init_with_dtor(&discovery->nodes, sizeof(idcu_Node), 16, node_dtor);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    ret = idcu_hash_map_init(&discovery->nodes_by_id, 64);
    if (ret != IDCU_ERR_OK) {
        idcu_vector_destroy(&discovery->nodes);
        return ret;
    }
    ret = idcu_mutex_init(&discovery->lock);
    if (ret != IDCU_ERR_OK) {
        idcu_hash_map_destroy(&discovery->nodes_by_id);
        idcu_vector_destroy(&discovery->nodes);
        return ret;
    }
    discovery->discovery_interval_ms = discovery_interval_ms;
    discovery->node_timeout_ms = node_timeout_ms;
    discovery->initialized = 1;
    return IDCU_ERR_OK;
}

void idcu_discovery_destroy(idcu_Discovery* discovery) {
    if (!discovery) {
        return;
    }
    idcu_mutex_lock(&discovery->lock);
    idcu_hash_map_destroy(&discovery->nodes_by_id);
    idcu_vector_destroy(&discovery->nodes);
    idcu_Mutex lock_copy = discovery->lock;
    discovery->initialized = 0;
    discovery->running = 0;
    idcu_mutex_unlock(&lock_copy);
    idcu_mutex_destroy(&lock_copy);
}

int idcu_discovery_register_node(idcu_Discovery* discovery, const idcu_Node* node) {
    if (!discovery || !discovery->initialized || !node || !node->id) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&discovery->lock);
    idcu_Node new_node = *node;
    new_node.last_seen_ms = get_time_ms();
    if (new_node.status == IDCU_NODE_STATUS_UNKNOWN) {
        new_node.status = IDCU_NODE_STATUS_ONLINE;
    }
    int ret = idcu_vector_push_back(&discovery->nodes, &new_node);
    if (ret == IDCU_ERR_OK) {
        idcu_Node* stored_node = (idcu_Node*)idcu_vector_back(&discovery->nodes);
        idcu_hash_map_set(&discovery->nodes_by_id, stored_node->id, stored_node);
    }
    idcu_mutex_unlock(&discovery->lock);
    return ret;
}

int idcu_discovery_unregister_node(idcu_Discovery* discovery, const char* node_id) {
    if (!discovery || !discovery->initialized || !node_id) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&discovery->lock);
    size_t count = idcu_vector_size(&discovery->nodes);
    for (size_t i = 0; i < count; i++) {
        idcu_Node* node = (idcu_Node*)idcu_vector_at(&discovery->nodes, i);
        if (strcmp(node->id, node_id) == 0) {
            idcu_hash_map_remove(&discovery->nodes_by_id, node_id);
            idcu_vector_erase(&discovery->nodes, i);
            idcu_mutex_unlock(&discovery->lock);
            return IDCU_ERR_OK;
        }
    }
    idcu_mutex_unlock(&discovery->lock);
    return IDCU_ERR_NOT_FOUND;
}

idcu_Node* idcu_discovery_get_node(idcu_Discovery* discovery, const char* node_id) {
    if (!discovery || !discovery->initialized || !node_id) {
        return NULL;
    }
    idcu_mutex_lock(&discovery->lock);
    idcu_Node* node = (idcu_Node*)idcu_hash_map_get(&discovery->nodes_by_id, node_id);
    idcu_mutex_unlock(&discovery->lock);
    return node;
}

int idcu_discovery_heartbeat(idcu_Discovery* discovery, const char* node_id) {
    if (!discovery || !discovery->initialized || !node_id) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&discovery->lock);
    idcu_Node* node = (idcu_Node*)idcu_hash_map_get(&discovery->nodes_by_id, node_id);
    if (node) {
        node->last_seen_ms = get_time_ms();
        node->status = IDCU_NODE_STATUS_ONLINE;
        idcu_mutex_unlock(&discovery->lock);
        return IDCU_ERR_OK;
    }
    idcu_mutex_unlock(&discovery->lock);
    return IDCU_ERR_NOT_FOUND;
}

int idcu_discovery_update_node_status(idcu_Discovery* discovery, const char* node_id, idcu_NodeStatus status) {
    if (!discovery || !discovery->initialized || !node_id) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&discovery->lock);
    idcu_Node* node = (idcu_Node*)idcu_hash_map_get(&discovery->nodes_by_id, node_id);
    if (node) {
        node->status = status;
        idcu_mutex_unlock(&discovery->lock);
        return IDCU_ERR_OK;
    }
    idcu_mutex_unlock(&discovery->lock);
    return IDCU_ERR_NOT_FOUND;
}

int idcu_discovery_refresh(idcu_Discovery* discovery) {
    if (!discovery || !discovery->initialized) {
        return IDCU_ERR_INVALID_ARG;
    }
    uint64_t now = get_time_ms();
    idcu_mutex_lock(&discovery->lock);
    size_t count = idcu_vector_size(&discovery->nodes);
    for (size_t i = 0; i < count; i++) {
        idcu_Node* node = (idcu_Node*)idcu_vector_at(&discovery->nodes, i);
        uint64_t elapsed = now - node->last_seen_ms;
        if (elapsed > discovery->node_timeout_ms) {
            if (node->status == IDCU_NODE_STATUS_ONLINE) {
                node->status = IDCU_NODE_STATUS_OFFLINE;
            }
        }
    }
    idcu_mutex_unlock(&discovery->lock);
    return IDCU_ERR_OK;
}

int idcu_discovery_get_nodes_by_status(idcu_Discovery* discovery, idcu_NodeStatus status, idcu_Vector* results) {
    if (!discovery || !discovery->initialized || !results) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&discovery->lock);
    size_t count = idcu_vector_size(&discovery->nodes);
    for (size_t i = 0; i < count; i++) {
        idcu_Node* node = (idcu_Node*)idcu_vector_at(&discovery->nodes, i);
        if (node->status == status) {
            idcu_vector_push_back(results, node);
        }
    }
    idcu_mutex_unlock(&discovery->lock);
    return IDCU_ERR_OK;
}

int idcu_discovery_get_all_nodes(idcu_Discovery* discovery, idcu_Vector* results) {
    if (!discovery || !discovery->initialized || !results) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&discovery->lock);
    size_t count = idcu_vector_size(&discovery->nodes);
    for (size_t i = 0; i < count; i++) {
        idcu_Node* node = (idcu_Node*)idcu_vector_at(&discovery->nodes, i);
        idcu_vector_push_back(results, node);
    }
    idcu_mutex_unlock(&discovery->lock);
    return IDCU_ERR_OK;
}

int idcu_discovery_find_nodes_by_tag(idcu_Discovery* discovery, const char* tag, idcu_Vector* results) {
    if (!discovery || !discovery->initialized || !tag || !results) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&discovery->lock);
    size_t count = idcu_vector_size(&discovery->nodes);
    for (size_t i = 0; i < count; i++) {
        idcu_Node* node = (idcu_Node*)idcu_vector_at(&discovery->nodes, i);
        if (strstr(node->tags, tag) != NULL) {
            idcu_vector_push_back(results, node);
        }
    }
    idcu_mutex_unlock(&discovery->lock);
    return IDCU_ERR_OK;
}
