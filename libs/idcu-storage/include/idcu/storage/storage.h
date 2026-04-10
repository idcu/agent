#ifndef IDCU_STORAGE_STORAGE_H
#define IDCU_STORAGE_STORAGE_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/storage/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_storage_init(idcu_Storage_Context** ctx);
void idcu_storage_destroy(idcu_Storage_Context* ctx);
int idcu_storage_is_initialized(idcu_Storage_Context* ctx);
uint64_t idcu_storage_get_operation_count(idcu_Storage_Context* ctx);
uint64_t idcu_storage_get_error_count(idcu_Storage_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
