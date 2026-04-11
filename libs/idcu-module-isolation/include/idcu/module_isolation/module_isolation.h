#ifndef IDCU_MODULE_ISOLATION_MODULE_ISOLATION_H
#define IDCU_MODULE_ISOLATION_MODULE_ISOLATION_H

#include "idcu/common/error_code.h"
#include "idcu/module_isolation/types.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_module_isolation_init(idcu_ModuleIsolation_Context** ctx);
int idcu_module_isolation_create(idcu_ModuleIsolation_Context* ctx, const char* module_name, idcu_IsolationLevel level);
int idcu_module_isolation_destroy(idcu_ModuleIsolation_Context* ctx, const char* module_name);
void idcu_module_isolation_cleanup(idcu_ModuleIsolation_Context* ctx);

int idcu_module_isolation_set_memory_limit(idcu_ModuleIsolation_Context* ctx, const char* module_name, uint64_t bytes);
int idcu_module_isolation_set_cpu_limit(idcu_ModuleIsolation_Context* ctx, const char* module_name, uint32_t percent);
int idcu_module_isolation_get_status(idcu_ModuleIsolation_Context* ctx, const char* module_name, int* status);

#ifdef __cplusplus
}
#endif

#endif
