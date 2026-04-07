#ifndef IDCU_SECURITY_ENHANCED_MODULE_SECURITY_H
#define IDCU_SECURITY_ENHANCED_MODULE_SECURITY_H

#include "idcu/common/error_code.h"
#include <stdint.h>
#include <stddef.h>

#define IDCU_MODULE_SIGNATURE_SIZE 256
#define IDCU_MODULE_PUBLIC_KEY_SIZE 512
#define IDCU_MODULE_HASH_SIZE 32
#define IDCU_MODULE_NAME_MAX 128
#define IDCU_MODULE_VERSION_MAX 32
#define IDCU_CONFIG_KEY_MAX 64
#define IDCU_CONFIG_VALUE_MAX 4096
#define IDCU_CONFIG_MAX_ENTRIES 256

typedef enum {
    IDCU_SIGNATURE_ALG_SHA256_RSA = 0,
    IDCU_SIGNATURE_ALG_SHA256_ECDSA,
    IDCU_SIGNATURE_ALG_ED25519
} idcu_SignatureAlgorithm;

typedef struct {
    uint8_t signature[IDCU_MODULE_SIGNATURE_SIZE];
    size_t signature_size;
    idcu_SignatureAlgorithm alg;
    uint8_t module_hash[IDCU_MODULE_HASH_SIZE];
    char module_name[IDCU_MODULE_NAME_MAX];
    char module_version[IDCU_MODULE_VERSION_MAX];
    uint64_t timestamp;
} idcu_ModuleSignature;

typedef struct {
    uint8_t public_key[IDCU_MODULE_PUBLIC_KEY_SIZE];
    size_t key_size;
    idcu_SignatureAlgorithm alg;
    char key_id[64];
    int is_revoked;
    uint64_t created_at;
    uint64_t expires_at;
} idcu_SigningKey;

typedef struct {
    uint8_t data[IDCU_CONFIG_VALUE_MAX];
    size_t data_size;
    uint8_t iv[16];
    size_t iv_size;
    uint8_t tag[16];
    size_t tag_size;
    int is_encrypted;
} idcu_EncryptedConfig;

typedef struct idcu_ModuleSecurityContext idcu_ModuleSecurityContext;

int idcu_module_security_init(void);
void idcu_module_security_shutdown(void);

int idcu_module_security_context_create(idcu_ModuleSecurityContext** ctx);
void idcu_module_security_context_destroy(idcu_ModuleSecurityContext* ctx);

int idcu_module_security_add_trusted_key(idcu_ModuleSecurityContext* ctx, const idcu_SigningKey* key);
int idcu_module_security_remove_trusted_key(idcu_ModuleSecurityContext* ctx, const char* key_id);
int idcu_module_security_revoke_key(idcu_ModuleSecurityContext* ctx, const char* key_id);
int idcu_module_security_clear_trusted_keys(idcu_ModuleSecurityContext* ctx);

int idcu_module_signature_verify(idcu_ModuleSecurityContext* ctx, 
                                   const void* module_data, size_t module_size,
                                   const idcu_ModuleSignature* signature);

int idcu_module_signature_sign(const void* module_data, size_t module_size,
                                const uint8_t* private_key, size_t key_size,
                                idcu_SignatureAlgorithm alg,
                                idcu_ModuleSignature* signature);

int idcu_module_signature_serialize(const idcu_ModuleSignature* signature, 
                                      char* buffer, size_t buffer_size, size_t* output_size);
int idcu_module_signature_deserialize(const char* buffer, size_t buffer_size,
                                        idcu_ModuleSignature* signature);

int idcu_config_encrypt(const void* plaintext, size_t plaintext_size,
                        const uint8_t* key, size_t key_size,
                        idcu_EncryptedConfig* encrypted);
int idcu_config_decrypt(const idcu_EncryptedConfig* encrypted,
                        const uint8_t* key, size_t key_size,
                        void* plaintext, size_t* plaintext_size);

int idcu_config_encrypt_string(const char* plaintext, const uint8_t* key, size_t key_size,
                                 char* base64_output, size_t output_size);
int idcu_config_decrypt_string(const char* base64_input, const uint8_t* key, size_t key_size,
                                 char* plaintext, size_t plaintext_size);

int idcu_encrypted_config_serialize(const idcu_EncryptedConfig* cfg,
                                       char* buffer, size_t buffer_size, size_t* output_size);
int idcu_encrypted_config_deserialize(const char* buffer, size_t buffer_size,
                                         idcu_EncryptedConfig* cfg);

int idcu_generate_encryption_key(uint8_t* key, size_t key_size);
int idcu_derive_key_from_password(const char* password, const uint8_t* salt, size_t salt_size,
                                     uint8_t* key, size_t key_size);

#endif // IDCU_SECURITY_ENHANCED_MODULE_SECURITY_H
