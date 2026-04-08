# 任务 3.20: idcu-sandbox - 沙箱安全库

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的沙箱安全库，支持资源限制（CPU、内存、磁盘）、系统调用过滤、文件系统访问控制、网络访问控制、进程隔离和安全审计，满足单沙箱创建时间 ≤ 10ms、规则检查延迟 ≤ 1ms 的性能要求。

### 1.2 不做什么
- 不实现内核级沙箱（如 seccomp-bpf）
- 不实现虚拟内存隔离
- 不实现容器化隔离
- 不实现完整的系统调用拦截（本阶段仅实现规则检查框架）

### 1.3 输入
- 沙箱配置：资源限制、访问规则、网络策略
- 访问请求：路径访问、网络连接、系统调用
- 运行状态：CPU 时间、内存使用、磁盘空间

### 1.4 输出
- 沙箱 ID：唯一标识
- 访问决策：允许/拒绝/审计
- 违规记录：违规类型、时间戳、详情
- 统计数据：违规计数、资源使用

### 1.5 前置依赖
- idcu-common 基础库已可用
- idcu-json 库已可用（用于违规记录 JSON 序列化）
- idcu-log 库已可用（用于审计日志）
- phase2 已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **资源限制**: 软件层面监控（不依赖 cgroups）
- **规则引擎**: 基于模式匹配的规则检查
- **权限模型**: 白名单 + 黑名单混合模式
- **审计日志**: 内存缓存 + idcu-log 输出
- **线程安全**: 使用 idcu_mutex 保护共享状态

### 2.2 核心逻辑
```
1. 初始化沙箱管理器
2. 创建沙箱配置：
   a. 设置资源限制
   b. 添加文件系统访问规则
   c. 添加网络访问规则
   d. 添加系统调用规则
3. 创建沙箱实例
4. 启用沙箱
5. 运行时访问检查：
   a. 检查路径访问
   b. 检查网络访问
   c. 检查系统调用
   d. 检查资源使用
   e. 记录违规（如启用审计）
6. 销毁沙箱
```

### 2.3 数据结构/接口
```c
// 权限类型
typedef enum {
    IDCU_SANDBOX_PERM_ALLOW = 0,
    IDCU_SANDBOX_PERM_DENY,
    IDCU_SANDBOX_PERM_AUDIT
} idcu_SandboxPermission;

// 规则类型
typedef enum {
    IDCU_SANDBOX_RULE_PATH = 0,
    IDCU_SANDBOX_RULE_SYSCALL,
    IDCU_SANDBOX_RULE_NETWORK,
    IDCU_SANDBOX_RULE_RESOURCE
} idcu_SandboxRuleType;

// 资源限制
typedef struct {
    uint64_t max_cpu_time_ms;
    uint64_t max_memory_bytes;
    uint64_t max_disk_bytes;
    uint64_t max_open_files;
    uint64_t max_processes;
    uint64_t max_network_connections;
} idcu_SandboxResourceLimits;

// 沙箱配置
typedef struct {
    idcu_SandboxId id;
    char name[128];
    idcu_SandboxResourceLimits resource_limits;
    idcu_SandboxFileSystemConfig fs_config;
    idcu_SandboxNetworkConfig network_config;
    idcu_SandboxSyscallConfig syscall_config;
    idcu_Vector rules;
    int enabled;
    int audit_enabled;
} idcu_SandboxConfig;

// 核心 API
int  idcu_sandbox_manager_init(idcu_SandboxManager* manager);
void idcu_sandbox_manager_destroy(idcu_SandboxManager* manager);
idcu_SandboxId idcu_sandbox_manager_create(idcu_SandboxManager* manager, const idcu_SandboxConfig* config);
int  idcu_sandbox_manager_enable(idcu_SandboxManager* manager, idcu_SandboxId sandbox_id);
int  idcu_sandbox_check_path_access(idcu_Sandbox* sandbox, const char* path, int mode);
int  idcu_sandbox_check_network_access(idcu_Sandbox* sandbox, const char* address, uint16_t port);
int  idcu_sandbox_log_violation(idcu_Sandbox* sandbox, idcu_SandboxRuleType type, const char* violation, const char* details, const char* process_name);
```

### 2.4 跨平台适配
- **Windows**: 使用 `GetProcessTimes` 获取 CPU 时间，`GetProcessMemoryInfo` 获取内存使用
- **Linux**: 使用 `clock_gettime` 获取 CPU 时间，`/proc/[pid]/status` 获取内存使用
- **路径匹配**: Windows 使用反斜杠，Linux 使用正斜杠，内部统一转换

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以创建和销毁沙箱
- [ ] 可以配置资源限制（CPU、内存、磁盘）
- [ ] 可以配置文件系统访问规则（允许/拒绝/只读路径）
- [ ] 可以配置网络访问规则（地址、端口）
- [ ] 可以配置系统调用规则
- [ ] 访问检查正常工作（允许/拒绝）
- [ ] 违规记录正常工作（记录违规详情）
- [ ] 审计日志正常输出

### 3.2 性能验收
- 单沙箱创建时间 ≤ 10ms
- 单沙箱销毁时间 ≤ 5ms
- 规则检查延迟 ≤ 1ms/次
- 支持同时管理 ≥ 100 个沙箱
- 内存占用 ≤ 256KB/沙箱
- QPS ≥ 10,000 次访问检查/秒

### 3.3 异常验收
- [ ] 传入 NULL 参数时安全处理
- [ ] 配置无效规则时返回明确错误码
- [ ] 资源超限检测正常工作
- [ ] 并发访问沙箱管理器不崩溃
- [ ] 内存泄漏检测通过（Valgrind/AddressSanitizer）

---

## 4. 执行计划

### 4.1 工期
3 小时/人

### 4.2 里程碑
- D1-00: 完成头文件定义（30 分钟）
- D1-30: 完成核心数据结构和管理器（1 小时）
- D1-90: 完成访问检查和违规记录（1 小时）
- D2-30: 完成单元测试（30 分钟）

### 4.3 人力
1 人（技能要求：C 语言 + 安全编程）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，结构体前缀 idcu_
- 所有公共 API 有 Doxygen 风格注释

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 测试用例覆盖：沙箱创建/销毁、各种访问规则、违规记录、并发场景
- 性能测试验证创建时间和检查延迟

### 5.3 部署指引
- 编译命令：`cmake -B build && cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::sandbox)`

---

## 6. 风险与应对

### 6.1 风险1
描述：规则匹配性能不达标  
应对：使用前缀树或哈希表优化路径匹配，规则按类型分区管理

### 6.2 风险2
描述：资源监控不准确  
应对：分别实现 Windows 和 Linux 的原生资源监控，定期校准

---

## 7. 详细实现步骤

### 1. 创建目录结构
```bash
mkdir -p libs/idcu-sandbox/include/idcu/sandbox
mkdir -p libs/idcu-sandbox/src/idcu/sandbox
mkdir -p libs/idcu-sandbox/tests
mkdir -p libs/idcu-sandbox/examples
```

### 2. 创建沙箱头文件 (sandbox.h)
定义权限类型、规则类型、资源限制、配置结构体、沙箱实例和管理器，以及核心 API。

### 3. 创建 CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-sandbox VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-sandbox STATIC
    src/idcu/sandbox/sandbox.c
)

target_include_directories(idcu-sandbox PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-sandbox PRIVATE
    idcu::common
    idcu::json
    idcu::log
)

add_library(idcu::sandbox ALIAS idcu-sandbox)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)
```yaml
name: idcu-sandbox
version: 1.0.0
description: Sandbox security library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-json
  - idcu-log

build:
  type: cmake
  targets:
    - idcu-sandbox

headers:
  - idcu/sandbox/sandbox.h

features:
  - resource_limits: Resource limits (CPU, memory, disk)
  - syscall_filter: System call filtering
  - fs_access: File system access control
  - network_access: Network access control
  - process_isolation: Process isolation
  - audit: Security audit logging
  - rules: Custom security rules

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md
包含功能特性、快速开始指南、权限类型说明、规则类型说明和 API 文档链接。

---

## 8. 验证检查清单

- [ ] 沙箱头文件已创建
- [ ] 沙箱实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以正常编译通过
- [ ] 单元测试通过率 100%
- [ ] 性能测试达标（创建时间 ≤ 10ms，检查延迟 ≤ 1ms）
- [ ] 跨平台测试通过（Windows + Linux）
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-sandbox/
git commit -m "feat: add idcu-sandbox library

- Add resource limits (CPU, memory, disk)
- Add system call filtering framework
- Add file system access control
- Add network access control
- Add process isolation support
- Add security audit logging
- Add custom security rules
- Add CMake build configuration
- Add module.yaml metadata"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 合法访问被阻止 | 规则过于严格 | 调整规则配置，检查路径模式匹配 |
| 违规未记录 | 审计未启用 | 调用 idcu_sandbox_enable_audit 启用审计 |
| 系统调用过滤不工作 | 平台不支持或未实现 | 检查平台兼容性，本阶段可能仅实现框架 |
| 资源限制不生效 | 未实现内核级限制 | 本阶段仅实现监控，需依赖外部机制 |
| 性能不达标 | 规则数量过多 | 优化规则匹配算法，减少规则数量 |
