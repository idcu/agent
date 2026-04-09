#ifndef IDCU_STORAGE_STORAGE_H
#define IDCU_STORAGE_STORAGE_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/storage/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_storage_init(idcu_Storage_Context** ctx);
void idcu_storage_destroy(idcu_Storage_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
