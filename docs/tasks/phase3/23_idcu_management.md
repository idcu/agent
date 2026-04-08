# 任务 3.23: idcu-management - 管理库

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-08  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的管理库，提供命令行接口、HTTP 管理 API、模块管理接口、配置管理接口、系统监控接口，满足 API 响应延迟 ≤ 100ms、支持 1000+ 并发管理请求的性能要求。

### 1.2 不做什么
- 不实现 Web UI 界面
- 不实现用户认证和授权（由上层应用负责）
- 不实现审计日志持久化
- 不实现分布式管理

### 1.3 输入
- 管理命令（status、module_list、module_start、module_stop、config_get、config_set、metrics_get、health_check）
- HTTP 请求数据
- 命令行参数

### 1.4 输出
- 管理响应（状态码、消息、数据）
- HTTP 响应（JSON 格式）
- 命令行输出

### 1.5 前置依赖
- idcu-common 库已实现
- idcu-http-server 库已实现
- idcu-metrics 库已实现
- phase3 前 22 个任务已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **CLI**: 命令行参数解析 + 标准输出
- **HTTP API**: RESTful API + JSON 格式
- **模块管理**: 调用 idcu-plugin 库
- **配置管理**: 调用 idcu-config 库
- **系统监控**: 调用 idcu-metrics 和 idcu-healthcheck 库

### 2.2 核心逻辑
```
1. 初始化管理库
   a. 初始化 HTTP 服务器（可选）
   b. 注册 API 路由
   c. 初始化命令行解析器

2. 处理管理请求
   a. 解析请求类型和参数
   b. 调用相应的底层库
   c. 格式化响应
   d. 返回结果

3. HTTP API 流程
   a. 接收 HTTP 请求
   b. 解析 URL 和请求体
   c. 路由到处理函数
   d. 执行管理操作
   e. 返回 JSON 响应

4. CLI 流程
   a. 解析命令行参数
   b. 执行管理操作
   c. 输出结果到标准输出
```

### 2.3 数据结构/接口
```c
typedef enum {
    IDCU_MGMT_CMD_STATUS,
    IDCU_MGMT_CMD_MODULE_LIST,
    IDCU_MGMT_CMD_MODULE_START,
    IDCU_MGMT_CMD_MODULE_STOP,
    IDCU_MGMT_CMD_CONFIG_GET,
    IDCU_MGMT_CMD_CONFIG_SET,
    IDCU_MGMT_CMD_METRICS_GET,
    IDCU_MGMT_CMD_HEALTH_CHECK
} idcu_MgmtCommand;

typedef struct {
    int code;
    char* message;
    char* data;
    size_t data_size;
} idcu_MgmtResponse;

int  idcu_mgmt_init(void);
void idcu_mgmt_destroy(void);
int  idcu_mgmt_execute(idcu_MgmtCommand cmd, const char* args, idcu_MgmtResponse* response);
int  idcu_mgmt_cli_start(int port);
int  idcu_mgmt_http_start(int port);
```

### 2.4 跨平台适配
- **CLI**: 使用标准 C 库，跨平台兼容
- **HTTP**: 使用 idcu-http-server 库封装的跨平台 API
- **路径**: 使用 idcu-common 库统一处理路径分隔符

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以获取系统状态
- [ ] 可以列出、启动、停止模块
- [ ] 可以获取和设置配置
- [ ] 可以获取指标数据
- [ ] 可以执行健康检查
- [ ] CLI 接口正常工作
- [ ] HTTP API 接口正常工作

### 3.2 性能验收
- API 响应延迟 ≤ 100ms
- 支持 1000+ 并发管理请求
- CLI 命令执行耗时 ≤ 500ms
- 内存占用 ≤ 5MB

### 3.3 异常验收
- [ ] 无效命令返回明确错误码
- [ ] 无效参数返回明确错误码
- [ ] HTTP 请求失败返回 4xx/5xx 状态码
- [ ] 多线程并发操作无数据竞争

---

## 4. 执行计划

### 4.1 工期
2 天/人

### 4.2 里程碑
- D1：完成接口定义、头文件、CMakeLists.txt、module.yaml、README.md
- D2：完成 CLI 和 HTTP API 实现、单元测试

### 4.3 人力
1 人（技能要求：C 语言 + HTTP 服务器）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名：idcu_mgmt_* 小写加下划线

### 5.2 测试要求
- 单元测试覆盖率 ≥ 70%
- 测试覆盖：所有管理命令、异常场景

### 5.3 部署指引
- 编译命令：`cmake -B build &amp;&amp; cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::management)`

---

## 6. 风险与应对

### 6.1 风险 1
描述：HTTP API 安全问题  
应对：提供认证和授权接口，由上层应用实现

### 6.2 风险 2
描述：管理命令执行时间过长  
应对：异步执行长耗时命令，提供状态查询

---

## 7. 详细实现步骤

### 7.1 创建目录结构

```bash
mkdir -p libs/idcu-management/include/idcu/management
mkdir -p libs/idcu-management/src/idcu/management
mkdir -p libs/idcu-management/tests
mkdir -p libs/idcu-management/examples
```

### 7.2 创建管理头文件 (management.h)

创建 `libs/idcu-management/include/idcu/management/management.h`：

```c
#ifndef IDCU_MANAGEMENT_MANAGEMENT_H
#define IDCU_MANAGEMENT_MANAGEMENT_H

#include "idcu/common/error_code.h"
#include &lt;stddef.h&gt;
#include &lt;stdint.h&gt;

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IDCU_MGMT_CMD_STATUS,
    IDCU_MGMT_CMD_MODULE_LIST,
    IDCU_MGMT_CMD_MODULE_START,
    IDCU_MGMT_CMD_MODULE_STOP,
    IDCU_MGMT_CMD_CONFIG_GET,
    IDCU_MGMT_CMD_CONFIG_SET,
    IDCU_MGMT_CMD_METRICS_GET,
    IDCU_MGMT_CMD_HEALTH_CHECK
} idcu_MgmtCommand;

typedef struct {
    int code;
    char* message;
    char* data;
    size_t data_size;
} idcu_MgmtResponse;

int  idcu_mgmt_init(void);
void idcu_mgmt_destroy(void);
int  idcu_mgmt_execute(idcu_MgmtCommand cmd, const char* args, idcu_MgmtResponse* response);
void idcu_mgmt_response_destroy(idcu_MgmtResponse* response);
int  idcu_mgmt_cli_start(int port);
void idcu_mgmt_cli_stop(void);
int  idcu_mgmt_http_start(int port);
void idcu_mgmt_http_stop(void);

#ifdef __cplusplus
}
#endif

#endif
```

### 7.3 创建 CMakeLists.txt

创建 `libs/idcu-management/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-management VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-management STATIC
    src/idcu/management/management.c
    src/idcu/management/cli.c
    src/idcu/management/http_api.c
)

target_include_directories(idcu-management PUBLIC
    $&lt;BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include&gt;
    $&lt;INSTALL_INTERFACE:include&gt;
)

target_link_libraries(idcu-management PUBLIC
    idcu::common
    idcu::http-server
    idcu::metrics
)

add_library(idcu::management ALIAS idcu-management)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 7.4 创建模块配置文件 (module.yaml)

创建 `libs/idcu-management/module.yaml`：

```yaml
name: idcu-management
version: 1.0.0
description: Management library with CLI and HTTP API
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-http-server
  - idcu-metrics

build:
  type: cmake
  targets:
    - idcu-management

headers:
  - idcu/management/management.h

features:
  - cli: Command line interface
  - http_api: HTTP management API
  - module_management: Module management interface
  - config_management: Configuration management interface
  - monitoring: System monitoring interface

testing:
  enabled: true
  framework: internal
```

---

## 8. 验证检查清单

- [ ] 头文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 代码可以成功编译
- [ ] 基本管理功能正常
- [ ] 单元测试通过

---

## 9. Git 提交

```bash
git add libs/idcu-management/
git commit -m "feat: add idcu-management library

- Add management CLI interface
- Add HTTP management API
- Add module and config management
- Add CMake build configuration
- Add module.yaml metadata"
```
