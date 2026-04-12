#include "idcu/os/os.h"
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>

#if IDCU_OS_ANDROID

#define CORO_STACK_SIZE (1024 * 1024)

struct idcu_coro_t {
    jmp_buf ctx;
    jmp_buf caller_ctx;
    void* stack;
    size_t stack_size;
    idcu_coro_func_t func;
    void* arg;
    bool done;
    bool started;
};

static idcu_coro_t* g_current_coro = NULL;
static idcu_coro_t* g_main_coro = NULL;

static void coro_trampoline(int sig)
{
    (void)sig;
    
    if (g_current_coro && !g_current_coro->started) {
        g_current_coro->started = true;
        if (g_current_coro->func) {
            g_current_coro->func(g_current_coro->arg);
        }
        g_current_coro->done = true;
        
        if (g_main_coro) {
            longjmp(g_main_coro->ctx, 1);
        }
    }
}

idcu_coro_t* idcu_coro_create(idcu_coro_func_t func, void* arg, size_t stack_size)
{
    idcu_coro_t* coro = (idcu_coro_t*)malloc(sizeof(idcu_coro_t));
    if (!coro) {
        return NULL;
    }
    
    if (stack_size == 0) {
        stack_size = CORO_STACK_SIZE;
    }
    
    coro->stack = malloc(stack_size);
    if (!coro->stack) {
        free(coro);
        return NULL;
    }
    
    coro->stack_size = stack_size;
    coro->func = func;
    coro->arg = arg;
    coro->done = false;
    coro->started = false;
    
    if (!g_main_coro) {
        g_main_coro = (idcu_coro_t*)malloc(sizeof(idcu_coro_t));
        if (g_main_coro) {
            g_main_coro->stack = NULL;
            g_main_coro->done = false;
            g_main_coro->started = true;
        }
    }
    
    return coro;
}

void idcu_coro_destroy(idcu_coro_t* coro)
{
    if (coro) {
        if (coro->stack) {
            free(coro->stack);
        }
        free(coro);
    }
}

int idcu_coro_yield(void)
{
    if (!g_current_coro || g_current_coro->done) {
        return -1;
    }
    
    if (g_main_coro) {
        if (setjmp(g_current_coro->ctx) == 0) {
            longjmp(g_main_coro->ctx, 1);
        }
    }
    
    return 0;
}

int idcu_coro_resume(idcu_coro_t* coro)
{
    if (!coro || coro->done) {
        return -1;
    }
    
    idcu_coro_t* prev_coro = g_current_coro;
    g_current_coro = coro;
    
    if (!coro->started) {
        struct sigaction sa;
        sa.sa_handler = coro_trampoline;
        sa.sa_flags = 0;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGUSR1, &sa, NULL);
        
        if (setjmp(g_main_coro->ctx) == 0) {
            raise(SIGUSR1);
        }
    } else {
        if (setjmp(g_main_coro->ctx) == 0) {
            longjmp(coro->ctx, 1);
        }
    }
    
    g_current_coro = prev_coro;
    return coro->done ? 1 : 0;
}

bool idcu_coro_is_done(idcu_coro_t* coro)
{
    if (!coro) {
        return true;
    }
    return coro->done;
}

#endif
