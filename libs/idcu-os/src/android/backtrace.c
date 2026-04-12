#include "idcu/os/os.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if IDCU_OS_ANDROID

#include <android/api-level.h>

#if __ANDROID_API__ >= 33
#include <unwind.h>
#else
typedef int _Unwind_Reason_Code;
typedef struct _Unwind_Context _Unwind_Context;
#endif

typedef struct {
    void** frames;
    size_t max_frames;
    size_t current;
} backtrace_state_t;

#if __ANDROID_API__ >= 33
static _Unwind_Reason_Code unwind_callback(struct _Unwind_Context* ctx, void* arg)
{
    backtrace_state_t* state = (backtrace_state_t*)arg;
    if (state->current >= state->max_frames) {
        return _URC_END_OF_STACK;
    }
    
    uintptr_t ip = _Unwind_GetIP(ctx);
    state->frames[state->current++] = (void*)ip;
    return _URC_NO_REASON;
}
#endif

int idcu_backtrace_capture(void** frames, size_t max_frames)
{
    if (!frames || max_frames == 0) {
        return 0;
    }
    
#if __ANDROID_API__ >= 33
    backtrace_state_t state;
    state.frames = frames;
    state.max_frames = max_frames;
    state.current = 0;
    
    _Unwind_Backtrace(unwind_callback, &state);
    return state.current;
#else
    void* h = dlopen("libcorkscrew.so", RTLD_LAZY);
    if (h) {
        typedef int (*backtrace_func)(void**, int);
        backtrace_func bt = (backtrace_func)dlsym(h, "backtrace");
        if (bt) {
            int result = bt(frames, (int)max_frames);
            dlclose(h);
            return result;
        }
        dlclose(h);
    }
    
    h = dlopen("libunwind.so", RTLD_LAZY);
    if (h) {
        typedef int (*backtrace_func)(void**, int);
        backtrace_func bt = (backtrace_func)dlsym(h, "backtrace");
        if (bt) {
            int result = bt(frames, (int)max_frames);
            dlclose(h);
            return result;
        }
        dlclose(h);
    }
    
    return 0;
#endif
}

int idcu_backtrace_format(void** frames, size_t num_frames, char* buf, size_t buf_len)
{
    if (!frames || !buf || buf_len == 0 || num_frames == 0) {
        return -1;
    }
    
    size_t offset = 0;
    int written;
    
    for (size_t i = 0; i < num_frames && offset < buf_len - 1; i++) {
        Dl_info info;
        if (dladdr(frames[i], &info) && info.dli_sname) {
            written = snprintf(buf + offset, buf_len - offset,
                             "#%zu: %p in %s\n",
                             i, frames[i], info.dli_sname);
        } else {
            written = snprintf(buf + offset, buf_len - offset,
                             "#%zu: %p\n",
                             i, frames[i]);
        }
        
        if (written > 0) {
            offset += written;
        }
    }
    
    buf[offset] = '\0';
    return 0;
}

#endif
