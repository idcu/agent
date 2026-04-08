#ifndef IDCU_COMMON_SECURITY_H
#define IDCU_COMMON_SECURITY_H

#include "idcu/common/error_code.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    int idcu_sec_check_bounds(const void* base, size_t base_size, const void* ptr,
                              size_t access_size);

    int idcu_sec_validate_string(const char* str, size_t max_len);

    int idcu_sec_validate_path(const char* path, size_t max_len);

    int idcu_sec_safe_memcpy(void* dst, size_t dst_size, const void* src, size_t src_size);

    int idcu_sec_safe_strncpy(char* dst, const char* src, size_t dst_size);

    int idcu_sec_safe_strncat(char* dst, const char* src, size_t dst_size);

    void idcu_sec_zero_memory(void* ptr, size_t size);

    uint32_t idcu_sec_checksum(const void* data, size_t size);

    int idcu_sec_verify_checksum(const void* data, size_t size, uint32_t expected_checksum);

    int idcu_sec_is_safe_integer_add(int32_t a, int32_t b);

    int idcu_sec_is_safe_integer_multiply(int32_t a, int32_t b);

    int idcu_sec_safe_add(int32_t a, int32_t b, int32_t* result);

    int idcu_sec_safe_multiply(int32_t a, int32_t b, int32_t* result);

#ifdef __cplusplus
}
#endif

#endif
