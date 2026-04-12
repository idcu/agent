#include "idcu/os/os.h"
#include <rtthread.h>
#include <string.h>

int idcu_backtrace_capture(void** frames, int max_frames)
{
    return 0;
}

int idcu_backtrace_format(void* const* frames, int num_frames, char* buf, size_t buf_len)
{
    if (!buf || buf_len == 0) {
        return -1;
    }
    strncpy(buf, "Backtrace not available on RT-Thread", buf_len - 1);
    buf[buf_len - 1] = '\0';
    return 0;
}
