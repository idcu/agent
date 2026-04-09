#include <idcu/coroutine/coroutine.h>
#include <idcu/common/vector.h>
#include <idcu/common/lock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <ucontext.h>
#include <unistd.h>
#endif

typedef struct idcu_Coroutine {
    idcu_CoroutineState state;
    char name[64];
    void* stack;
    size_t stack_size;
    idcu_CoroutineFunc func;
    void* arg;
    idcu_CoroutineScheduler* scheduler;

#ifdef _WIN32
    LPVOID fiber;
    LPVOID prev_fiber;
#else
    ucontext_t ctx;
    ucontext_t main_ctx;
#endif
} idcu_Coroutine;

typedef struct idcu_CoroutineScheduler {
    idcu_Vector ready_queue;
    idcu_Vector all_coroutines;
    idcu_Coroutine* current_coro;
    volatile int running;
    idcu_Mutex mutex;

#ifdef _WIN32
    LPVOID main_fiber;
#else
    ucontext_t scheduler_ctx;
#endif
} idcu_CoroutineScheduler;

static __thread idcu_CoroutineScheduler* g_current_scheduler = NULL;
static __thread idcu_Coroutine* g_current_coro = NULL;

#ifdef _WIN32

static void WINAPI coroutine_trampoline(LPVOID param) {
    idcu_Coroutine* coro = (idcu_Coroutine*)param;
    g_current_coro = coro;
    coro->state = IDCU_CORO_RUNNING;
    
    if (coro->func) {
        coro->func(coro->arg);
    }
    
    coro->state = IDCU_CORO_FINISHED;
    SwitchToFiber(coro->prev_fiber);
}

#else

static void coroutine_trampoline(void) {
    idcu_Coroutine* coro = g_current_coro;
    if (!coro) return;
    
    coro->state = IDCU_CORO_RUNNING;
    
    if (coro->func) {
        coro->func(coro->arg);
    }
    
    coro->state = IDCU_CORO_FINISHED;
    swapcontext(&coro->ctx, &coro->scheduler->scheduler_ctx);
}

#endif

int idcu_coro_scheduler_init(idcu_CoroutineScheduler** scheduler) {
    if (!scheduler) return IDCU_ERR_INVALID_ARG;
    
    idcu_CoroutineScheduler* s = (idcu_CoroutineScheduler*)calloc(1, sizeof(idcu_CoroutineScheduler));
    if (!s) return IDCU_ERR_OUT_OF_MEM;
    
    int result = idcu_vector_init(&s->ready_queue, sizeof(idcu_Coroutine*), 16);
    if (result != IDCU_SUCCESS) {
        free(s);
        return result;
    }
    
    result = idcu_vector_init(&s->all_coroutines, sizeof(idcu_Coroutine*), 16);
    if (result != IDCU_SUCCESS) {
        idcu_vector_destroy(&s->ready_queue);
        free(s);
        return result;
    }
    
    result = idcu_mutex_init(&s->mutex);
    if (result != IDCU_SUCCESS) {
        idcu_vector_destroy(&s->ready_queue);
        idcu_vector_destroy(&s->all_coroutines);
        free(s);
        return result;
    }
    
    s->current_coro = NULL;
    s->running = 0;
    
#ifdef _WIN32
    s->main_fiber = ConvertThreadToFiber(NULL);
    if (!s->main_fiber) {
        idcu_mutex_destroy(&s->mutex);
        idcu_vector_destroy(&s->ready_queue);
        idcu_vector_destroy(&s->all_coroutines);
        free(s);
        return IDCU_ERR_GENERAL;
    }
#endif
    
    *scheduler = s;
    g_current_scheduler = s;
    return IDCU_SUCCESS;
}

int idcu_coro_scheduler_destroy(idcu_CoroutineScheduler* scheduler) {
    if (!scheduler) return IDCU_ERR_INVALID_ARG;
    
    idcu_mutex_lock(&scheduler->mutex);
    
    size_t count = idcu_vector_size(&scheduler->all_coroutines);
    for (size_t i = 0; i < count; i++) {
        idcu_Coroutine** coro_ptr = (idcu_Coroutine**)idcu_vector_get(&scheduler->all_coroutines, i);
        if (coro_ptr && *coro_ptr) {
            idcu_Coroutine* coro = *coro_ptr;
            
            if (coro->stack) {
                free(coro->stack);
            }
#ifdef _WIN32
            if (coro->fiber) {
                DeleteFiber(coro->fiber);
            }
#endif
            free(coro);
        }
    }
    
    idcu_vector_destroy(&scheduler->ready_queue);
    idcu_vector_destroy(&scheduler->all_coroutines);
    idcu_mutex_unlock(&scheduler->mutex);
    idcu_mutex_destroy(&scheduler->mutex);
    
#ifdef _WIN32
    ConvertFiberToThread();
#endif
    
    free(scheduler);
    if (g_current_scheduler == scheduler) {
        g_current_scheduler = NULL;
    }
    
    return IDCU_SUCCESS;
}

int idcu_coro_scheduler_run(idcu_CoroutineScheduler* scheduler) {
    if (!scheduler) return IDCU_ERR_INVALID_ARG;
    
    scheduler->running = 1;
    g_current_scheduler = scheduler;
    
    while (scheduler->running) {
        idcu_mutex_lock(&scheduler->mutex);
        
        if (idcu_vector_empty(&scheduler->ready_queue)) {
            idcu_mutex_unlock(&scheduler->mutex);
            break;
        }
        
        idcu_Coroutine** coro_ptr = (idcu_Coroutine**)idcu_vector_get(&scheduler->ready_queue, 0);
        idcu_vector_remove(&scheduler->ready_queue, 0);
        
        idcu_mutex_unlock(&scheduler->mutex);
        
        if (!coro_ptr || !*coro_ptr) continue;
        
        idcu_Coroutine* coro = *coro_ptr;
        
        if (coro->state == IDCU_CORO_FINISHED) continue;
        
        scheduler->current_coro = coro;
        g_current_coro = coro;
        coro->state = IDCU_CORO_RUNNING;
        
#ifdef _WIN32
        coro->prev_fiber = scheduler->main_fiber;
        SwitchToFiber(coro->fiber);
#else
        swapcontext(&scheduler->scheduler_ctx, &coro->ctx);
#endif
        
        scheduler->current_coro = NULL;
        g_current_coro = NULL;
        
        if (coro->state == IDCU_CORO_FINISHED) {
            idcu_coro_destroy(coro);
        }
    }
    
    scheduler->running = 0;
    return IDCU_SUCCESS;
}

int idcu_coro_scheduler_stop(idcu_CoroutineScheduler* scheduler) {
    if (!scheduler) return IDCU_ERR_INVALID_ARG;
    scheduler->running = 0;
    return IDCU_SUCCESS;
}

int idcu_coro_create(idcu_CoroutineScheduler* scheduler, idcu_Coroutine** coro,
                      idcu_CoroutineFunc func, void* arg,
                      const idcu_CoroutineConfig* config) {
    if (!scheduler || !coro || !func) return IDCU_ERR_INVALID_ARG;
    
    idcu_Coroutine* c = (idcu_Coroutine*)calloc(1, sizeof(idcu_Coroutine));
    if (!c) return IDCU_ERR_OUT_OF_MEM;
    
    c->state = IDCU_CORO_READY;
    c->func = func;
    c->arg = arg;
    c->scheduler = scheduler;
    
    size_t stack_size = config ? config->stack_size : IDCU_COROUTINE_DEFAULT_STACK_SIZE;
    if (stack_size == 0) stack_size = IDCU_COROUTINE_DEFAULT_STACK_SIZE;
    
    c->stack_size = stack_size;
    c->stack = malloc(stack_size);
    if (!c->stack) {
        free(c);
        return IDCU_ERR_OUT_OF_MEM;
    }
    
    if (config && config->name) {
        strncpy(c->name, config->name, sizeof(c->name) - 1);
        c->name[sizeof(c->name) - 1] = '\0';
    } else {
        snprintf(c->name, sizeof(c->name), "coro-%u", (unsigned int)idcu_vector_size(&scheduler->all_coroutines));
    }
    
#ifdef _WIN32
    c->fiber = CreateFiber(stack_size, coroutine_trampoline, c);
    if (!c->fiber) {
        free(c->stack);
        free(c);
        return IDCU_ERR_GENERAL;
    }
#else
    if (getcontext(&c->ctx) == -1) {
        free(c->stack);
        free(c);
        return IDCU_ERR_GENERAL;
    }
    
    c->ctx.uc_stack.ss_sp = c->stack;
    c->ctx.uc_stack.ss_size = stack_size;
    c->ctx.uc_link = &scheduler->scheduler_ctx;
    
    makecontext(&c->ctx, coroutine_trampoline, 0);
#endif
    
    idcu_mutex_lock(&scheduler->mutex);
    idcu_vector_push_back(&scheduler->all_coroutines, &c);
    idcu_vector_push_back(&scheduler->ready_queue, &c);
    idcu_mutex_unlock(&scheduler->mutex);
    
    *coro = c;
    return IDCU_SUCCESS;
}

int idcu_coro_destroy(idcu_Coroutine* coro) {
    if (!coro) return IDCU_ERR_INVALID_ARG;
    
    idcu_CoroutineScheduler* scheduler = coro->scheduler;
    if (scheduler) {
        idcu_mutex_lock(&scheduler->mutex);
        size_t count = idcu_vector_size(&scheduler->all_coroutines);
        for (size_t i = 0; i < count; i++) {
            idcu_Coroutine** coro_ptr = (idcu_Coroutine**)idcu_vector_get(&scheduler->all_coroutines, i);
            if (coro_ptr && *coro_ptr == coro) {
                idcu_vector_remove(&scheduler->all_coroutines, i);
                break;
            }
        }
        
        count = idcu_vector_size(&scheduler->ready_queue);
        for (size_t i = 0; i < count; i++) {
            idcu_Coroutine** coro_ptr = (idcu_Coroutine**)idcu_vector_get(&scheduler->ready_queue, i);
            if (coro_ptr && *coro_ptr == coro) {
                idcu_vector_remove(&scheduler->ready_queue, i);
                break;
            }
        }
        idcu_mutex_unlock(&scheduler->mutex);
    }
    
    if (coro->stack) {
        free(coro->stack);
    }
#ifdef _WIN32
    if (coro->fiber) {
        DeleteFiber(coro->fiber);
    }
#endif
    free(coro);
    
    return IDCU_SUCCESS;
}

int idcu_coro_yield(void) {
    idcu_Coroutine* coro = g_current_coro;
    if (!coro) return IDCU_ERR_GENERAL;
    
    idcu_CoroutineScheduler* scheduler = coro->scheduler;
    if (!scheduler) return IDCU_ERR_GENERAL;
    
    coro->state = IDCU_CORO_READY;
    
    idcu_mutex_lock(&scheduler->mutex);
    idcu_vector_push_back(&scheduler->ready_queue, &coro);
    idcu_mutex_unlock(&scheduler->mutex);
    
#ifdef _WIN32
    SwitchToFiber(coro->prev_fiber);
#else
    swapcontext(&coro->ctx, &scheduler->scheduler_ctx);
#endif
    
    return IDCU_SUCCESS;
}

int idcu_coro_resume(idcu_Coroutine* coro) {
    if (!coro) return IDCU_ERR_INVALID_ARG;
    
    idcu_CoroutineScheduler* scheduler = coro->scheduler;
    if (!scheduler) return IDCU_ERR_GENERAL;
    
    if (coro->state == IDCU_CORO_FINISHED) return IDCU_ERR_GENERAL;
    
    coro->state = IDCU_CORO_READY;
    
    idcu_mutex_lock(&scheduler->mutex);
    idcu_vector_push_back(&scheduler->ready_queue, &coro);
    idcu_mutex_unlock(&scheduler->mutex);
    
    return IDCU_SUCCESS;
}

idcu_CoroutineState idcu_coro_get_state(const idcu_Coroutine* coro) {
    if (!coro) return IDCU_CORO_FINISHED;
    return coro->state;
}

const char* idcu_coro_get_name(const idcu_Coroutine* coro) {
    if (!coro) return NULL;
    return coro->name;
}

idcu_Coroutine* idcu_coro_current(void) {
    return g_current_coro;
}
