#ifndef MODULE_DEF_H
#define MODULE_DEF_H

#include <stdint.h>
#include "config.h"
#include "error_code.h"
#include "log.h"

// 模块状态
typedef enum {
    MOD_STATE_UNINIT   = 0,
    MOD_STATE_INITED   = 1,
    MOD_STATE_RUNNING  = 2,
    MOD_STATE_STOPPED  = 3,
    MOD_STATE_ERROR    = 4
} ModuleState;

// 模块优先级
typedef enum {
    MOD_PRIO_LOW       = 0,
    MOD_PRIO_NORMAL    = 1,
    MOD_PRIO_HIGH      = 2,
    MOD_PRIO_REALTIME  = 3
} ModulePrio;

// 模块接口定义
typedef struct {
    const char*         name;           // 模块名
    int (*init)(void);                  // 初始化
    int (*run)(void);                   // 运行
    int (*stop)(void);                  // 停止
} ModuleInterface;

#define REGISTER_MODULE(name, init_fn, run_fn, stop_fn) \
    const ModuleInterface __module_##name \
        __attribute__((section(".modules"))) = { \
            #name, init_fn, run_fn, stop_fn \
        }

#endif