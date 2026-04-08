# 任务 3.28: idcu-module-isolation - 模块隔离

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-09  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的模块隔离库，支持模块内存隔离、模块权限控制、模块资源限制、模块故障隔离、模块间通信安全、模块沙箱执行，满足模块启动时间 ≤ 500ms、故障恢复时间 ≤ 1s、支持 100+ 隔离模块的性能要求。

### 1.2 不做什么
- 不实现虚拟机级别的完全隔离
- 不实现内核级别的隔离（如 cgroups）
- 不实现模块持久化状态
- 不实现跨进程隔离

### 1.3 输入
- 模块路径
- 隔离级别配置
- 资源限制配置（内存、CPU、文件描述符等）
- 权限配置（文件访问、网络访问、模块调用等）
- 沙箱配置

### 1.4 输出
- 隔离模块 ID
- 模块状态（运行、停止、崩溃）
- 资源使用情况
- 告警和崩溃回调
- 返回码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 库已实现
- idcu-sandbox 库已实现
- idcu-permission 库已实现
- idcu-log 库已实现
- phase3 前 27 个任务已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **隔离级别**: 5 级（NONE、LIGHT、MEDIUM、STRICT、FULL）
- **资源限制**: 内存、CPU、磁盘、网络、文件描述符
- **权限控制**: 基于位掩码的权限系统
- **沙箱执行**: 使用 idcu-sandbox 库
- **故障检测**: 看门狗 + 心跳机制

### 2.2 核心逻辑
```
隔离器工作流程：
1. 初始化隔离器，加载默认配置
2. 加载模块，配置隔离参数
3. 初始化沙箱环境
4. 启动模块
5. 监控模块运行状态
6. 限制资源使用
7. 检查权限访问
8. 处理模块崩溃
9. 支持模块重启

模块加载流程：
1. 验证模块文件
2. 分配隔离资源
3. 设置权限规则
4. 初始化沙箱
5. 加载模块到沙箱
6. 调用模块初始化函数
7. 启动监控
```

### 2.3 数据结构/接口
```c
typedef uint64_t idcu_IsolatedModuleId;

typedef enum {
    IDCU_ISOLATION_LEVEL_NONE = 0,
    IDCU_ISOLATION_LEVEL_LIGHT,
    IDCU_ISOLATION_LEVEL_MEDIUM,
    IDCU_ISOLATION_LEVEL_STRICT,
    IDCU_ISOLATION_LEVEL_FULL
} idcu_IsolationLevel;

typedef enum {
    IDCU_RESOURCE_CPU = 0,
    IDCU_RESOURCE_MEMORY,
    IDCU_RESOURCE_DISK,
    IDCU_RESOURCE_NETWORK,
    IDCU_RESOURCE_FILE_DESCRIPTORS
} idcu_ResourceType;

typedef struct {
    idcu_IsolatedModuleId module_id;
    char module_name[128];
    idcu_IsolationLevel isolation_level;
    idcu_ResourceConstraints resource_limits;
    uint32_t allowed_permissions;
    int running;
    int crashed;
} idcu_IsolatedModule;

int  idcu_module_isolator_init(idcu_ModuleIsolator* isolator, const idcu_ModuleIsolationConfig* config);
void idcu_module_isolator_destroy(idcu_ModuleIsolator* isolator);
int  idcu_module_isolator_start(idcu_ModuleIsolator* isolator);
int  idcu_module_isolator_stop(idcu_ModuleIsolator* isolator);
int  idcu_module_isolator_load_module(idcu_ModuleIsolator* isolator, const char* path, const idcu_IsolatedModule* template, idcu_IsolatedModuleId* out_id);
int  idcu_module_isolator_unload_module(idcu_ModuleIsolator* isolator, idcu_IsolatedModuleId module_id);
int  idcu_module_isolator_start_module(idcu_ModuleIsolator* isolator, idcu_IsolatedModuleId module_id);
int  idcu_module_isolator_stop_module(idcu_ModuleIsolator* isolator, idcu_IsolatedModuleId module_id);
int  idcu_module_set_resource_limits(idcu_IsolatedModule* module, const idcu_ResourceConstraints* limits);
int  idcu_module_grant_permission(idcu_IsolatedModule* module, idcu_ModulePermission perm);
int  idcu_module_has_permission(idcu_IsolatedModule* module, idcu_ModulePermission perm);
```

### 2.4 跨平台适配
- **沙箱**: 使用 idcu-sandbox 库封装的跨平台沙箱 API
- **线程**: 使用 idcu-common 库提供的跨平台线程 API
- **同步**: 使用 idcu-common 库提供的跨平台互斥锁

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以加载和隔离模块
- [ ] 资源限制正常工作
- [ ] 权限控制正常工作
- [ ] 模块故障可以隔离
- [ ] 模块间通信安全
- [ ] 沙箱执行正常工作
- [ ] 崩溃模块可以自动重启

### 3.2 性能验收
- 模块启动时间 ≤ 500ms
- 故障恢复时间 ≤ 1s
- 支持 100+ 隔离模块
- 隔离开销 ≤ 5% CPU
- 内存隔离开销 ≤ 10MB/模块

### 3.3 异常验收
- [ ] 模块崩溃不影响其他模块
- [ ] 越权访问被拒绝
- [ ] 资源超限被限制
- [ ] 多线程并发操作无数据竞争

---

## 4. 执行计划

### 4.1 工期
2 天/人

### 4.2 里程碑
- D1：完成接口定义、头文件、CMakeLists.txt、module.yaml、README.md
- D2：完成核心隔离逻辑、资源限制、权限控制、单元测试

### 4.3 人力
1 人（技能要求：C 语言 + 系统安全）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名：idcu_module_isolator_* 小写加下划线

### 5.2 测试要求
- 单元测试覆盖率 ≥ 75%
- 测试覆盖：模块加载、资源限制、权限控制、故障隔离

### 5.3 部署指引
- 编译命令：`cmake -B build &amp;&amp; cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::module-isolation)`

---

## 6. 风险与应对

### 6.1 风险 1
描述：隔离级别太高影响模块功能  
应对：提供多级隔离，允许按需选择

### 6.2 风险 2
描述：资源限制实现复杂  
应对：先实现基础限制（内存、文件描述符），其他后续扩展

---

## 7. 详细实现步骤

### 7.1 创建目录结构

```bash
mkdir -p libs/idcu-module-isolation/include/idcu/module_isolation
mkdir -p libs/idcu-module-isolation/src/idcu/module_isolation
mkdir -p libs/idcu-module-isolation/tests
mkdir -p libs/idcu-module-isolation/examples
```

### 7.2 创建模块隔离头文件 (module_isolation.h)

（保留原有的详细代码实现）

---

## 8. 验证检查清单

- [ ] 模块隔离头文件已创建
- [ ] 模块隔离实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以加载和隔离模块
- [ ] 资源限制正常工作
- [ ] 权限控制正常工作

---

## 9. Git 提交

```bash
git add libs/idcu-module-isolation/
git commit -m "feat: add idcu-module-isolation library

- Add module memory isolation
- Add module permission control
- Add module resource limits
- Add module fault isolation
- Add inter-module communication security
- Add module sandbox execution
- Add CMake build configuration
- Add module.yaml metadata"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 模块无法加载 | 权限或路径问题 | 检查文件权限和路径 |
| 模块崩溃 | 隔离级别太严格 | 适当降低隔离级别 |
| 资源超限 | 限制设置过低 | 调整资源限制配置 |
