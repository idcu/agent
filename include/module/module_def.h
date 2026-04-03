#ifndef IDCU_MODULE_MODULE_DEF_H
#define IDCU_MODULE_MODULE_DEF_H

#include <stdint.h>
#include "common/config.h"
#include "common/error_code.h"
#include "utils/log.h"

// æ¨¡åç¶æ?
typedef enum {
    MOD_STATE_UNINIT   = 0,
    MOD_STATE_INITED   = 1,
    MOD_STATE_RUNNING  = 2,
    MOD_STATE_STOPPED  = 3,
    MOD_STATE_ERROR    = 4
} ModuleState;

// æ¨¡åä¼åçº?
typedef enum {
    MOD_PRIO_LOW       = 0,
    MOD_PRIO_NORMAL    = 1,
    MOD_PRIO_HIGH      = 2,
    MOD_PRIO_REALTIME  = 3
} ModulePrio;

// æ¨¡åæ¥å£å®ä¹
typedef struct {
    const char*         name;           // æ¨¡åå?
    int (*init)(void);                  // åå§å?
    int (*run)(void);                   // è¿è¡
    int (*stop)(void);                  // åæ­¢
} ModuleInterface;

#define REGISTER_MODULE(name, init_fn, run_fn, stop_fn) \
    const ModuleInterface __module_##name \
        __attribute__((section(".modules"))) = { \
            #name, init_fn, run_fn, stop_fn \
        }

#endif // IDCU_MODULE_MODULE_DEF_H