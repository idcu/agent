#ifndef IDCU_MODULE_VERIFIER_MODULE_VERIFIER_H
#define IDCU_MODULE_VERIFIER_MODULE_VERIFIER_H

#include "idcu/common/error_code.h"
#include <stdint.h>
#include <stddef.h>

#define IDCU_VERIFIER_HASH_SIZE 32
#define IDCU_VERIFIER_SIGNATURE_SIZE 256
#define IDCU_VERIFIER_MAX_KEY_SIZE 4096

typedef enum {
    IDCU_VERIFY_TYPE_HASH = 0,
    IDCU_VERIFY_TYPE_SIGNATURE,
    IDCU_VERIFY_TYPE_BOTH
} idcu_VerifyType;

typedef struct {
    uint8_t hash[IDCU_VERIFIER_HASH_SIZE];
    uint8_t signature[IDCU_VERIFIER_SIGNATURE_SIZE];
    size_t signature_len;
    char public_key_path[512];
    idcu_VerifyType verify_type;
} idcu_ModuleVerifier;

int idcu_verifier_init(idcu_ModuleVerifier* verifier);
void idcu_verifier_destroy(idcu_ModuleVerifier* verifier);

int idcu_verifier_set_public_key(idcu_ModuleVerifier* verifier, const char* key_path);
int idcu_verifier_set_verify_type(idcu_ModuleVerifier* verifier, idcu_VerifyType type);

int idcu_verifier_compute_hash(const char* file_path, uint8_t* hash_out, size_t hash_size);
int idcu_verifier_verify_hash(const char* file_path, const uint8_t* expected_hash, size_t hash_size);

int idcu_verifier_verify_signature(const char* file_path, const uint8_t* signature, 
                                    size_t signature_len, const char* public_key_path);

int idcu_verifier_verify_module(idcu_ModuleVerifier* verifier, const char* file_path);
int idcu_verifier_verify_module_with_hash(idcu_ModuleVerifier* verifier, const char* file_path, 
                                           const uint8_t* expected_hash);

int idcu_verifier_load_expected_hash(const char* hash_file_path, uint8_t* hash_out, size_t hash_size);
int idcu_verifier_save_hash(const char* file_path, const uint8_t* hash, size_t hash_size);

#endif // IDCU_MODULE_VERIFIER_MODULE_VERIFIER_H
