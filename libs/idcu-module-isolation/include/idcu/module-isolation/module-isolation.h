#ifndef IDCU_MODULE-ISOLATION_MODULE-ISOLATION_H
#define IDCU_MODULE-ISOLATION_MODULE-ISOLATION_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/module-isolation/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_module-isolation_init(idcu_Module-Isolation_Context** ctx);
void idcu_module-isolation_destroy(idcu_Module-Isolation_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
