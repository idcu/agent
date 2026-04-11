#include <idcu/cache_module/cache_module.h>
#include <string.h>

int idcu_cache_module_init(idcu_CacheModule* cm) {
    if (!cm) {
        return IDCU_ERR_INVALID_ARG;
    }

    memset(cm, 0, sizeof(idcu_CacheModule));
    cm->initialized = 0;

    idcu_CacheConfig config = {
        .max_entries = 10000,
        .default_ttl_ms = 3600000
    };

    int ret = idcu_cache_create(&config, &cm->cache);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    cm->initialized = 1;
    return IDCU_ERR_OK;
}

int idcu_cache_module_start(idcu_CacheModule* cm) {
    if (!cm || !cm->initialized) {
        return IDCU_ERR_INVALID_STATE;
    }
    return IDCU_ERR_OK;
}

int idcu_cache_module_stop(idcu_CacheModule* cm) {
    if (!cm || !cm->initialized) {
        return IDCU_ERR_INVALID_STATE;
    }
    return IDCU_ERR_OK;
}

void idcu_cache_module_destroy(idcu_CacheModule* cm) {
    if (!cm) {
        return;
    }

    if (cm->cache) {
        idcu_cache_destroy(cm->cache);
        cm->cache = NULL;
    }

    cm->initialized = 0;
    memset(cm, 0, sizeof(idcu_CacheModule));
}

int idcu_cache_module_set(idcu_CacheModule* cm, const char* key, const void* value, size_t size, uint64_t ttl_ms) {
    if (!cm || !cm->cache || !key || !value) {
        return IDCU_ERR_INVALID_ARG;
    }
    (void)ttl_ms;
    return IDCU_ERR_OK;
}

int idcu_cache_module_get(idcu_CacheModule* cm, const char* key, void** value, size_t* size) {
    if (!cm || !cm->cache || !key || !value || !size) {
        return IDCU_ERR_INVALID_ARG;
    }
    *value = NULL;
    *size = 0;
    return IDCU_ERR_NOT_FOUND;
}

int idcu_cache_module_delete(idcu_CacheModule* cm, const char* key) {
    if (!cm || !cm->cache || !key) {
        return IDCU_ERR_INVALID_ARG;
    }
    return IDCU_ERR_OK;
}

idcu_Cache* idcu_cache_module_get_cache(idcu_CacheModule* cm) {
    if (!cm) {
        return NULL;
    }
    return cm->cache;
}
