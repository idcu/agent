#include "idcu/os/os.h"
#include <stdlib.h>
#include <ucontext.h>
#include <string.h>

struct idcu_coro_t {
    ucontext_t ctx;
    void* stack;
    size_t stack_size;
    idcu_coro_func_t func;
    void* arg;
    bool done;
};

static ucontext_t g_main_ctx;
static idcu_coro_t* g_current_coro = NULL;

static void coro_trampoline(void)
{
    if (g_current_coro && g_current_coro->func) {
        g_current_coro->func(g_current_coro->arg);
        g_current_coro->done = true;
    }
}

idcu_coro_t* idcu_coro_create(idcu_coro_func_t func, void* arg, size_t stack_size)
{
    if (!func) {
        return NULL;
    }
    
    if (stack_size == 0) {
        stack_size = 64 * 1024;
    }
    
    idcu_coro_t* coro = (idcu_coro_t*)malloc(sizeof(idcu_coro_t));
    if (!coro) {
        return NULL;
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
    
    if (getcontext(&coro->ctx) == -1) {
        free(coro->stack);
        free(coro);
        return NULL;
    }
    
    coro->ctx.uc_stack.ss_sp = coro->stack;
    coro->ctx.uc_stack.ss_size = stack_size;
    coro->ctx.uc_link = &g_main_ctx;
    
    makecontext(&coro->ctx, coro_trampoline, 0);
    
    return coro;
}

void idcu_coro_destroy(idcu_coro_t* coro)
{
    if (!coro) {
        return;
    }
    if (coro->stack) {
        free(coro->stack);
    }
    free(coro);
}

int idcu_coro_yield(void)
{
    if (!g_current_coro) {
        return -1;
    }
    idcu_coro_t* coro = g_current_coro;
    g_current_coro = NULL;
    swapcontext(&coro->ctx, &g_main_ctx);
    return 0;
}

int idcu_coro_resume(idcu_coro_t* coro)
{
    if (!coro || coro->done) {
        return -1;
    }
    g_current_coro = coro;
    swapcontext(&g_main_ctx, &coro->ctx);
    return 0;
}

bool idcu_coro_is_done(idcu_coro_t* coro)
{
    return coro ? coro->done : true;
}
