#include "idcu/common/security.h"
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#else
#include <unistd.h>
#include <fcntl.h>
#endif

void idcu_secure_zero(void* ptr, size_t size)
{
    if (!ptr || size == 0) {
        return;
    }
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (size--) {
        *p++ = 0;
    }
}

int idcu_memcmp_constant(const void* a, const void* b, size_t size)
{
    if (a == b) {
        return 0;
    }
    if (!a || !b) {
        return -1;
    }

    const volatile unsigned char* pa = (const volatile unsigned char*)a;
    const volatile unsigned char* pb = (const volatile unsigned char*)b;
    volatile unsigned char diff = 0;

    for (size_t i = 0; i < size; ++i) {
        diff |= pa[i] ^ pb[i];
    }

    return (int)diff;
}

static uint64_t idcu_seed_random(void)
{
    uint64_t seed = 0;

#ifdef _WIN32
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    seed ^= (uint64_t)counter.QuadPart;
    seed ^= (uint64_t)GetCurrentProcessId();
    seed ^= (uint64_t)GetCurrentThreadId();
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    seed ^= (uint64_t)ts.tv_sec;
    seed ^= (uint64_t)ts.tv_nsec;
    seed ^= (uint64_t)getpid();
#endif

    return seed;
}

uint64_t idcu_random_uint64(void)
{
    static uint64_t state = 0;
    if (state == 0) {
        state = idcu_seed_random();
    }

    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;

    return state;
}

void idcu_random_bytes(uint8_t* buffer, size_t size)
{
    if (!buffer || size == 0) {
        return;
    }

#ifdef _WIN32
    HCRYPTPROV hProv = 0;
    if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        CryptGenRandom(hProv, (DWORD)size, buffer);
        CryptReleaseContext(hProv, 0);
        return;
    }
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) {
        read(fd, buffer, size);
        close(fd);
        return;
    }
#endif

    for (size_t i = 0; i < size; i += sizeof(uint64_t)) {
        uint64_t rand_val = idcu_random_uint64();
        size_t copy_size = (size - i) < sizeof(uint64_t) ? (size - i) : sizeof(uint64_t);
        memcpy(buffer + i, &rand_val, copy_size);
    }
}

bool idcu_safe_add_uint64(uint64_t a, uint64_t b, uint64_t* result)
{
    if (!result) {
        return false;
    }
    if (b > UINT64_MAX - a) {
        return false;
    }
    *result = a + b;
    return true;
}

bool idcu_safe_sub_uint64(uint64_t a, uint64_t b, uint64_t* result)
{
    if (!result) {
        return false;
    }
    if (a < b) {
        return false;
    }
    *result = a - b;
    return true;
}

bool idcu_safe_mul_uint64(uint64_t a, uint64_t b, uint64_t* result)
{
    if (!result) {
        return false;
    }
    if (a != 0 && b > UINT64_MAX / a) {
        return false;
    }
    *result = a * b;
    return true;
}

bool idcu_safe_add_size_t(size_t a, size_t b, size_t* result)
{
    if (!result) {
        return false;
    }
    if (b > SIZE_MAX - a) {
        return false;
    }
    *result = a + b;
    return true;
}

bool idcu_safe_sub_size_t(size_t a, size_t b, size_t* result)
{
    if (!result) {
        return false;
    }
    if (a < b) {
        return false;
    }
    *result = a - b;
    return true;
}

bool idcu_safe_mul_size_t(size_t a, size_t b, size_t* result)
{
    if (!result) {
        return false;
    }
    if (a != 0 && b > SIZE_MAX / a) {
        return false;
    }
    *result = a * b;
    return true;
}

bool idcu_safe_add_int64(int64_t a, int64_t b, int64_t* result)
{
    if (!result) {
        return false;
    }
    if ((b > 0 && a > INT64_MAX - b) || (b < 0 && a < INT64_MIN - b)) {
        return false;
    }
    *result = a + b;
    return true;
}

bool idcu_safe_sub_int64(int64_t a, int64_t b, int64_t* result)
{
    if (!result) {
        return false;
    }
    if ((b < 0 && a > INT64_MAX + b) || (b > 0 && a < INT64_MIN + b)) {
        return false;
    }
    *result = a - b;
    return true;
}

bool idcu_safe_mul_int64(int64_t a, int64_t b, int64_t* result)
{
    if (!result) {
        return false;
    }
    if (a == 0 || b == 0) {
        *result = 0;
        return true;
    }
    if (a > 0) {
        if (b > 0) {
            if (a > INT64_MAX / b) {
                return false;
            }
        } else {
            if (b < INT64_MIN / a) {
                return false;
            }
        }
    } else {
        if (b > 0) {
            if (a < INT64_MIN / b) {
                return false;
            }
        } else {
            if (a != 0 && b < INT64_MAX / a) {
                return false;
            }
        }
    }
    *result = a * b;
    return true;
}

int idcu_safe_memcpy(void* dst, size_t dst_size, const void* src, size_t count)
{
    if (!dst || !src) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (count > dst_size) {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    if (count == 0) {
        return IDCU_ERR_OK;
    }
    memcpy(dst, src, count);
    return IDCU_ERR_OK;
}

int idcu_safe_memmove(void* dst, size_t dst_size, const void* src, size_t count)
{
    if (!dst || !src) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (count > dst_size) {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    if (count == 0) {
        return IDCU_ERR_OK;
    }
    memmove(dst, src, count);
    return IDCU_ERR_OK;
}

int idcu_safe_memset(void* dst, size_t dst_size, int c, size_t count)
{
    if (!dst) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (count > dst_size) {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    if (count == 0) {
        return IDCU_ERR_OK;
    }
    memset(dst, c, count);
    return IDCU_ERR_OK;
}

int idcu_safe_strcpy(char* dst, size_t dst_size, const char* src)
{
    if (!dst || !src) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (dst_size == 0) {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    size_t src_len = strlen(src);
    if (src_len >= dst_size) {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    memcpy(dst, src, src_len + 1);
    return IDCU_ERR_OK;
}

int idcu_safe_strncpy(char* dst, size_t dst_size, const char* src, size_t count)
{
    if (!dst || !src) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (dst_size == 0) {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    size_t src_len = strlen(src);
    size_t copy_len = (count < src_len) ? count : src_len;
    if (copy_len >= dst_size) {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    memcpy(dst, src, copy_len);
    dst[copy_len] = '\0';
    return IDCU_ERR_OK;
}

int idcu_safe_strcat(char* dst, size_t dst_size, const char* src)
{
    if (!dst || !src) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (dst_size == 0) {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    size_t dst_len = strlen(dst);
    size_t src_len = strlen(src);
    if (dst_len + src_len >= dst_size) {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    memcpy(dst + dst_len, src, src_len + 1);
    return IDCU_ERR_OK;
}

int idcu_safe_strncat(char* dst, size_t dst_size, const char* src, size_t count)
{
    if (!dst || !src) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (dst_size == 0) {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    size_t dst_len = strlen(dst);
    size_t src_len = strlen(src);
    size_t copy_len = (count < src_len) ? count : src_len;
    if (dst_len + copy_len >= dst_size) {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    memcpy(dst + dst_len, src, copy_len);
    dst[dst_len + copy_len] = '\0';
    return IDCU_ERR_OK;
}

size_t idcu_safe_strlen(const char* str, size_t max_len)
{
    if (!str) {
        return 0;
    }
    size_t len = 0;
    while (len < max_len && str[len] != '\0') {
        len++;
    }
    return len;
}

int idcu_validate_input(const void* ptr, size_t size, size_t max_size)
{
    if (!ptr) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (size > max_size) {
        return IDCU_ERR_OUT_OF_RANGE;
    }
    return IDCU_ERR_OK;
}
