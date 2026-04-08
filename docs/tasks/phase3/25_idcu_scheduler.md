# 任务 3.25: idcu-scheduler - 任务调度库

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-08  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的任务调度库，支持定时任务、周期任务、一次性任务、Cron 表达式、任务优先级、任务依赖、任务状态监控、任务取消，满足调度精度 ≤ 100ms、支持 1000+ 并发任务、任务执行延迟 ≤ 1s 的性能要求。

### 1.2 不做什么
- 不实现分布式任务调度
- 不实现任务持久化（重启后丢失）
- 不实现负载均衡
- 不实现历史任务记录

### 1.3 输入
- 任务配置（名称、类型、优先级、函数、参数）
- Cron 表达式
- 调度器配置（tick 间隔）

### 1.4 输出
- 任务 ID（唯一标识）
- 任务状态（pending、running、completed、failed、cancelled）
- 返回码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 库已实现
- idcu-log 库已实现
- idcu-utils 库已实现
- phase3 前 24 个任务已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **调度策略**: 优先级队列 + 定时器
- **Cron 解析**: 标准 Cron 表达式解析（6 位：秒 分 时 日 月 周）
- **任务执行**: 多线程工作池
- **优先级**: 4 级优先级（low、normal、high、critical）

### 2.2 核心逻辑
```
调度器工作流程：
1. 主线程定期（tick_interval_ms）检查任务队列
2. 找出到期任务，按优先级排序
3. 将任务分配给工作线程
4. 工作线程执行任务
5. 更新任务状态，调用回调

任务添加流程：
1. 分配唯一任务 ID
2. 初始化任务结构体
3. 计算下次执行时间
4. 加入任务队列
5. 唤醒调度线程

Cron 计算流程：
1. 解析 Cron 表达式
2. 从当前时间开始计算
3. 找到第一个符合条件的时间点
4. 返回下次执行时间戳
```

### 2.3 数据结构/接口
```c
typedef uint64_t idcu_TaskId;
typedef enum { IDCU_TASK_TYPE_ONCE, IDCU_TASK_TYPE_PERIODIC, IDCU_TASK_TYPE_CRON } idcu_TaskType;
typedef enum { IDCU_TASK_STATUS_PENDING, IDCU_TASK_STATUS_SCHEDULED, IDCU_TASK_STATUS_RUNNING, IDCU_TASK_STATUS_COMPLETED, IDCU_TASK_STATUS_FAILED, IDCU_TASK_STATUS_CANCELLED, IDCU_TASK_STATUS_PAUSED } idcu_TaskStatus;
typedef enum { IDCU_TASK_PRIORITY_LOW, IDCU_TASK_PRIORITY_NORMAL, IDCU_TASK_PRIORITY_HIGH, IDCU_TASK_PRIORITY_CRITICAL } idcu_TaskPriority;

typedef int (*idcu_TaskFunc)(void* user_data);
typedef void (*idcu_TaskCompleteCallback)(idcu_TaskId id, int result, void* user_data);

int  idcu_scheduler_init(idcu_Scheduler* scheduler, const idcu_SchedulerConfig* config);
int  idcu_scheduler_start(idcu_Scheduler* scheduler);
void idcu_scheduler_stop(idcu_Scheduler* scheduler);
idcu_TaskId idcu_scheduler_add_once(idcu_Scheduler* scheduler, const char* name, uint64_t delay_ms, idcu_TaskFunc func, void* user_data);
idcu_TaskId idcu_scheduler_add_periodic(idcu_Scheduler* scheduler, const char* name, uint64_t interval_ms, idcu_TaskFunc func, void* user_data);
idcu_TaskId idcu_scheduler_add_cron(idcu_Scheduler* scheduler, const char* name, const char* cron_expr, idcu_TaskFunc func, void* user_data);
int  idcu_scheduler_cancel_task(idcu_Scheduler* scheduler, idcu_TaskId id);
int  idcu_scheduler_get_task_status(idcu_Scheduler* scheduler, idcu_TaskId id, idcu_TaskStatus* status);
```

### 2.4 跨平台适配
- **时间**: 使用 idcu-common 库提供的跨平台时间函数
- **线程**: 使用 idcu-common 库提供的跨平台线程 API
- **同步**: 使用 idcu-common 库提供的跨平台互斥锁

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以添加和执行一次性任务
- [ ] 可以添加和执行周期任务
- [ ] 可以添加和执行 Cron 任务
- [ ] 任务优先级正常工作
- [ ] 任务依赖正常工作
- [ ] 可以取消任务
- [ ] 任务状态监控正常工作

### 3.2 性能验收
- 调度精度 ≤ 100ms
- 支持 1000+ 并发任务
- 任务执行延迟 ≤ 1s
- Cron 解析耗时 ≤ 1ms
- 内存占用 ≤ 10MB（1000 任务）

### 3.3 异常验收
- [ ] 无效 Cron 表达式返回错误
- [ ] 任务执行失败可以重试
- [ ] 调度器可以安全停止
- [ ] 多线程并发添加/取消无数据竞争

---

## 4. 执行计划

### 4.1 工期
2 天/人

### 4.2 里程碑
- D1：完成接口定义、头文件、CMakeLists.txt、module.yaml、README.md
- D2：完成核心调度逻辑、Cron 解析、单元测试

### 4.3 人力
1 人（技能要求：C 语言 + 定时器编程）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名：idcu_scheduler_* 小写加下划线

### 5.2 测试要求
- 单元测试覆盖率 ≥ 70%
- 测试覆盖：任务添加、取消、执行、Cron 解析、异常场景

### 5.3 部署指引
- 编译命令：`cmake -B build &amp;&amp; cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::scheduler)`

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险 1
描述：任务执行时间过长阻塞调度器  
应对：使用独立的工作线程池执行任务

### 6.2 风险 2
描述：Cron 表达式计算错误  
应对：提供 Cron 表达式验证函数，充分测试

---

## 7. 详细实现步骤

（保留原有详细实现步骤）

---

## 8. 验证检查清单

- [ ] 任务调度头文件已创建
- [ ] 任务调度实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以添加和执行任务
- [ ] Cron 表达式正常工作
- [ ] 任务状态监控正常工作

---

## 9. Git 提交

```bash
git add libs/idcu-scheduler/
git commit -m "feat: add idcu-scheduler library

- Add one-time tasks
- Add periodic tasks
- Add cron expressions
- Add task priorities
- Add task dependencies
- Add task status monitoring
- Add task cancellation
- Add task retry
- Add task statistics
- Add CMake build configuration
- Add module.yaml metadata"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 任务不执行 | 任务被禁用 | 确保任务已启用 |
| Cron 表达式错误 | 格式不正确 | 检查 Cron 表达式格式 |
| 任务失败 | 函数返回错误 | 检查任务函数实现 |
