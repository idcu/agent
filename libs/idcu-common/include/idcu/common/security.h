#ifndef IDCU_COMMON_SECURITY_H
#define IDCU_COMMON_SECURITY_H

#include "error_code.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void idcu_secure_zero(void* ptr, size_t size);
int  idcu_memcmp_constant(const void* a, const void* b, size_t size);

uint64_t idcu_random_uint64(void);
void     idcu_random_bytes(uint8_t* buffer, size_t size);

#define IDCU_CHECK_NULL(ptr) \
    ((ptr) == NULL)

#define IDCU_CHECK_NULL_RETURN(ptr, err_code) \
    do { \
        if (IDCU_CHECK_NULL(ptr)) { \
            return (err_code); \
        } \
    } while (0)

#define IDCU_CHECK_NULL_RETURN_VOID(ptr) \
    do { \
        if (IDCU_CHECK_NULL(ptr)) { \
            return; \
        } \
    } while (0)

#define IDCU_CHECK_RANGE(val, min, max) \
    ((val) >= (min) && (val) <= (max))

#define IDCU_CHECK_RANGE_RETURN(val, min, max, err_code) \
    do { \
        if (!IDCU_CHECK_RANGE((val), (min), (max))) { \
            return (err_code); \
        } \
    } while (0)

#define IDCU_CHECK_SIZE(size, max_size) \
    ((size) <= (max_size))

#define IDCU_CHECK_SIZE_RETURN(size, max_size, err_code) \
    do { \
        if (!IDCU_CHECK_SIZE((size), (max_size))) { \
            return (err_code); \
        } \
    } while (0)

#define IDCU_CHECK_PTR_AND_SIZE(ptr, size, max_size) \
    (!IDCU_CHECK_NULL(ptr) && IDCU_CHECK_SIZE((size), (max_size)))

#define IDCU_CHECK_PTR_AND_SIZE_RETURN(ptr, size, max_size, err_code) \
    do { \
        if (IDCU_CHECK_NULL(ptr)) { \
            return (err_code); \
        } \
        if (!IDCU_CHECK_SIZE((size), (max_size))) { \
            return (err_code); \
        } \
    } while (0)

bool idcu_safe_add_uint64(uint64_t a, uint64_t b, uint64_t* result);
bool idcu_safe_sub_uint64(uint64_t a, uint64_t b, uint64_t* result);
bool idcu_safe_mul_uint64(uint64_t a, uint64_t b, uint64_t* result);

bool idcu_safe_add_size_t(size_t a, size_t b, size_t* result);
bool idcu_safe_sub_size_t(size_t a, size_t b, size_t* result);
bool idcu_safe_mul_size_t(size_t a, size_t b, size_t* result);

bool idcu_safe_add_int64(int64_t a, int64_t b, int64_t* result);
bool idcu_safe_sub_int64(int64_t a, int64_t b, int64_t* result);
bool idcu_safe_mul_int64(int64_t a, int64_t b, int64_t* result);

int idcu_safe_memcpy(void* dst, size_t dst_size, const void* src, size_t count);
int idcu_safe_memmove(void* dst, size_t dst_size, const void* src, size_t count);
int idcu_safe_memset(void* dst, size_t dst_size, int c, size_t count);

int idcu_safe_strcpy(char* dst, size_t dst_size, const char* src);
int idcu_safe_strncpy(char* dst, size_t dst_size, const char* src, size_t count);
int idcu_safe_strcat(char* dst, size_t dst_size, const char* src);
int idcu_safe_strncat(char* dst, size_t dst_size, const char* src, size_t count);

size_t idcu_safe_strlen(const char* str, size_t max_len);

int idcu_validate_input(const void* ptr, size_t size, size_t max_size);

#ifdef __cplusplus
}
#endif

#endif
