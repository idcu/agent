#include "idcu/os/os.h"
#include <stdlib.h>
#include <windows.h>
#include <string.h>

struct idcu_coro_t {
    LPVOID fiber;
    LPVOID prev_fiber;
    idcu_coro_func_t func;
    void* arg;
    bool done;
    size_t stack_size;
};

static idcu_coro_t* g_current_coro = NULL;
static LPVOID g_main_fiber = NULL;

static void WINAPI coro_trampoline(LPVOID param)
{
    idcu_coro_t* coro = (idcu_coro_t*)param;
    if (coro && coro->func) {
        coro->func(coro->arg);
        coro->done = true;
    }
    SwitchToFiber(g_main_fiber);
}

idcu_coro_t* idcu_coro_create(idcu_coro_func_t func, void* arg, size_t stack_size)
{
    if (!func) {
        return NULL;
    }
    
    if (!g_main_fiber) {
        g_main_fiber = ConvertThreadToFiber(NULL);
    }
    
    if (stack_size == 0) {
        stack_size = 64 * 1024;
    }
    
    idcu_coro_t* coro = (idcu_coro_t*)malloc(sizeof(idcu_coro_t));
    if (!coro) {
        return NULL;
    }
    
    coro->func = func;
    coro->arg = arg;
    coro->done = false;
    coro->stack_size = stack_size;
    
    coro->fiber = CreateFiber(stack_size, coro_trampoline, coro);
    if (!coro->fiber) {
        free(coro);
        return NULL;
    }
    
    return coro;
}

void idcu_coro_destroy(idcu_coro_t* coro)
{
    if (!coro) {
        return;
    }
    if (coro->fiber) {
        DeleteFiber(coro->fiber);
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
    SwitchToFiber(g_main_fiber);
    return 0;
}

int idcu_coro_resume(idcu_coro_t* coro)
{
    if (!coro || coro->done) {
        return -1;
    }
    g_current_coro = coro;
    SwitchToFiber(coro->fiber);
    return 0;
}

bool idcu_coro_is_done(idcu_coro_t* coro)
{
    return coro ? coro->done : true;
}
