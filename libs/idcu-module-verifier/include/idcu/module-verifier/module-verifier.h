#ifndef IDCU_MODULE-VERIFIER_MODULE-VERIFIER_H
#define IDCU_MODULE-VERIFIER_MODULE-VERIFIER_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/module-verifier/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_module-verifier_init(idcu_Module-Verifier_Context** ctx);
void idcu_module-verifier_destroy(idcu_Module-Verifier_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
