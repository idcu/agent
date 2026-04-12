#include "idcu/os/os.h"
#include <rtthread.h>

uint64_t idcu_time_now_ms(void)
{
    rt_tick_t tick = rt_tick_get();
    return (uint64_t)tick * (1000 / RT_TICK_PER_SECOND);
}

uint64_t idcu_time_now_us(void)
{
    rt_tick_t tick = rt_tick_get();
    return (uint64_t)tick * (1000000 / RT_TICK_PER_SECOND);
}

uint64_t idcu_time_now_ns(void)
{
    rt_tick_t tick = rt_tick_get();
    return (uint64_t)tick * (1000000000 / RT_TICK_PER_SECOND);
}

void idcu_sleep_ms(uint32_t ms)
{
    rt_thread_mdelay(ms);
}

void idcu_sleep_us(uint32_t us)
{
    rt_thread_mdelay((us + 999) / 1000);
}
