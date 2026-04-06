#ifndef IDCU_SECURITY_ENHANCED_SECURITY_ENHANCED_H
#define IDCU_SECURITY_ENHANCED_SECURITY_ENHANCED_H

#include "idcu/common/error_code.h"
#include <stdint.h>
#include <stddef.h>

#define IDCU_SECURITY_KEY_MAX 64
#define IDCU_SECURITY_IV_MAX 16
#define IDCU_SECURITY_HASH_MAX 64
#define IDCU_SECURITY_SALT_MAX 32
#define IDCU_SECURITY_PASSWORD_MAX 256

typedef enum {
    IDCU_SECURITY_ALG_AES_256_CBC = 0,
    IDCU_SECURITY_ALG_CHACHA20_POLY1305
} idcu_SecurityAlgorithm;

typedef struct idcu_SecurityContext idcu_SecurityContext;

int idcu_security_init(void);
void idcu_security_shutdown(void);

int idcu_security_generate_key(uint8_t* key, size_t key_size);
int idcu_security_generate_iv(uint8_t* iv, size_t iv_size);
int idcu_security_generate_salt(uint8_t* salt, size_t salt_size);

int idcu_security_hash(const void* data, size_t data_size, uint8_t* hash, size_t* hash_size);
int idcu_security_hmac(const void* data, size_t data_size, const uint8_t* key, size_t key_size,
                        uint8_t* hmac, size_t* hmac_size);

int idcu_security_encrypt(idcu_SecurityAlgorithm alg,
                          const uint8_t* key, size_t key_size,
                          const uint8_t* iv, size_t iv_size,
                          const void* plaintext, size_t plaintext_size,
                          void* ciphertext, size_t* ciphertext_size);

int idcu_security_decrypt(idcu_SecurityAlgorithm alg,
                          const uint8_t* key, size_t key_size,
                          const uint8_t* iv, size_t iv_size,
                          const void* ciphertext, size_t ciphertext_size,
                          void* plaintext, size_t* plaintext_size);

int idcu_security_hash_password(const char* password, const uint8_t* salt, size_t salt_size,
                                 uint8_t* hash, size_t* hash_size);
int idcu_security_verify_password(const char* password, const uint8_t* salt, size_t salt_size,
                                    const uint8_t* hash, size_t hash_size);

int idcu_security_random_bytes(uint8_t* buffer, size_t size);
int idcu_security_random_int(uint64_t* result, uint64_t min, uint64_t max);

int idcu_security_base64_encode(const void* data, size_t data_size, char* output, size_t* output_size);
int idcu_security_base64_decode(const char* input, size_t input_size, void* output, size_t* output_size);

int idcu_security_hex_encode(const void* data, size_t data_size, char* output, size_t* output_size);
int idcu_security_hex_decode(const char* input, size_t input_size, void* output, size_t* output_size);

#endif // IDCU_SECURITY_ENHANCED_SECURITY_ENHANCED_H
