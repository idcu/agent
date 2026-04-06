#ifndef IDCU_MANAGEMENT_MANAGEMENT_H
#define IDCU_MANAGEMENT_MANAGEMENT_H

#include "idcu/common/error_code.h"
#include "idcu/module_system/module_system.h"
#include "idcu/coroutine/coroutine.h"
#include "idcu/metrics/metrics.h"
#include "idcu/config/config.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Agent 状态信息
typedef struct {
    char version[32];
    char uptime_str[64];
    uint64_t uptime_seconds;
    int running;
} idcu_AgentStatus;

// 模块详细信息
typedef struct {
    idcu_ModuleInfo info;
    int loaded;
    int running;
} idcu_ModuleDetail;

// 协程统计信息
typedef struct {
    uint32_t total_count;
    uint32_t ready_count;
    uint32_t running_count;
    uint32_t suspended_count;
    uint32_t finished_count;
} idcu_CoroutineStats;

// 初始化运维管理模块
int idcu_management_init(void);

// 关闭运维管理模块
void idcu_management_shutdown(void);

// 获取 Agent 状态
int idcu_management_get_agent_status(idcu_AgentStatus* status);

// 获取已加载模块列表
int idcu_management_get_modules(idcu_ModuleDetail* modules, size_t max_modules, size_t* actual_count);

// 获取模块详情
int idcu_management_get_module_detail(const char* name, idcu_ModuleDetail* detail);

// 加载模块
int idcu_management_load_module(const char* name);

// 卸载模块
int idcu_management_unload_module(const char* name);

// 启动模块
int idcu_management_start_module(const char* name);

// 停止模块
int idcu_management_stop_module(const char* name);

// 获取协程统计
int idcu_management_get_coroutine_stats(idcu_CoroutineStats* stats);

// 获取指标数据
int idcu_management_get_metrics(char* buffer, size_t buffer_size);

// 获取配置信息
int idcu_management_get_config(char* buffer, size_t buffer_size);

// 触发健康检查
int idcu_management_trigger_health_check(void);

#ifdef __cplusplus
}
#endif

#endif // IDCU_MANAGEMENT_MANAGEMENT_H
