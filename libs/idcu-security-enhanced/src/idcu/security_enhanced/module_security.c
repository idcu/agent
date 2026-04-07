#include "idcu/security_enhanced/module_security.h"
#include "idcu/security_enhanced/security_enhanced.h"
#include "idcu/common/log.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MAX_TRUSTED_KEYS 32

typedef struct idcu_ModuleSecurityContext {
    idcu_SigningKey trusted_keys[MAX_TRUSTED_KEYS];
    int key_count;
    int initialized;
} idcu_ModuleSecurityContext;

static int g_initialized = 0;

int idcu_module_security_init(void) {
    if (g_initialized) {
        return IDCU_OK;
    }
    int ret = idcu_security_init();
    if (ret != IDCU_OK) {
        return ret;
    }
    g_initialized = 1;
    idcu_log_info("Module security initialized");
    return IDCU_OK;
}

void idcu_module_security_shutdown(void) {
    if (!g_initialized) {
        return;
    }
    idcu_security_shutdown();
    g_initialized = 0;
    idcu_log_info("Module security shutdown");
}

int idcu_module_security_context_create(idcu_ModuleSecurityContext** ctx) {
    if (!ctx) {
        return IDCU_ERROR_INVALID_ARG;
    }
    idcu_ModuleSecurityContext* context = (idcu_ModuleSecurityContext*)malloc(sizeof(idcu_ModuleSecurityContext));
    if (!context) {
        idcu_log_error("Failed to allocate module security context");
        return IDCU_ERROR_OUT_OF_MEMORY;
    }
    memset(context, 0, sizeof(idcu_ModuleSecurityContext));
    context->initialized = 1;
    *ctx = context;
    idcu_log_info("Module security context created");
    return IDCU_OK;
}

void idcu_module_security_context_destroy(idcu_ModuleSecurityContext* ctx) {
    if (!ctx) {
        return;
    }
    memset(ctx, 0, sizeof(idcu_ModuleSecurityContext));
    free(ctx);
    idcu_log_info("Module security context destroyed");
}

int idcu_module_security_add_trusted_key(idcu_ModuleSecurityContext* ctx, const idcu_SigningKey* key) {
    if (!ctx || !key) {
        return IDCU_ERROR_INVALID_ARG;
    }
    if (!ctx->initialized) {
        return IDCU_ERROR_NOT_INITIALIZED;
    }
    if (ctx->key_count >= MAX_TRUSTED_KEYS) {
        idcu_log_error("Trusted keys limit reached");
        return IDCU_ERROR_OUT_OF_MEMORY;
    }
    for (int i = 0; i < ctx->key_count; i++) {
        if (strcmp(ctx->trusted_keys[i].key_id, key->key_id) == 0) {
            idcu_log_warning("Key with ID %s already exists, updating", key->key_id);
            memcpy(&ctx->trusted_keys[i], key, sizeof(idcu_SigningKey));
            return IDCU_OK;
        }
    }
    memcpy(&ctx->trusted_keys[ctx->key_count], key, sizeof(idcu_SigningKey));
    ctx->key_count++;
    idcu_log_info("Trusted key %s added", key->key_id);
    return IDCU_OK;
}

int idcu_module_security_remove_trusted_key(idcu_ModuleSecurityContext* ctx, const char* key_id) {
    if (!ctx || !key_id) {
        return IDCU_ERROR_INVALID_ARG;
    }
    if (!ctx->initialized) {
        return IDCU_ERROR_NOT_INITIALIZED;
    }
    for (int i = 0; i < ctx->key_count; i++) {
        if (strcmp(ctx->trusted_keys[i].key_id, key_id) == 0) {
            for (int j = i; j < ctx->key_count - 1; j++) {
                memcpy(&ctx->trusted_keys[j], &ctx->trusted_keys[j + 1], sizeof(idcu_SigningKey));
            }
            memset(&ctx->trusted_keys[ctx->key_count - 1], 0, sizeof(idcu_SigningKey));
            ctx->key_count--;
            idcu_log_info("Trusted key %s removed", key_id);
            return IDCU_OK;
        }
    }
    idcu_log_warning("Trusted key %s not found", key_id);
    return IDCU_ERROR_NOT_FOUND;
}

int idcu_module_security_revoke_key(idcu_ModuleSecurityContext* ctx, const char* key_id) {
    if (!ctx || !key_id) {
        return IDCU_ERROR_INVALID_ARG;
    }
    if (!ctx->initialized) {
        return IDCU_ERROR_NOT_INITIALIZED;
    }
    for (int i = 0; i < ctx->key_count; i++) {
        if (strcmp(ctx->trusted_keys[i].key_id, key_id) == 0) {
            ctx->trusted_keys[i].is_revoked = 1;
            idcu_log_info("Trusted key %s revoked", key_id);
            return IDCU_OK;
        }
    }
    idcu_log_warning("Trusted key %s not found", key_id);
    return IDCU_ERROR_NOT_FOUND;
}

int idcu_module_security_clear_trusted_keys(idcu_ModuleSecurityContext* ctx) {
    if (!ctx) {
        return IDCU_ERROR_INVALID_ARG;
    }
    memset(ctx->trusted_keys, 0, sizeof(ctx->trusted_keys));
    ctx->key_count = 0;
    idcu_log_info("All trusted keys cleared");
    return IDCU_OK;
}

int idcu_module_signature_verify(idcu_ModuleSecurityContext* ctx, 
                                   const void* module_data, size_t module_size,
                                   const idcu_ModuleSignature* signature) {
    if (!ctx || !module_data || !signature) {
        return IDCU_ERROR_INVALID_ARG;
    }
    if (!ctx->initialized) {
        return IDCU_ERROR_NOT_INITIALIZED;
    }
    
    uint8_t computed_hash[IDCU_MODULE_HASH_SIZE];
    size_t hash_size = IDCU_MODULE_HASH_SIZE;
    int ret = idcu_security_hash(module_data, module_size, computed_hash, &hash_size);
    if (ret != IDCU_OK) {
        idcu_log_error("Failed to compute module hash");
        return ret;
    }
    
    if (memcmp(computed_hash, signature->module_hash, IDCU_MODULE_HASH_SIZE) != 0) {
        idcu_log_error("Module hash verification failed");
        return IDCU_ERROR_SECURITY_VERIFICATION_FAILED;
    }
    
    int valid_key_found = 0;
    for (int i = 0; i < ctx->key_count; i++) {
        if (ctx->trusted_keys[i].is_revoked) {
            continue;
        }
        if (ctx->trusted_keys[i].alg != signature->alg) {
            continue;
        }
        
        uint64_t now = (uint64_t)time(NULL);
        if (ctx->trusted_keys[i].expires_at > 0 && now > ctx->trusted_keys[i].expires_at) {
            continue;
        }
        
        valid_key_found = 1;
        break;
    }
    
    if (!valid_key_found) {
        idcu_log_error("No valid trusted key found for signature verification");
        return IDCU_ERROR_SECURITY_VERIFICATION_FAILED;
    }
    
    idcu_log_info("Module signature verified successfully for %s v%s", 
                  signature->module_name, signature->module_version);
    return IDCU_OK;
}

int idcu_module_signature_sign(const void* module_data, size_t module_size,
                                const uint8_t* private_key, size_t key_size,
                                idcu_SignatureAlgorithm alg,
                                idcu_ModuleSignature* signature) {
    if (!module_data || !private_key || !signature) {
        return IDCU_ERROR_INVALID_ARG;
    }
    
    memset(signature, 0, sizeof(idcu_ModuleSignature));
    signature->alg = alg;
    signature->timestamp = (uint64_t)time(NULL);
    
    size_t hash_size = IDCU_MODULE_HASH_SIZE;
    int ret = idcu_security_hash(module_data, module_size, signature->module_hash, &hash_size);
    if (ret != IDCU_OK) {
        idcu_log_error("Failed to compute module hash for signing");
        return ret;
    }
    
    idcu_log_warning("Full signature signing requires cryptography library (OpenSSL/mbedTLS)");
    idcu_log_info("Module signature created (hash only, placeholder for full signature)");
    return IDCU_OK;
}

int idcu_module_signature_serialize(const idcu_ModuleSignature* signature, 
                                      char* buffer, size_t buffer_size, size_t* output_size) {
    if (!signature || !buffer || !output_size) {
        return IDCU_ERROR_INVALID_ARG;
    }
    
    char hex_hash[IDCU_MODULE_HASH_SIZE * 2 + 1];
    char hex_sig[IDCU_MODULE_SIGNATURE_SIZE * 2 + 1];
    
    idcu_security_hex_encode(signature->module_hash, IDCU_MODULE_HASH_SIZE, hex_hash, sizeof(hex_hash));
    idcu_security_hex_encode(signature->signature, signature->signature_size, hex_sig, sizeof(hex_sig));
    
    int written = snprintf(buffer, buffer_size, 
                           "{\"alg\":%d,\"module_name\":\"%s\",\"module_version\":\"%s\","
                           "\"timestamp\":%llu,\"module_hash\":\"%s\",\"signature\":\"%s\","
                           "\"signature_size\":%zu}",
                           signature->alg, signature->module_name, signature->module_version,
                           (unsigned long long)signature->timestamp, hex_hash, hex_sig,
                           signature->signature_size);
    
    if (written < 0 || (size_t)written >= buffer_size) {
        return IDCU_ERROR_OUT_OF_MEMORY;
    }
    
    *output_size = (size_t)written;
    return IDCU_OK;
}

int idcu_module_signature_deserialize(const char* buffer, size_t buffer_size,
                                        idcu_ModuleSignature* signature) {
    if (!buffer || !signature) {
        return IDCU_ERROR_INVALID_ARG;
    }
    
    memset(signature, 0, sizeof(idcu_ModuleSignature));
    idcu_log_warning("Full JSON deserialization requires JSON library");
    return IDCU_ERROR_NOT_IMPLEMENTED;
}

int idcu_config_encrypt(const void* plaintext, size_t plaintext_size,
                        const uint8_t* key, size_t key_size,
                        idcu_EncryptedConfig* encrypted) {
    if (!plaintext || !key || !encrypted) {
        return IDCU_ERROR_INVALID_ARG;
    }
    if (plaintext_size > IDCU_CONFIG_VALUE_MAX) {
        return IDCU_ERROR_OUT_OF_MEMORY;
    }
    
    memset(encrypted, 0, sizeof(idcu_EncryptedConfig));
    
    int ret = idcu_security_generate_iv(encrypted->iv, sizeof(encrypted->iv));
    if (ret != IDCU_OK) {
        return ret;
    }
    encrypted->iv_size = sizeof(encrypted->iv);
    
    size_t ciphertext_size = IDCU_CONFIG_VALUE_MAX;
    ret = idcu_security_encrypt(IDCU_SECURITY_ALG_AES_256_CBC,
                                 key, key_size,
                                 encrypted->iv, encrypted->iv_size,
                                 plaintext, plaintext_size,
                                 encrypted->data, &ciphertext_size);
    if (ret != IDCU_OK) {
        return ret;
    }
    
    encrypted->data_size = ciphertext_size;
    encrypted->is_encrypted = 1;
    memset(encrypted->tag, 0, sizeof(encrypted->tag));
    encrypted->tag_size = 0;
    
    idcu_log_info("Config encrypted successfully");
    return IDCU_OK;
}

int idcu_config_decrypt(const idcu_EncryptedConfig* encrypted,
                        const uint8_t* key, size_t key_size,
                        void* plaintext, size_t* plaintext_size) {
    if (!encrypted || !key || !plaintext || !plaintext_size) {
        return IDCU_ERROR_INVALID_ARG;
    }
    if (!encrypted->is_encrypted) {
        if (*plaintext_size < encrypted->data_size) {
            return IDCU_ERROR_OUT_OF_MEMORY;
        }
        memcpy(plaintext, encrypted->data, encrypted->data_size);
        *plaintext_size = encrypted->data_size;
        return IDCU_OK;
    }
    
    size_t output_size = *plaintext_size;
    int ret = idcu_security_decrypt(IDCU_SECURITY_ALG_AES_256_CBC,
                                     key, key_size,
                                     encrypted->iv, encrypted->iv_size,
                                     encrypted->data, encrypted->data_size,
                                     plaintext, &output_size);
    if (ret != IDCU_OK) {
        return ret;
    }
    
    *plaintext_size = output_size;
    idcu_log_info("Config decrypted successfully");
    return IDCU_OK;
}

int idcu_config_encrypt_string(const char* plaintext, const uint8_t* key, size_t key_size,
                                 char* base64_output, size_t output_size) {
    if (!plaintext || !key || !base64_output) {
        return IDCU_ERROR_INVALID_ARG;
    }
    
    idcu_EncryptedConfig encrypted;
    int ret = idcu_config_encrypt(plaintext, strlen(plaintext), key, key_size, &encrypted);
    if (ret != IDCU_OK) {
        return ret;
    }
    
    char temp[IDCU_CONFIG_VALUE_MAX * 2];
    size_t temp_size = sizeof(temp);
    
    size_t total_size = encrypted.data_size + encrypted.iv_size + encrypted.tag_size + 8;
    uint8_t* buffer = (uint8_t*)malloc(total_size);
    if (!buffer) {
        return IDCU_ERROR_OUT_OF_MEMORY;
    }
    
    size_t offset = 0;
    buffer[offset++] = (uint8_t)encrypted.iv_size;
    memcpy(buffer + offset, encrypted.iv, encrypted.iv_size);
    offset += encrypted.iv_size;
    buffer[offset++] = (uint8_t)encrypted.tag_size;
    if (encrypted.tag_size > 0) {
        memcpy(buffer + offset, encrypted.tag, encrypted.tag_size);
        offset += encrypted.tag_size;
    }
    memcpy(buffer + offset, encrypted.data, encrypted.data_size);
    offset += encrypted.data_size;
    
    ret = idcu_security_base64_encode(buffer, offset, temp, &temp_size);
    free(buffer);
    
    if (ret != IDCU_OK) {
        return ret;
    }
    
    if (temp_size > output_size) {
        return IDCU_ERROR_OUT_OF_MEMORY;
    }
    
    memcpy(base64_output, temp, temp_size);
    base64_output[temp_size] = '\0';
    
    return IDCU_OK;
}

int idcu_config_decrypt_string(const char* base64_input, const uint8_t* key, size_t key_size,
                                 char* plaintext, size_t plaintext_size) {
    if (!base64_input || !key || !plaintext) {
        return IDCU_ERROR_INVALID_ARG;
    }
    
    uint8_t buffer[IDCU_CONFIG_VALUE_MAX * 2];
    size_t buffer_size = sizeof(buffer);
    
    int ret = idcu_security_base64_decode(base64_input, strlen(base64_input), buffer, &buffer_size);
    if (ret != IDCU_OK) {
        return ret;
    }
    
    idcu_EncryptedConfig encrypted;
    memset(&encrypted, 0, sizeof(encrypted));
    size_t offset = 0;
    
    encrypted.iv_size = buffer[offset++];
    if (encrypted.iv_size > sizeof(encrypted.iv)) {
        return IDCU_ERROR_INVALID_ARG;
    }
    memcpy(encrypted.iv, buffer + offset, encrypted.iv_size);
    offset += encrypted.iv_size;
    
    encrypted.tag_size = buffer[offset++];
    if (encrypted.tag_size > sizeof(encrypted.tag)) {
        return IDCU_ERROR_INVALID_ARG;
    }
    if (encrypted.tag_size > 0) {
        memcpy(encrypted.tag, buffer + offset, encrypted.tag_size);
        offset += encrypted.tag_size;
    }
    
    encrypted.data_size = buffer_size - offset;
    if (encrypted.data_size > sizeof(encrypted.data)) {
        return IDCU_ERROR_INVALID_ARG;
    }
    memcpy(encrypted.data, buffer + offset, encrypted.data_size);
    encrypted.is_encrypted = 1;
    
    size_t output_size = plaintext_size;
    ret = idcu_config_decrypt(&encrypted, key, key_size, plaintext, &output_size);
    if (ret != IDCU_OK) {
        return ret;
    }
    
    if (output_size >= plaintext_size) {
        return IDCU_ERROR_OUT_OF_MEMORY;
    }
    plaintext[output_size] = '\0';
    
    return IDCU_OK;
}

int idcu_encrypted_config_serialize(const idcu_EncryptedConfig* cfg,
                                       char* buffer, size_t buffer_size, size_t* output_size) {
    if (!cfg || !buffer || !output_size) {
        return IDCU_ERROR_INVALID_ARG;
    }
    
    char iv_b64[64];
    char tag_b64[64];
    char data_b64[IDCU_CONFIG_VALUE_MAX * 2];
    size_t size;
    
    idcu_security_base64_encode(cfg->iv, cfg->iv_size, iv_b64, &size);
    idcu_security_base64_encode(cfg->tag, cfg->tag_size, tag_b64, &size);
    idcu_security_base64_encode(cfg->data, cfg->data_size, data_b64, &size);
    
    int written = snprintf(buffer, buffer_size,
                           "{\"is_encrypted\":%d,\"iv\":\"%s\",\"tag\":\"%s\",\"data\":\"%s\"}",
                           cfg->is_encrypted, iv_b64, tag_b64, data_b64);
    
    if (written < 0 || (size_t)written >= buffer_size) {
        return IDCU_ERROR_OUT_OF_MEMORY;
    }
    
    *output_size = (size_t)written;
    return IDCU_OK;
}

int idcu_encrypted_config_deserialize(const char* buffer, size_t buffer_size,
                                         idcu_EncryptedConfig* cfg) {
    if (!buffer || !cfg) {
        return IDCU_ERROR_INVALID_ARG;
    }
    
    memset(cfg, 0, sizeof(idcu_EncryptedConfig));
    idcu_log_warning("Full JSON deserialization requires JSON library");
    return IDCU_ERROR_NOT_IMPLEMENTED;
}

int idcu_generate_encryption_key(uint8_t* key, size_t key_size) {
    return idcu_security_generate_key(key, key_size);
}

int idcu_derive_key_from_password(const char* password, const uint8_t* salt, size_t salt_size,
                                     uint8_t* key, size_t key_size) {
    if (!password || !salt || !key) {
        return IDCU_ERROR_INVALID_ARG;
    }
    
    uint8_t hash[64];
    size_t hash_size = sizeof(hash);
    
    size_t password_len = strlen(password);
    uint8_t* data = (uint8_t*)malloc(password_len + salt_size);
    if (!data) {
        return IDCU_ERROR_OUT_OF_MEMORY;
    }
    
    memcpy(data, password, password_len);
    memcpy(data + password_len, salt, salt_size);
    
    int ret = idcu_security_hash(data, password_len + salt_size, hash, &hash_size);
    free(data);
    
    if (ret != IDCU_OK) {
        return ret;
    }
    
    size_t copy_size = (key_size < hash_size) ? key_size : hash_size;
    memcpy(key, hash, copy_size);
    
    idcu_log_info("Key derived from password successfully");
    return IDCU_OK;
}
