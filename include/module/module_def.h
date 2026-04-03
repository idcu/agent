#ifndef IDCU_MODULE_MODULE_DEF_H
#define IDCU_MODULE_MODULE_DEF_H

#include <stdint.h>
#include "common/config.h"
#include "common/error_code.h"
#include "utils/log.h"

// 模块状态
typedef enum {
    IDCU_MOD_STATE_UNINIT   = 0,
    IDCU_MOD_STATE_INITED   = 1,
    IDCU_MOD_STATE_RUNNING  = 2,
    IDCU_MOD_STATE_STOPPED  = 3,
    IDCU_MOD_STATE_ERROR    = 4
} idcu_ModuleState;

// 模块优先级
typedef enum {
    IDCU_MOD_PRIO_LOW       = 0,
    IDCU_MOD_PRIO_NORMAL    = 1,
    IDCU_MOD_PRIO_HIGH      = 2,
    IDCU_MOD_PRIO_REALTIME  = 3
} idcu_ModulePrio;

// 模块接口定义
typedef struct {
    const char*         name;           // 模块名
    int (*init)(void);                  // 初始化
    int (*run)(void);                   // 运行
    int (*stop)(void);                  // 停止
} idcu_ModuleInterface;

#define IDCU_REGISTER_MODULE(name, init_fn, run_fn, stop_fn) \
    const idcu_ModuleInterface __idcu_module_##name \
        __attribute__((section(".modules"))) = { \
            #name, init_fn, run_fn, stop_fn \
        }

#endif // IDCU_MODULE_MODULE_DEF_H
