#include "idcu/os/os.h"
#include <rtthread.h>
#include <stdlib.h>

struct idcu_coro_t {
    rt_thread_t thread;
    void* stack;
    size_t stack_size;
    idcu_coro_func_t func;
    void* arg;
    int finished;
    void* result;
};

static idcu_coro_t* g_current_coro = NULL;

idcu_coro_t* idcu_coro_create(idcu_coro_func_t func, void* arg, size_t stack_size)
{
    idcu_coro_t* coro = (idcu_coro_t*)rt_malloc(sizeof(idcu_coro_t));
    if (!coro) {
        return NULL;
    }
    
    if (stack_size == 0) {
        stack_size = 4096;
    }
    
    coro->stack = rt_malloc(stack_size);
    if (!coro->stack) {
        rt_free(coro);
        return NULL;
    }
    
    coro->func = func;
    coro->arg = arg;
    coro->stack_size = stack_size;
    coro->finished = 0;
    coro->result = NULL;
    coro->thread = NULL;
    
    return coro;
}

void idcu_coro_destroy(idcu_coro_t* coro)
{
    if (!coro) {
        return;
    }
    if (coro->stack) {
        rt_free(coro->stack);
    }
    rt_free(coro);
}

int idcu_coro_resume(idcu_coro_t* coro)
{
    if (!coro || coro->finished) {
        return -1;
    }
    
    g_current_coro = coro;
    coro->result = coro->func(coro->arg);
    coro->finished = 1;
    g_current_coro = NULL;
    
    return 0;
}

int idcu_coro_yield(void)
{
    return 0;
}

idcu_coro_t* idcu_coro_current(void)
{
    return g_current_coro;
}
