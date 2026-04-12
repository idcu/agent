#include "idcu/os.h"
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

uint64_t idcu_time_now_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
}

uint64_t idcu_time_now_us(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + (uint64_t)tv.tv_usec;
}

uint64_t idcu_time_now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000000000 + (uint64_t)ts.tv_nsec;
}

void idcu_sleep_ms(uint32_t ms)
{
    usleep(ms * 1000);
}

void idcu_sleep_us(uint32_t us)
{
    usleep(us);
}
