#ifndef IDCU_MODULE_VERIFIER_MODULE_VERIFIER_H
#define IDCU_MODULE_VERIFIER_MODULE_VERIFIER_H

#include "idcu/common/error_code.h"
#include "idcu/module_verifier/types.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_module_verifier_init(idcu_ModuleVerifier_Context** ctx);
void idcu_module_verifier_destroy(idcu_ModuleVerifier_Context* ctx);

int idcu_module_verifier_verify(idcu_ModuleVerifier_Context* ctx, const char* module_path, uint32_t verify_types);
int idcu_module_verifier_get_status(idcu_ModuleVerifier_Context* ctx, const char* module_path, idcu_VerifyStatus* status);
int idcu_module_verifier_get_checksum(idcu_ModuleVerifier_Context* ctx, const char* module_path, char* checksum, size_t checksum_size);

int idcu_module_verifier_verify_checksum(idcu_ModuleVerifier_Context* ctx, const char* module_path, const char* expected_checksum);
int idcu_module_verifier_verify_signature(idcu_ModuleVerifier_Context* ctx, const char* module_path, const char* public_key_path);
int idcu_module_verifier_verify_dependencies(idcu_ModuleVerifier_Context* ctx, const char* module_path);

#ifdef __cplusplus
}
#endif

#endif
