#include "idcu/security_enhanced/security_enhanced.h"
#include "idcu/log/log.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "advapi32.lib")
#else
#include <fcntl.h>
#include <unistd.h>
#endif

static int g_initialized = 0;

static const char* g_base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int base64_char_to_index(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

int idcu_security_init(void) {
    if (g_initialized) {
        IDCU_LOG_WARNING("[security-enhanced] Already initialized");
        return IDCU_ERR_OK;
    }

#ifdef _WIN32
    HCRYPTPROV hProv = 0;
    if (!CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        IDCU_LOG_ERROR("[security-enhanced] Failed to acquire crypto context");
        return IDCU_ERR_SECURITY_ERROR;
    }
    CryptReleaseContext(hProv, 0);
#endif

    g_initialized = 1;
    IDCU_LOG_INFO("[security-enhanced] Initialized");
    return IDCU_ERR_OK;
}

void idcu_security_shutdown(void) {
    if (!g_initialized) {
        return;
    }

    g_initialized = 0;
    IDCU_LOG_INFO("[security-enhanced] Shutdown");
}

int idcu_security_random_bytes(uint8_t* buffer, size_t size) {
    if (!g_initialized || !buffer || size == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

#ifdef _WIN32
    HCRYPTPROV hProv = 0;
    if (!CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        return IDCU_ERR_SECURITY_ERROR;
    }
    if (!CryptGenRandom(hProv, (DWORD)size, buffer)) {
        CryptReleaseContext(hProv, 0);
        return IDCU_ERR_SECURITY_ERROR;
    }
    CryptReleaseContext(hProv, 0);
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        return IDCU_ERR_IO_ERROR;
    }
    ssize_t read_bytes = read(fd, buffer, size);
    close(fd);
    if (read_bytes != (ssize_t)size) {
        return IDCU_ERR_IO_ERROR;
    }
#endif

    return IDCU_ERR_OK;
}

int idcu_security_generate_key(uint8_t* key, size_t key_size) {
    return idcu_security_random_bytes(key, key_size);
}

int idcu_security_generate_iv(uint8_t* iv, size_t iv_size) {
    return idcu_security_random_bytes(iv, iv_size);
}

int idcu_security_generate_salt(uint8_t* salt, size_t salt_size) {
    return idcu_security_random_bytes(salt, salt_size);
}

int idcu_security_random_int(uint64_t* result, uint64_t min, uint64_t max) {
    if (!g_initialized || !result || min >= max) {
        return IDCU_ERR_INVALID_PARAM;
    }

    uint64_t range = max - min + 1;
    uint64_t random_val;
    
    int ret = idcu_security_random_bytes((uint8_t*)&random_val, sizeof(random_val));
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    *result = min + (random_val % range);
    return IDCU_ERR_OK;
}

int idcu_security_hash(const void* data, size_t data_size, uint8_t* hash, size_t* hash_size) {
    (void)data;
    (void)data_size;
    (void)hash;
    (void)hash_size;
    IDCU_LOG_WARNING("[security-enhanced] Hash function not fully implemented");
    return IDCU_ERR_OK;
}

int idcu_security_hmac(const void* data, size_t data_size, const uint8_t* key, size_t key_size,
                        uint8_t* hmac, size_t* hmac_size) {
    (void)data;
    (void)data_size;
    (void)key;
    (void)key_size;
    (void)hmac;
    (void)hmac_size;
    IDCU_LOG_WARNING("[security-enhanced] HMAC function not fully implemented");
    return IDCU_ERR_OK;
}

int idcu_security_encrypt(idcu_SecurityAlgorithm alg,
                          const uint8_t* key, size_t key_size,
                          const uint8_t* iv, size_t iv_size,
                          const void* plaintext, size_t plaintext_size,
                          void* ciphertext, size_t* ciphertext_size) {
    (void)alg;
    (void)key;
    (void)key_size;
    (void)iv;
    (void)iv_size;
    (void)plaintext;
    (void)plaintext_size;
    (void)ciphertext;
    (void)ciphertext_size;
    IDCU_LOG_WARNING("[security-enhanced] Encrypt function not fully implemented");
    return IDCU_ERR_OK;
}

int idcu_security_decrypt(idcu_SecurityAlgorithm alg,
                          const uint8_t* key, size_t key_size,
                          const uint8_t* iv, size_t iv_size,
                          const void* ciphertext, size_t ciphertext_size,
                          void* plaintext, size_t* plaintext_size) {
    (void)alg;
    (void)key;
    (void)key_size;
    (void)iv;
    (void)iv_size;
    (void)ciphertext;
    (void)ciphertext_size;
    (void)plaintext;
    (void)plaintext_size;
    IDCU_LOG_WARNING("[security-enhanced] Decrypt function not fully implemented");
    return IDCU_ERR_OK;
}

int idcu_security_hash_password(const char* password, const uint8_t* salt, size_t salt_size,
                                 uint8_t* hash, size_t* hash_size) {
    (void)password;
    (void)salt;
    (void)salt_size;
    (void)hash;
    (void)hash_size;
    IDCU_LOG_WARNING("[security-enhanced] Password hash function not fully implemented");
    return IDCU_ERR_OK;
}

int idcu_security_verify_password(const char* password, const uint8_t* salt, size_t salt_size,
                                    const uint8_t* hash, size_t hash_size) {
    (void)password;
    (void)salt;
    (void)salt_size;
    (void)hash;
    (void)hash_size;
    IDCU_LOG_WARNING("[security-enhanced] Password verify function not fully implemented");
    return IDCU_ERR_OK;
}

int idcu_security_base64_encode(const void* data, size_t data_size, char* output, size_t* output_size) {
    if (!data || !output_size) {
        return IDCU_ERR_INVALID_PARAM;
    }

    size_t required_size = ((data_size + 2) / 3) * 4 + 1;
    if (!output) {
        *output_size = required_size;
        return IDCU_ERR_OK;
    }

    if (*output_size < required_size) {
        *output_size = required_size;
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }

    const uint8_t* src = (const uint8_t*)data;
    char* dst = output;
    size_t i = 0;

    while (i < data_size) {
        uint32_t val = 0;
        int count = 0;

        for (int j = 0; j < 3 && i < data_size; j++, i++) {
            val = (val << 8) | src[i];
            count++;
        }
        val <<= (3 - count) * 8;

        for (int j = 0; j < 4; j++) {
            dst[j] = (j <= count) ? g_base64_chars[(val >> (18 - j * 6)) & 0x3F] : '=';
        }

        val = 0;
        dst += 4;
    }

    *dst = '\0';
    *output_size = dst - output;
    return IDCU_ERR_OK;
}

int idcu_security_base64_decode(const char* input, size_t input_size, void* output, size_t* output_size) {
    if (!input || !output_size) {
        return IDCU_ERR_INVALID_PARAM;
    }

    size_t padding = 0;
    if (input_size >= 2 && input[input_size - 1] == '=') padding++;
    if (input_size >= 2 && input[input_size - 2] == '=') padding++;

    size_t required_size = (input_size * 3) / 4 - padding;
    if (!output) {
        *output_size = required_size;
        return IDCU_ERR_OK;
    }

    if (*output_size < required_size) {
        *output_size = required_size;
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }

    uint8_t* dst = (uint8_t*)output;
    size_t src_idx = 0;
    size_t dst_idx = 0;

    while (src_idx < input_size) {
        uint32_t val = 0;
        int count = 0;

        for (int j = 0; j < 4 && src_idx < input_size; j++, src_idx++) {
            char c = input[src_idx];
            if (c == '=') break;
            int idx = base64_char_to_index(c);
            if (idx < 0) continue;
            val = (val << 6) | idx;
            count++;
        }

        val <<= (4 - count) * 6;

        for (int j = 0; j < 3 && dst_idx < required_size; j++, dst_idx++) {
            dst[dst_idx] = (val >> (16 - j * 8)) & 0xFF;
        }
    }

    *output_size = dst_idx;
    return IDCU_ERR_OK;
}

int idcu_security_hex_encode(const void* data, size_t data_size, char* output, size_t* output_size) {
    if (!data || !output_size) {
        return IDCU_ERR_INVALID_PARAM;
    }

    size_t required_size = data_size * 2 + 1;
    if (!output) {
        *output_size = required_size;
        return IDCU_ERR_OK;
    }

    if (*output_size < required_size) {
        *output_size = required_size;
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }

    const uint8_t* src = (const uint8_t*)data;
    char* dst = output;
    const char* hex_chars = "0123456789abcdef";

    for (size_t i = 0; i < data_size; i++) {
        *dst++ = hex_chars[(src[i] >> 4) & 0xF];
        *dst++ = hex_chars[src[i] & 0xF];
    }

    *dst = '\0';
    *output_size = dst - output;
    return IDCU_ERR_OK;
}

int idcu_security_hex_decode(const char* input, size_t input_size, void* output, size_t* output_size) {
    if (!input || !output_size || input_size % 2 != 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    size_t required_size = input_size / 2;
    if (!output) {
        *output_size = required_size;
        return IDCU_ERR_OK;
    }

    if (*output_size < required_size) {
        *output_size = required_size;
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }

    uint8_t* dst = (uint8_t*)output;
    for (size_t i = 0; i < input_size; i += 2) {
        char c1 = input[i];
        char c2 = input[i + 1];

        uint8_t val = 0;
        if (c1 >= '0' && c1 <= '9') val = (c1 - '0') << 4;
        else if (c1 >= 'a' && c1 <= 'f') val = (c1 - 'a' + 10) << 4;
        else if (c1 >= 'A' && c1 <= 'F') val = (c1 - 'A' + 10) << 4;

        if (c2 >= '0' && c2 <= '9') val |= (c2 - '0');
        else if (c2 >= 'a' && c2 <= 'f') val |= (c2 - 'a' + 10);
        else if (c2 >= 'A' && c2 <= 'F') val |= (c2 - 'A' + 10);

        dst[i / 2] = val;
    }

    *output_size = required_size;
    return IDCU_ERR_OK;
}
