# 任务文档索引

本文档索引了所有已拆解完成的任务文档。

## 阶段 1: 项目初始化和基础构建

| 任务 | 状态 | 文件 |
|-----|------|------|
| 1.0 阶段 1 概述 | ✅ 完成 | [phase1/00_phase1_overview.md](phase1/00_phase1_overview.md) |
| 1.1 创建项目目录结构 | ✅ 完成 | [phase1/01_create_project_structure.md](phase1/01_create_project_structure.md) |
| 1.2 编写主程序入口 | ✅ 完成 | [phase1/02_write_main_entry.md](phase1/02_write_main_entry.md) |
| 1.3 配置 CMake 构建系统 | ✅ 完成 | [phase1/03_configure_cmake.md](phase1/03_configure_cmake.md) |
| 1.4 验证项目可以编译 | ✅ 完成 | [phase1/04_verify_build.md](phase1/04_verify_build.md) |
| 1.5 添加代码质量工具 | ✅ 完成 | [phase1/05_add_code_quality_tools.md](phase1/05_add_code_quality_tools.md) |
| 1.6 搭建测试框架 | ✅ 完成 | [phase1/06_setup_test_framework.md](phase1/06_setup_test_framework.md) |
| 1.7 初始化 idcu-module-build | ✅ 完成 | [phase1/07_init_module_build.md](phase1/07_init_module_build.md) |

## 阶段 2: 核心基础设施构建

| 任务 | 状态 | 文件 |
|-----|------|------|
| 2.0 阶段 2 概述 | ✅ 完成 | [phase2/00_phase2_overview.md](phase2/00_phase2_overview.md) |
| 2.1 创建通用基础库 (idcu-common) | ✅ 完成 | [phase2/01_create_idcu_common.md](phase2/01_create_idcu_common.md) |
| 2.2 完善 idcu-module-build | ✅ 完成 | [phase2/02_improve_module_build.md](phase2/02_improve_module_build.md) |
| 2.3 模块系统 | ✅ 完成 | [phase2/03_module_system.md](phase2/03_module_system.md) |
| 2.4 协程调度器 | ✅ 完成 | [phase2/04_coroutine_scheduler.md](phase2/04_coroutine_scheduler.md) |
| 2.5 消息总线 | ✅ 完成 | [phase2/05_message_bus.md](phase2/05_message_bus.md) |
| 2.6 微内核核心 | ✅ 完成 | [phase2/06_micro_kernel.md](phase2/06_micro_kernel.md) |
| 2.7 SDK 基础 | ✅ 完成 | [phase2/07_sdk_base.md](phase2/07_sdk_base.md) |
| 2.8 SDK 基础 (备用) | ✅ 完成 | [phase2/07_sdk_basic.md](phase2/07_sdk_basic.md) |

## 阶段 3: 独立库开发与完善

### 基础工具层

| 任务 | 状态 | 文件 |
|-----|------|------|
| 3.1 idcu-log - 日志系统 | ✅ 完成 | [phase3/01_idcu_log.md](phase3/01_idcu_log.md) |
| 3.2 idcu-json - JSON 解析 | ✅ 完成 | [phase3/02_idcu_json.md](phase3/02_idcu_json.md) |
| 3.3 idcu-yaml - YAML 解析 | ✅ 完成 | [phase3/03_idcu_yaml.md](phase3/03_idcu_yaml.md) |
| 3.4 idcu-memory - 内存池 | ✅ 完成 | [phase3/04_idcu_memory.md](phase3/04_idcu_memory.md) |
| 3.5 idcu-utils - 工具库 | ✅ 完成 | [phase3/05_idcu_utils.md](phase3/05_idcu_utils.md) |

### 配置与存储层

| 任务 | 状态 | 文件 |
|-----|------|------|
| 3.6 idcu-config - 配置管理 | ✅ 完成 | [phase3/06_idcu_config.md](phase3/06_idcu_config.md) |
| 3.7 idcu-storage - 持久化存储 | ✅ 完成 | [phase3/07_idcu_storage.md](phase3/07_idcu_storage.md) |
| 3.8 idcu-cache - 内存缓存 | ✅ 完成 | [phase3/08_idcu_cache.md](phase3/08_idcu_cache.md) |

### 网络与通信层

| 任务 | 状态 | 文件 |
|-----|------|------|
| 3.9 idcu-network - 网络层 | ✅ 完成 | [phase3/09_idcu_network.md](phase3/09_idcu_network.md) |
| 3.10 idcu-conn-pool - 连接池 | ✅ 完成 | [phase3/10_idcu_conn_pool.md](phase3/10_idcu_conn_pool.md) |
| 3.11 idcu-http-server - HTTP 服务器 | ✅ 完成 | [phase3/11_idcu_http_server.md](phase3/11_idcu_http_server.md) |
| 3.12 idcu-http-client - HTTP 客户端 | ✅ 完成 | [phase3/12_idcu_http_client.md](phase3/12_idcu_http_client.md) |
| 3.13 idcu-msgbus - 消息总线库 | ✅ 完成 | [phase3/13_idcu_msgbus.md](phase3/13_idcu_msgbus.md) |
| 3.14 idcu-coroutine - 协程库 | ⏳ 待创建 | phase3/14_idcu_coroutine.md |

### 监控与服务层

| 任务 | 状态 | 文件 |
|-----|------|------|
| 3.15 idcu-metrics - 指标收集 | ✅ 完成 | [phase3/15_idcu_metrics.md](phase3/15_idcu_metrics.md) |
| 3.16 idcu-healthcheck - 健康检查 | ✅ 完成 | [phase3/16_idcu_healthcheck.md](phase3/16_idcu_healthcheck.md) |
| 3.17 idcu-alert - 告警管理 | ✅ 完成 | [phase3/17_idcu_alert.md](phase3/17_idcu_alert.md) |
| 3.18 idcu-watchdog - 看门狗 | ✅ 完成 | [phase3/18_idcu_watchdog.md](phase3/18_idcu_watchdog.md) |
| 3.19 idcu-discovery - 节点发现 | ✅ 完成 | [phase3/19_idcu_discovery.md](phase3/19_idcu_discovery.md) |

### 安全与插件层

| 任务 | 状态 | 文件 |
|-----|------|------|
| 3.20 idcu-sandbox - 沙箱安全 | ✅ 完成 | [phase3/20_idcu_sandbox.md](phase3/20_idcu_sandbox.md) |
| 3.21 idcu-permission - 权限管理 | ✅ 完成 | [phase3/21_idcu_permission.md](phase3/21_idcu_permission.md) |
| 3.22 idcu-plugin - 插件系统 | ✅ 完成 | [phase3/22_idcu_plugin.md](phase3/22_idcu_plugin.md) |
| 3.23 idcu-management - 管理 CLI | ⏳ 待创建 | phase3/23_idcu_management.md |

### 高级功能层

| 任务 | 状态 | 文件 |
|-----|------|------|
| 3.24 idcu-distributed - 分布式支持 | ⏳ 待创建 | phase3/24_idcu_distributed.md |
| 3.25 idcu-scheduler - 任务调度 | ✅ 完成 | [phase3/25_idcu_scheduler.md](phase3/25_idcu_scheduler.md) |
| 3.26 idcu-device-collector - 设备采集 | ⏳ 待创建 | phase3/26_idcu_device_collector.md |
| 3.27 idcu-server-monitor - 服务器监控 | ⏳ 待创建 | phase3/27_idcu_server_monitor.md |
| 3.28 idcu-module-isolation - 模块隔离 | ⏳ 待创建 | phase3/28_idcu_module_isolation.md |
| 3.29 idcu-module-verifier - 模块验证 | ⏳ 待创建 | phase3/29_idcu_module_verifier.md |

## 阶段 4: 模块系统完善

| 任务 | 状态 | 文件 |
|-----|------|------|
| 4.0 阶段 4 概述 | ✅ 完成 | [phase4/00_phase4_overview.md](phase4/00_phase4_overview.md) |
| 4.1 log-integration - 日志集成 | ✅ 完成 | [phase4/01_log_integration.md](phase4/01_log_integration.md) |
| 4.2 config-integration - 配置集成 | ✅ 完成 | [phase4/02_config_integration.md](phase4/02_config_integration.md) |
| 4.3 json-integration - JSON 集成 | ✅ 完成 | [phase4/03_json_integration.md](phase4/03_json_integration.md) |
| 4.4 yaml-integration - YAML 集成 | ✅ 完成 | [phase4/04_yaml_integration.md](phase4/04_yaml_integration.md) |
| 4.5 network-integration - 网络集成 | ✅ 完成 | [phase4/05_network_integration.md](phase4/05_network_integration.md) |
| 4.6 metrics-integration - 指标集成 | ✅ 完成 | [phase4/06_metrics_integration.md](phase4/06_metrics_integration.md) |
| 4.7 basic-libs - 基础库统一集成 | ✅ 完成 | [phase4/07_basic_libs.md](phase4/07_basic_libs.md) |
| 4.8 SDK 完善 | ✅ 完成 | [phase4/08_sdk_complete.md](phase4/08_sdk_complete.md) |
| 4.9 REST API 模块 | ✅ 完成 | [phase4/01_rest_api.md](phase4/01_rest_api.md) |

## 阶段 5: 业务模块开发

| 任务 | 状态 | 文件 |
|-----|------|------|
| 5.0 阶段 5 概述 | ✅ 完成 | [phase5/00_phase5_overview.md](phase5/00_phase5_overview.md) |
| 5.1 core-module - 核心基础模块 | ✅ 完成 | [phase5/01_core_module.md](phase5/01_core_module.md) |
| 5.2 log-module - 日志业务模块 | ✅ 完成 | [phase5/02_log_module.md](phase5/02_log_module.md) |
| 5.3 config-module - 配置业务模块 | ✅ 完成 | [phase5/03_config_module.md](phase5/03_config_module.md) |
| 5.4 heartbeat - 心跳模块 | ✅ 完成 | [phase5/04_heartbeat.md](phase5/04_heartbeat.md) |
| 5.5 metrics-module - 指标模块 | ✅ 完成 | [phase5/05_metrics_module.md](phase5/05_metrics_module.md) |
| 5.6 healthcheck-module - 健康检查业务模块 | ✅ 完成 | [phase5/06_healthcheck_module.md](phase5/06_healthcheck_module.md) |
| 5.7 alert-module - 告警业务模块 | ✅ 完成 | [phase5/07_alert_module.md](phase5/07_alert_module.md) |
| 5.8 collect-module - 数据采集模块 | ✅ 完成 | [phase5/08_collect_module.md](phase5/08_collect_module.md) |
| 5.9 cache-module - 缓存业务模块 | ✅ 完成 | [phase5/09_cache_module.md](phase5/09_cache_module.md) |
| 5.10 storage-module - 存储业务模块 | ✅ 完成 | [phase5/10_storage_module.md](phase5/10_storage_module.md) |
| 5.11 security-module - 安全业务模块 | ✅ 完成 | [phase5/11_security_module.md](phase5/11_security_module.md) |
| 5.12 http-client-module - HTTP 客户端模块 | ✅ 完成 | [phase5/12_http_client_module.md](phase5/12_http_client_module.md) |
| 5.13 http-management - HTTP 管理模块 | ✅ 完成 | [phase5/13_http_management_module.md](phase5/13_http_management_module.md) |
| 5.14 task-queue - 任务队列模块 | ✅ 完成 | [phase5/05_task_queue.md](phase5/05_task_queue.md) |

## 参考资料

| 资料 | 状态 | 文件 |
|-----|------|------|
| 调试技巧 | ✅ 完成 | [reference/debugging_tips.md](reference/debugging_tips.md) |
| 最佳实践与经验教训 | ✅ 完成 | [reference/best_practices.md](reference/best_practices.md) |
| 任务完成标准流程 | ✅ 完成 | [reference/task_completion_flow.md](reference/task_completion_flow.md) |
| 常见问题 FAQ | ✅ 完成 | [reference/faq.md](reference/faq.md) |
| 技术决策记录 | ✅ 完成 | [reference/technical_decisions.md](reference/technical_decisions.md) |

## 开发指南

### 任务文档结构

每个任务文档都包含以下部分：

1. **目标** - 明确该任务要实现的功能
2. **详细步骤** - 具体的实现步骤和代码
3. **验证检查清单** - 验证任务完成的检查项
4. **Git 提交** - 建议的 Git 提交信息
5. **常见问题排查** - 可能遇到的问题和解决方案

### 模块配置文件

所有模块都使用 YAML 格式的 `module.yaml` 配置文件，包含：
- 名称、版本、描述
- 依赖关系
- 构建信息
- 功能特性列表
- 测试配置

### 代码标准

- 使用 C11 标准
- 遵循现有代码风格
- 使用统一的错误码 (idcu_ErrorCode)
- 确保线程安全
- 提供完整的文档注释

## 注意事项

1. **不要迁移现有代码** - 直接从文档开始新开发
2. **将现有代码精华写入文档** - 参考现有代码的好的实现
3. **使用 YAML 配置** - 模块配置文件默认使用 YAML 格式
4. **注意代码质量** - 确保代码安全、可靠、可维护
5. **只更新文档** - 当前任务仅更新文档，不处理任何代码
