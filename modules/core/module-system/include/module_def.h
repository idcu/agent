#ifndef IDCU_MODULE_MODULE_DEF_H
#define IDCU_MODULE_MODULE_DEF_H

#include <stdint.h>
#include "config.h"
#include "error_code.h"
#include "log.h"

#define IDCU_MAX_MODULE_DEPENDENCIES 16
#define IDCU_MODULE_NAME_MAX 64

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

// 版本信息结构体
typedef struct {
    uint16_t major;  // 主版本号 (不兼容修改)
    uint16_t minor;  // 次版本号 (向后兼容新增功能)
    uint16_t patch;  // 修订号 (向后兼容 bug 修复)
    const char* pre_release;  // 预发布版本标签 (如 "alpha", "beta.1")
} idcu_ModuleVersion;

// 模块依赖关系 (包含版本要求)
typedef struct {
    char dependency_name[IDCU_MODULE_NAME_MAX];
    idcu_ModuleVersion min_version;  // 最低要求版本
    idcu_ModuleVersion max_version;  // 最高兼容版本 (可选)
    int has_max_version;              // 是否设置了最高版本
} idcu_ModuleDependency;

// 模块接口定义
typedef struct {
    const char*         name;           // 模块名
    idcu_ModuleVersion  version;        // 模块版本
    const idcu_ModuleDependency* dependencies; // 依赖的模块列表
    int                 dependency_count; // 依赖模块数量
    int (*init)(void);                  // 初始化
    int (*run)(void);                   // 运行
    int (*stop)(void);                  // 停止
} idcu_ModuleInterface;

// 版本初始化辅助宏
#define IDCU_MODULE_VERSION(maj, min, pat) { (maj), (min), (pat), NULL }
#define IDCU_MODULE_VERSION_PRE(maj, min, pat, pre) { (maj), (min), (pat), (pre) }

// 跨平台 section 宏定义
#ifdef __GNUC__
#define IDCU_REGISTER_MODULE(name, ver, init_fn, run_fn, stop_fn) \
    const idcu_ModuleInterface __idcu_module_##name \
        __attribute__((section(".modules"), used)) = { \
            #name, ver, NULL, 0, init_fn, run_fn, stop_fn \
        }
#define IDCU_REGISTER_MODULE_WITH_DEPS(name, ver, deps, dep_count, init_fn, run_fn, stop_fn) \
    const idcu_ModuleInterface __idcu_module_##name \
        __attribute__((section(".modules"), used)) = { \
            #name, ver, deps, dep_count, init_fn, run_fn, stop_fn \
        }
#elif defined(_MSC_VER)
#define IDCU_REGISTER_MODULE(name, ver, init_fn, run_fn, stop_fn) \
    __pragma(section(".modules", read)) \
    __declspec(allocate(".modules")) \
    const idcu_ModuleInterface __idcu_module_##name = { \
        #name, ver, NULL, 0, init_fn, run_fn, stop_fn \
    }
#define IDCU_REGISTER_MODULE_WITH_DEPS(name, ver, deps, dep_count, init_fn, run_fn, stop_fn) \
    __pragma(section(".modules", read)) \
    __declspec(allocate(".modules")) \
    const idcu_ModuleInterface __idcu_module_##name = { \
        #name, ver, deps, dep_count, init_fn, run_fn, stop_fn \
    }
#else
#define IDCU_REGISTER_MODULE(name, ver, init_fn, run_fn, stop_fn) \
    const idcu_ModuleInterface __idcu_module_##name = { \
        #name, ver, NULL, 0, init_fn, run_fn, stop_fn \
    }
#define IDCU_REGISTER_MODULE_WITH_DEPS(name, ver, deps, dep_count, init_fn, run_fn, stop_fn) \
    const idcu_ModuleInterface __idcu_module_##name = { \
        #name, ver, deps, dep_count, init_fn, run_fn, stop_fn \
    }
#endif

#endif // IDCU_MODULE_MODULE_DEF_H
