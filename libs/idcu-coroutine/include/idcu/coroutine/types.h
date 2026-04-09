#ifndef IDCU_COROUTINE_TYPES_H
#define IDCU_COROUTINE_TYPES_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_COROUTINE_DEFAULT_STACK_SIZE (64 * 1024)
#define IDCU_COROUTINE_MAX_COROUTINES 128

typedef enum idcu_CoroutineState {
    IDCU_CORO_READY,
    IDCU_CORO_RUNNING,
    IDCU_CORO_WAITING,
    IDCU_CORO_SUSPENDED,
    IDCU_CORO_FINISHED
} idcu_CoroutineState;

typedef struct idcu_Coroutine idcu_Coroutine;
typedef struct idcu_CoroutineScheduler idcu_CoroutineScheduler;

typedef void (*idcu_CoroutineFunc)(void* arg);

typedef struct idcu_CoroutineConfig {
    size_t stack_size;
    const char* name;
} idcu_CoroutineConfig;

#ifdef __cplusplus
}
#endif

#endif
