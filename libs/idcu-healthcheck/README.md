# idcu-healthcheck

IDCU 项目的独立健康检查库，提供模块健康状态监控和管理功能。

## 特性

- 跨平台支持（Windows、Linux、macOS）
- 支持多种健康状态：UNKNOWN、HEALTHY、WARNING、CRITICAL、DEAD
- 模块注册和心跳更新
- 错误计数和重启计数跟踪
- 可配置的超时和阈值
- 状态变更回调通知
- 线程安全的操作
- 健康摘要统计

## 快速开始

### 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 安装

```bash
cmake --install .
```

### 在其他项目中使用

```cmake
find_package(idcu-common REQUIRED)
find_package(idcu-log REQUIRED)
find_package(idcu-healthcheck REQUIRED)
add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE idcu::healthcheck)
```

## API 文档

### 初始化和清理

```c
#include <idcu/healthcheck/healthcheck.h>

// 初始化健康监控器
int idcu_health_monitor_init(idcu_HealthMonitor* monitor);

// 销毁健康监控器
void idcu_health_monitor_destroy(idcu_HealthMonitor* monitor);
```

### 模块管理

```c
// 注册模块
int idcu_health_register_module(idcu_HealthMonitor* monitor, uint32_t module_id);

// 注销模块
int idcu_health_unregister_module(idcu_HealthMonitor* monitor, uint32_t module_id);

// 更新模块心跳
int idcu_health_update_heartbeat(idcu_HealthMonitor* monitor, uint32_t module_id);
```

### 状态报告

```c
// 报告错误
int idcu_health_report_error(idcu_HealthMonitor* monitor, uint32_t module_id);

// 报告重启
int idcu_health_report_restart(idcu_HealthMonitor* monitor, uint32_t module_id);

// 重置错误计数
int idcu_health_reset_error_count(idcu_HealthMonitor* monitor, uint32_t module_id);
```

### 状态查询

```c
// 获取模块健康状态
idcu_HealthStatus idcu_health_get_status(idcu_HealthMonitor* monitor, uint32_t module_id);

// 获取模块详细信息
const idcu_ModuleHealth* idcu_health_get_info(idcu_HealthMonitor* monitor, uint32_t module_id);

// 获取整体健康状态
idcu_HealthStatus idcu_health_get_overall_status(idcu_HealthMonitor* monitor);

// 获取健康摘要
void idcu_health_get_summary(idcu_HealthMonitor* monitor, idcu_HealthSummary* summary);

// 获取模块数量
uint32_t idcu_health_get_module_count(idcu_HealthMonitor* monitor);

// 获取所有模块信息
const idcu_ModuleHealth* idcu_health_get_all_modules(idcu_HealthMonitor* monitor, uint32_t* count);
```

### 检查和配置

```c
// 检查所有模块健康状态
int idcu_health_check_all(idcu_HealthMonitor* monitor);

// 设置状态变更回调
void idcu_health_set_callback(idcu_HealthMonitor* monitor, idcu_HealthCallback cb, void* user_data);

// 设置阈值
void idcu_health_set_thresholds(idcu_HealthMonitor* monitor, uint64_t timeout_ms, uint64_t warning, uint64_t critical);

// 获取系统运行时间
uint64_t idcu_health_get_uptime_ms(void);

// 状态转换为字符串
const char* idcu_health_status_to_string(idcu_HealthStatus status);
```

### 健康状态类型

```c
typedef enum {
    IDCU_HEALTH_UNKNOWN = 0,
    IDCU_HEALTH_HEALTHY,
    IDCU_HEALTH_WARNING,
    IDCU_HEALTH_CRITICAL,
    IDCU_HEALTH_DEAD
} idcu_HealthStatus;
```

## 测试

```bash
cd build
ctest
```

## 示例

库提供了以下示例代码：

- [简单健康监控示例](examples/example_healthcheck_basic.c) - 演示健康监控的基本使用
- [状态回调示例](examples/example_healthcheck_callback.c) - 演示如何使用状态变更回调

### 编译和运行示例

```bash
cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build .

# 运行简单示例
./example_healthcheck_basic

# 运行回调示例
./example_healthcheck_callback
```

## 依赖

- idcu-common - IDCU 基础通用组件库
- idcu-log - IDCU 日志库

## 许可证

详见项目根目录的 LICENSE 文件。
