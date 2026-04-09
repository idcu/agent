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
