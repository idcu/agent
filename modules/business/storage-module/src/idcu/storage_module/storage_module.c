#include <idcu/storage_module/storage_module.h>
#include <string.h>

int idcu_storage_module_init(idcu_StorageModule* sm) {
    if (!sm) {
        return IDCU_ERR_INVALID_ARG;
    }

    memset(sm, 0, sizeof(idcu_StorageModule));
    sm->initialized = 0;

    idcu_StorageConfig config = {
        .storage_path = "data",
        .max_file_size = 100 * 1024 * 1024
    };

    int ret = idcu_storage_create(&config, &sm->storage);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    sm->initialized = 1;
    return IDCU_ERR_OK;
}

int idcu_storage_module_start(idcu_StorageModule* sm) {
    if (!sm || !sm->initialized) {
        return IDCU_ERR_INVALID_STATE;
    }
    return IDCU_ERR_OK;
}

int idcu_storage_module_stop(idcu_StorageModule* sm) {
    if (!sm || !sm->initialized) {
        return IDCU_ERR_INVALID_STATE;
    }
    return IDCU_ERR_OK;
}

void idcu_storage_module_destroy(idcu_StorageModule* sm) {
    if (!sm) {
        return;
    }

    if (sm->storage) {
        idcu_storage_destroy(sm->storage);
        sm->storage = NULL;
    }

    sm->initialized = 0;
    memset(sm, 0, sizeof(idcu_StorageModule));
}

int idcu_storage_module_put(idcu_StorageModule* sm, const char* key, const void* value, size_t size) {
    if (!sm || !sm->storage || !key || !value) {
        return IDCU_ERR_INVALID_ARG;
    }
    return IDCU_ERR_OK;
}

int idcu_storage_module_get(idcu_StorageModule* sm, const char* key, void** value, size_t* size) {
    if (!sm || !sm->storage || !key || !value || !size) {
        return IDCU_ERR_INVALID_ARG;
    }
    *value = NULL;
    *size = 0;
    return IDCU_ERR_NOT_FOUND;
}

int idcu_storage_module_delete(idcu_StorageModule* sm, const char* key) {
    if (!sm || !sm->storage || !key) {
        return IDCU_ERR_INVALID_ARG;
    }
    return IDCU_ERR_OK;
}

idcu_Storage* idcu_storage_module_get_storage(idcu_StorageModule* sm) {
    if (!sm) {
        return NULL;
    }
    return sm->storage;
}
