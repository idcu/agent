#include "idcu/common/security.h"
#include <limits.h>
#include <string.h>

int idcu_sec_check_bounds(const void *base, size_t base_size, const void *ptr, size_t access_size) {
    if (!base || !ptr) {
        return IDCU_ERR_INVALID_PARAM;
    }

    const uint8_t *base_ptr = (const uint8_t *)base;
    const uint8_t *access_ptr = (const uint8_t *)ptr;

    if (access_ptr < base_ptr) {
        return IDCU_ERR_SECURITY_ERROR;
    }

    if (access_ptr + access_size > base_ptr + base_size) {
        return IDCU_ERR_SECURITY_ERROR;
    }

    return IDCU_ERR_OK;
}

int idcu_sec_validate_string(const char *str, size_t max_len) {
    if (!str) {
        return IDCU_ERR_INVALID_PARAM;
    }

    size_t len = 0;
    while (str[len] != '\0') {
        if (len >= max_len) {
            return IDCU_ERR_BUFFER_TOO_SMALL;
        }
        len++;
    }

    return IDCU_ERR_OK;
}

int idcu_sec_validate_path(const char *path, size_t max_len) {
    if (!path) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_sec_validate_string(path, max_len);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    if (strstr(path, "../") != NULL || strstr(path, "..\\") != NULL) {
        return IDCU_ERR_SECURITY_ERROR;
    }

    return IDCU_ERR_OK;
}

int idcu_sec_safe_memcpy(void *dst, size_t dst_size, const void *src, size_t src_size) {
    if (!dst || !src) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (src_size > dst_size) {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }

    memcpy(dst, src, src_size);
    return IDCU_ERR_OK;
}

int idcu_sec_safe_strncpy(char *dst, const char *src, size_t dst_size) {
    if (!dst || !src) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (dst_size == 0) {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }

    size_t i;
    for (i = 0; i < dst_size - 1 && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';

    if (src[i] != '\0') {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }

    return IDCU_ERR_OK;
}

int idcu_sec_safe_strncat(char *dst, const char *src, size_t dst_size) {
    if (!dst || !src) {
        return IDCU_ERR_INVALID_PARAM;
    }

    size_t dst_len = strlen(dst);
    if (dst_len >= dst_size) {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }

    size_t remaining = dst_size - dst_len - 1;
    size_t i;
    for (i = 0; i < remaining && src[i] != '\0'; i++) {
        dst[dst_len + i] = src[i];
    }
    dst[dst_len + i] = '\0';

    if (src[i] != '\0') {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }

    return IDCU_ERR_OK;
}

void idcu_sec_zero_memory(void *ptr, size_t size) {
    if (!ptr) {
        return;
    }

    volatile uint8_t *p = (volatile uint8_t *)ptr;
    for (size_t i = 0; i < size; i++) {
        p[i] = 0;
    }
}

uint32_t idcu_sec_checksum(const void *data, size_t size) {
    if (!data || size == 0) {
        return 0;
    }

    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t checksum = 0;

    for (size_t i = 0; i < size; i++) {
        checksum = (checksum << 5) - checksum + bytes[i];
        checksum = checksum & 0xFFFFFFFF;
    }

    return checksum;
}

int idcu_sec_verify_checksum(const void *data, size_t size, uint32_t expected_checksum) {
    uint32_t actual = idcu_sec_checksum(data, size);
    return (actual == expected_checksum) ? IDCU_ERR_OK : IDCU_ERR_VERIFICATION_FAILED;
}

int idcu_sec_is_safe_integer_add(int32_t a, int32_t b) {
    if (b > 0 && a > INT_MAX - b) {
        return 0;
    }
    if (b < 0 && a < INT_MIN - b) {
        return 0;
    }
    return 1;
}

int idcu_sec_is_safe_integer_multiply(int32_t a, int32_t b) {
    if (a == 0 || b == 0) {
        return 1;
    }

    if (a > 0) {
        if (b > 0) {
            if (a > INT_MAX / b)
                return 0;
        } else {
            if (b < INT_MIN / a)
                return 0;
        }
    } else {
        if (b > 0) {
            if (a < INT_MIN / b)
                return 0;
        } else {
            if (a != 0 && b != 0) {
                if (a < INT_MAX / b)
                    return 0;
            }
        }
    }

    return 1;
}

int idcu_sec_safe_add(int32_t a, int32_t b, int32_t *result) {
    if (!result) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (!idcu_sec_is_safe_integer_add(a, b)) {
        return IDCU_ERR_SECURITY_ERROR;
    }

    *result = a + b;
    return IDCU_ERR_OK;
}

int idcu_sec_safe_multiply(int32_t a, int32_t b, int32_t *result) {
    if (!result) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (!idcu_sec_is_safe_integer_multiply(a, b)) {
        return IDCU_ERR_SECURITY_ERROR;
    }

    *result = a * b;
    return IDCU_ERR_OK;
}
