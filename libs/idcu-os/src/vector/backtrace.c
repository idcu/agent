#include "idcu/os/os.h"
#include <execinfo.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int idcu_backtrace_capture(void** frames, size_t max_frames)
{
    return backtrace(frames, max_frames);
}

int idcu_backtrace_format(void** frames, size_t num_frames, char* buf, size_t buf_len)
{
    if (!frames || num_frames == 0 || !buf || buf_len == 0) {
        return -1;
    }
    
    char** symbols = backtrace_symbols(frames, num_frames);
    if (!symbols) {
        return -1;
    }
    
    size_t offset = 0;
    for (size_t i = 0; i < num_frames && offset < buf_len - 1; i++) {
        int written = snprintf(buf + offset, buf_len - offset, "#%zu: %s\n", i, symbols[i]);
        if (written > 0) {
            offset += written;
        }
    }
    
    free(symbols);
    return (int)offset;
}
