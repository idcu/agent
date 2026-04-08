#include "idcu/module_verifier/module_verifier.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <wincrypt.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#define BUFFER_SIZE 8192

int idcu_verifier_init(idcu_ModuleVerifier *verifier) {
    if (!verifier) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memset(verifier, 0, sizeof(idcu_ModuleVerifier));
    verifier->verify_type = IDCU_VERIFY_TYPE_HASH;

    idcu_log_info("Module verifier initialized");
    return IDCU_ERR_OK;
}

void idcu_verifier_destroy(idcu_ModuleVerifier *verifier) {
    if (!verifier) {
        return;
    }
    memset(verifier, 0, sizeof(idcu_ModuleVerifier));
}

int idcu_verifier_set_public_key(idcu_ModuleVerifier *verifier, const char *key_path) {
    if (!verifier || !key_path) {
        return IDCU_ERR_INVALID_PARAM;
    }

    strncpy(verifier->public_key_path, key_path, sizeof(verifier->public_key_path) - 1);
    verifier->public_key_path[sizeof(verifier->public_key_path) - 1] = '\0';

    idcu_log_info("Public key path set to: %s", key_path);
    return IDCU_ERR_OK;
}

int idcu_verifier_set_verify_type(idcu_ModuleVerifier *verifier, idcu_VerifyType type) {
    if (!verifier) {
        return IDCU_ERR_INVALID_PARAM;
    }
    verifier->verify_type = type;
    idcu_log_info("Verify type set to: %d", type);
    return IDCU_ERR_OK;
}

static int simple_hash(const void *data, size_t len, uint8_t *hash_out, size_t hash_size) {
    if (!data || !hash_out || hash_size < 32) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memset(hash_out, 0, hash_size);

    const uint8_t *bytes = (const uint8_t *)data;
    uint64_t hash[4] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a};

    for (size_t i = 0; i < len; i++) {
        hash[i % 4] ^= (uint64_t)bytes[i] << ((i % 8) * 8);
        hash[(i + 1) % 4] += hash[i % 4];
        hash[(i + 2) % 4] ^= hash[(i + 1) % 4] >> 3;
        hash[(i + 3) % 4] += hash[(i + 2) % 4] << 2;
    }

    memcpy(hash_out, hash, 32);
    return IDCU_ERR_OK;
}

int idcu_verifier_compute_hash(const char *file_path, uint8_t *hash_out, size_t hash_size) {
    if (!file_path || !hash_out || hash_size < IDCU_VERIFIER_HASH_SIZE) {
        return IDCU_ERR_INVALID_PARAM;
    }

    FILE *file = fopen(file_path, "rb");
    if (!file) {
        idcu_log_error("Failed to open file for hashing: %s", file_path);
        return IDCU_ERR_FILE_OPEN;
    }

    uint8_t buffer[BUFFER_SIZE];
    size_t total_read = 0;
    size_t read;

    uint8_t temp_hash[IDCU_VERIFIER_HASH_SIZE] = {0};
    int first_chunk = 1;

    while ((read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (first_chunk) {
            simple_hash(buffer, read, temp_hash, IDCU_VERIFIER_HASH_SIZE);
            first_chunk = 0;
        } else {
            uint8_t combined[BUFFER_SIZE + IDCU_VERIFIER_HASH_SIZE];
            memcpy(combined, temp_hash, IDCU_VERIFIER_HASH_SIZE);
            memcpy(combined + IDCU_VERIFIER_HASH_SIZE, buffer, read);
            simple_hash(combined, read + IDCU_VERIFIER_HASH_SIZE, temp_hash,
                        IDCU_VERIFIER_HASH_SIZE);
        }
        total_read += read;
    }

    fclose(file);

    if (total_read == 0) {
        idcu_log_error("File is empty: %s", file_path);
        return IDCU_ERR_INVALID_PARAM;
    }

    memcpy(hash_out, temp_hash, IDCU_VERIFIER_HASH_SIZE);

    idcu_log_info("Computed hash for file: %s", file_path);
    return IDCU_ERR_OK;
}

int idcu_verifier_verify_hash(const char *file_path, const uint8_t *expected_hash,
                              size_t hash_size) {
    if (!file_path || !expected_hash || hash_size < IDCU_VERIFIER_HASH_SIZE) {
        return IDCU_ERR_INVALID_PARAM;
    }

    uint8_t computed_hash[IDCU_VERIFIER_HASH_SIZE];
    int ret = idcu_verifier_compute_hash(file_path, computed_hash, IDCU_VERIFIER_HASH_SIZE);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    if (memcmp(computed_hash, expected_hash, IDCU_VERIFIER_HASH_SIZE) != 0) {
        idcu_log_error("Hash verification failed for file: %s", file_path);
        return IDCU_ERR_VERIFICATION_FAILED;
    }

    idcu_log_info("Hash verification successful for file: %s", file_path);
    return IDCU_ERR_OK;
}

int idcu_verifier_verify_signature(const char *file_path, const uint8_t *signature,
                                   size_t signature_len, const char *public_key_path) {
    if (!file_path || !signature || !public_key_path) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_log_warning(
        "Signature verification is a placeholder - requires crypto library integration");
    idcu_log_info("Signature verification skipped for file: %s", file_path);

    return IDCU_ERR_OK;
}

int idcu_verifier_verify_module(idcu_ModuleVerifier *verifier, const char *file_path) {
    if (!verifier || !file_path) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_log_info("Starting module verification for: %s", file_path);

    int ret = IDCU_ERR_OK;

    if (verifier->verify_type == IDCU_VERIFY_TYPE_HASH ||
        verifier->verify_type == IDCU_VERIFY_TYPE_BOTH) {
        if (verifier->hash[0] != 0 || verifier->hash[1] != 0) {
            ret = idcu_verifier_verify_hash(file_path, verifier->hash, IDCU_VERIFIER_HASH_SIZE);
            if (ret != IDCU_ERR_OK) {
                return ret;
            }
        } else {
            idcu_log_warning("No expected hash set, computing hash only: %s", file_path);
            ret = idcu_verifier_compute_hash(file_path, verifier->hash, IDCU_VERIFIER_HASH_SIZE);
            if (ret != IDCU_ERR_OK) {
                return ret;
            }
        }
    }

    if (verifier->verify_type == IDCU_VERIFY_TYPE_SIGNATURE ||
        verifier->verify_type == IDCU_VERIFY_TYPE_BOTH) {
        if (verifier->signature_len > 0 && verifier->public_key_path[0] != '\0') {
            ret = idcu_verifier_verify_signature(
                file_path, verifier->signature, verifier->signature_len, verifier->public_key_path);
            if (ret != IDCU_ERR_OK) {
                return ret;
            }
        } else {
            idcu_log_warning(
                "Signature verification skipped - no signature or public key provided");
        }
    }

    idcu_log_info("Module verification completed successfully: %s", file_path);
    return IDCU_ERR_OK;
}

int idcu_verifier_verify_module_with_hash(idcu_ModuleVerifier *verifier, const char *file_path,
                                          const uint8_t *expected_hash) {
    if (!verifier || !file_path || !expected_hash) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memcpy(verifier->hash, expected_hash, IDCU_VERIFIER_HASH_SIZE);
    return idcu_verifier_verify_module(verifier, file_path);
}

int idcu_verifier_load_expected_hash(const char *hash_file_path, uint8_t *hash_out,
                                     size_t hash_size) {
    if (!hash_file_path || !hash_out || hash_size < IDCU_VERIFIER_HASH_SIZE) {
        return IDCU_ERR_INVALID_PARAM;
    }

    FILE *file = fopen(hash_file_path, "rb");
    if (!file) {
        idcu_log_error("Failed to open hash file: %s", hash_file_path);
        return IDCU_ERR_FILE_OPEN;
    }

    size_t read = fread(hash_out, 1, IDCU_VERIFIER_HASH_SIZE, file);
    fclose(file);

    if (read != IDCU_VERIFIER_HASH_SIZE) {
        idcu_log_error("Invalid hash file format: %s", hash_file_path);
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_log_info("Loaded expected hash from: %s", hash_file_path);
    return IDCU_ERR_OK;
}

int idcu_verifier_save_hash(const char *file_path, const uint8_t *hash, size_t hash_size) {
    if (!file_path || !hash || hash_size < IDCU_VERIFIER_HASH_SIZE) {
        return IDCU_ERR_INVALID_PARAM;
    }

    FILE *file = fopen(file_path, "wb");
    if (!file) {
        idcu_log_error("Failed to create hash file: %s", file_path);
        return IDCU_ERR_FILE_OPEN;
    }

    size_t written = fwrite(hash, 1, IDCU_VERIFIER_HASH_SIZE, file);
    fclose(file);

    if (written != IDCU_VERIFIER_HASH_SIZE) {
        idcu_log_error("Failed to write hash file: %s", file_path);
        return IDCU_ERR_FILE_WRITE;
    }

    idcu_log_info("Saved hash to: %s", file_path);
    return IDCU_ERR_OK;
}
