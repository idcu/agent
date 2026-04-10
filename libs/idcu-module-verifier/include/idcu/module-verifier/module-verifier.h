#ifndef IDCU_MODULE-VERIFIER_MODULE-VERIFIER_H
#define IDCU_MODULE-VERIFIER_MODULE-VERIFIER_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/module-verifier/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_module-verifier_init(idcu_Module-Verifier_Context** ctx);
void idcu_module-verifier_destroy(idcu_Module-Verifier_Context* ctx);
int idcu_module-verifier_is_initialized(idcu_Module-Verifier_Context* ctx);
uint64_t idcu_module-verifier_get_operation_count(idcu_Module-Verifier_Context* ctx);
uint64_t idcu_module-verifier_get_error_count(idcu_Module-Verifier_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
