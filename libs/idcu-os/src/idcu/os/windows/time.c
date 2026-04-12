#include "idcu/os/os.h"
#include <windows.h>
#include <sys/timeb.h>

uint64_t idcu_time_now_ms(void)
{
    struct _timeb timebuffer;
    _ftime(&timebuffer);
    return (uint64_t)timebuffer.time * 1000 + (uint64_t)timebuffer.millitm;
}

uint64_t idcu_time_now_us(void)
{
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (uint64_t)(counter.QuadPart * 1000000 / frequency.QuadPart);
}

uint64_t idcu_time_now_ns(void)
{
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (uint64_t)(counter.QuadPart * 1000000000 / frequency.QuadPart);
}

void idcu_sleep_ms(uint32_t ms)
{
    Sleep(ms);
}

void idcu_sleep_us(uint32_t us)
{
    HANDLE timer;
    LARGE_INTEGER ft;
    ft.QuadPart = -(10 * (int64_t)us);
    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}
