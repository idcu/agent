#include "idcu/os/os.h"
#include <windows.h>

uint64_t idcu_time_now_ms(void)
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return (uli.QuadPart - 116444736000000000LL) / 10000;
}

uint64_t idcu_time_now_us(void)
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return (uli.QuadPart - 116444736000000000LL) / 10;
}

uint64_t idcu_time_now_ns(void)
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return (uli.QuadPart - 116444736000000000LL) * 100;
}

void idcu_sleep_ms(uint32_t ms)
{
    Sleep(ms);
}

void idcu_sleep_us(uint32_t us)
{
    Sleep(us / 1000 > 0 ? us / 1000 : 1);
}
