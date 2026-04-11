#ifndef IDCU_STORAGE_MODULE_STORAGE_MODULE_H
#define IDCU_STORAGE_MODULE_STORAGE_MODULE_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/storage/storage.h>
#include <idcu/sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    idcu_Storage* storage;
    int initialized;
    void* user_data;
} idcu_StorageModule;

int idcu_storage_module_init(idcu_StorageModule* sm);
int idcu_storage_module_start(idcu_StorageModule* sm);
int idcu_storage_module_stop(idcu_StorageModule* sm);
void idcu_storage_module_destroy(idcu_StorageModule* sm);

int idcu_storage_module_put(idcu_StorageModule* sm, const char* key, const void* value, size_t size);
int idcu_storage_module_get(idcu_StorageModule* sm, const char* key, void** value, size_t* size);
int idcu_storage_module_delete(idcu_StorageModule* sm, const char* key);

idcu_Storage* idcu_storage_module_get_storage(idcu_StorageModule* sm);

#ifdef __cplusplus
}
#endif

#endif
