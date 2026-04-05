#include "module_def.h"
#include "cache.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IDCU_CACHE_MAX_MODULE_CACHES 8

typedef struct {
    idcu_Cache* caches[IDCU_CACHE_MAX_MODULE_CACHES];
    int initialized;
} idcu_CacheModuleData;

static idcu_CacheModuleData* g_cache_module = NULL;

static int cache_module_init() {
    IDCU_LOG_INFO("[cache_module] Initializing cache module");

    if (!g_cache_module) {
        g_cache_module = (idcu_CacheModuleData*)malloc(sizeof(idcu_CacheModuleData));
        if (!g_cache_module) {
            IDCU_LOG_ERROR("[cache_module] Failed to allocate module data");
            return -1;
        }
        memset(g_cache_module, 0, sizeof(idcu_CacheModuleData));
    }

    g_cache_module->initialized = 1;
    IDCU_LOG_INFO("[cache_module] Cache module initialized successfully");
    return 0;
}

static int cache_module_run() {
    return 0;
}

static int cache_module_stop() {
    IDCU_LOG_INFO("[cache_module] Stopping cache module");

    if (g_cache_module) {
        for (int i = 0; i < IDCU_CACHE_MAX_MODULE_CACHES; i++) {
            if (g_cache_module->caches[i]) {
                idcu_cache_destroy(g_cache_module->caches[i]);
                g_cache_module->caches[i] = NULL;
            }
        }
        free(g_cache_module);
        g_cache_module = NULL;
    }

    IDCU_LOG_INFO("[cache_module] Cache module stopped");
    return 0;
}

int idcu_cache_module_create_cache(int* cache_id, size_t max_entries, idcu_CachePolicy policy) {
    if (!g_cache_module || !g_cache_module->initialized) {
        return -1;
    }
    if (!cache_id) {
        return -1;
    }

    int id = -1;
    for (int i = 0; i < IDCU_CACHE_MAX_MODULE_CACHES; i++) {
        if (!g_cache_module->caches[i]) {
            id = i;
            break;
        }
    }

    if (id < 0) {
        IDCU_LOG_ERROR("[cache_module] No available cache slots");
        return -1;
    }

    int ret = idcu_cache_init(&g_cache_module->caches[id], max_entries, policy);
    if (ret != IDCU_ERR_OK) {
        return -1;
    }

    *cache_id = id;
    IDCU_LOG_INFO("[cache_module] Created cache %d with max entries %zu", id, max_entries);
    return 0;
}

int idcu_cache_module_destroy_cache(int cache_id) {
    if (!g_cache_module || !g_cache_module->initialized) {
        return -1;
    }
    if (cache_id < 0 || cache_id >= IDCU_CACHE_MAX_MODULE_CACHES || !g_cache_module->caches[cache_id]) {
        return -1;
    }

    idcu_cache_destroy(g_cache_module->caches[cache_id]);
    g_cache_module->caches[cache_id] = NULL;
    IDCU_LOG_INFO("[cache_module] Destroyed cache %d", cache_id);
    return 0;
}

int idcu_cache_module_put(int cache_id, const char* key, const void* value, size_t size, uint64_t ttl_ms) {
    if (!g_cache_module || !g_cache_module->initialized) {
        return -1;
    }
    if (cache_id < 0 || cache_id >= IDCU_CACHE_MAX_MODULE_CACHES || !g_cache_module->caches[cache_id]) {
        return -1;
    }
    return idcu_cache_put(g_cache_module->caches[cache_id], key, value, size, ttl_ms) == IDCU_ERR_OK ? 0 : -1;
}

int idcu_cache_module_get(int cache_id, const char* key, void* buffer, size_t* buffer_size) {
    if (!g_cache_module || !g_cache_module->initialized) {
        return -1;
    }
    if (cache_id < 0 || cache_id >= IDCU_CACHE_MAX_MODULE_CACHES || !g_cache_module->caches[cache_id]) {
        return -1;
    }
    return idcu_cache_get(g_cache_module->caches[cache_id], key, buffer, buffer_size) == IDCU_ERR_OK ? 0 : -1;
}

int idcu_cache_module_remove(int cache_id, const char* key) {
    if (!g_cache_module || !g_cache_module->initialized) {
        return -1;
    }
    if (cache_id < 0 || cache_id >= IDCU_CACHE_MAX_MODULE_CACHES || !g_cache_module->caches[cache_id]) {
        return -1;
    }
    return idcu_cache_remove(g_cache_module->caches[cache_id], key) == IDCU_ERR_OK ? 0 : -1;
}

int idcu_cache_module_clear(int cache_id) {
    if (!g_cache_module || !g_cache_module->initialized) {
        return -1;
    }
    if (cache_id < 0 || cache_id >= IDCU_CACHE_MAX_MODULE_CACHES || !g_cache_module->caches[cache_id]) {
        return -1;
    }
    return idcu_cache_clear(g_cache_module->caches[cache_id]) == IDCU_ERR_OK ? 0 : -1;
}

int idcu_cache_module_put_string(int cache_id, const char* key, const char* value, uint64_t ttl_ms) {
    if (!g_cache_module || !g_cache_module->initialized) {
        return -1;
    }
    if (cache_id < 0 || cache_id >= IDCU_CACHE_MAX_MODULE_CACHES || !g_cache_module->caches[cache_id]) {
        return -1;
    }
    return idcu_cache_put_string(g_cache_module->caches[cache_id], key, value, ttl_ms) == IDCU_ERR_OK ? 0 : -1;
}

int idcu_cache_module_get_string(int cache_id, const char* key, char* buffer, size_t buffer_size) {
    if (!g_cache_module || !g_cache_module->initialized) {
        return -1;
    }
    if (cache_id < 0 || cache_id >= IDCU_CACHE_MAX_MODULE_CACHES || !g_cache_module->caches[cache_id]) {
        return -1;
    }
    return idcu_cache_get_string(g_cache_module->caches[cache_id], key, buffer, buffer_size) == IDCU_ERR_OK ? 0 : -1;
}

int idcu_cache_module_put_int(int cache_id, const char* key, int64_t value, uint64_t ttl_ms) {
    if (!g_cache_module || !g_cache_module->initialized) {
        return -1;
    }
    if (cache_id < 0 || cache_id >= IDCU_CACHE_MAX_MODULE_CACHES || !g_cache_module->caches[cache_id]) {
        return -1;
    }
    return idcu_cache_put_int(g_cache_module->caches[cache_id], key, value, ttl_ms) == IDCU_ERR_OK ? 0 : -1;
}

int idcu_cache_module_get_int(int cache_id, const char* key, int64_t* value) {
    if (!g_cache_module || !g_cache_module->initialized) {
        return -1;
    }
    if (cache_id < 0 || cache_id >= IDCU_CACHE_MAX_MODULE_CACHES || !g_cache_module->caches[cache_id]) {
        return -1;
    }
    return idcu_cache_get_int(g_cache_module->caches[cache_id], key, value) == IDCU_ERR_OK ? 0 : -1;
}

IDCU_REGISTER_MODULE(cache_module, IDCU_MODULE_VERSION(1, 0, 0),
                     cache_module_init, cache_module_run, cache_module_stop);
