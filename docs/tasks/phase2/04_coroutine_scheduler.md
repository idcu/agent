# 任务 2.4: 协程调度器

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建高性能的协程调度器，为 IDCU Agent 提供轻量级并发支持，包括协程上下文切换、协程创建和销毁、协程调度算法、协程间通信、协程池管理。

### 1.2 不做什么
- 不实现抢占式调度
- 不实现多线程协程调度
- 不实现协程栈动态增长
- 不实现协程调试工具

### 1.3 输入
- idcu-common 库（任务 2.1 完成）

### 1.4 输出
- 完整的协程调度器库
- 协程创建/销毁 API
- 协程调度器 API
- 协程状态管理

### 1.5 前置依赖
- 任务 2.1 已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **上下文切换**: Windows Fiber / Linux ucontext
- **调度策略**: 轮询 (Round-Robin)
- **栈大小**: 64KB 默认
- **最大协程数**: 128

### 2.2 核心逻辑
```
1. 协程创建
   ├── 分配栈空间
   ├── 初始化上下文
   ├── 设置入口函数
   └── 加入就绪队列

2. 协程调度
   ├── 从就绪队列取出
   ├── 切换上下文
   ├── 执行协程函数
   ├── yield 时切换回
   └── 重新加入队列

3. 协程销毁
   ├── 释放栈空间
   ├── 清理上下文
   └── 从队列移除
```

### 2.3 数据结构/接口
```c
// 协程状态
typedef enum {
    IDCU_CORO_READY,
    IDCU_CORO_RUNNING,
    IDCU_CORO_WAITING,
    IDCU_CORO_SUSPENDED,
    IDCU_CORO_FINISHED
} idcu_CoroutineState;

// 调度器
int idcu_coro_scheduler_init(idcu_CoroutineScheduler** scheduler);
int idcu_coro_create(idcu_CoroutineScheduler* scheduler, idcu_Coroutine** coro,
                      const idcu_CoroutineConfig* config);
int idcu_coro_yield(void);
int idcu_coro_scheduler_run(idcu_CoroutineScheduler* scheduler);
```

### 2.4 跨平台适配
- **上下文切换**: Windows Fiber API / Linux ucontext
- **线程本地存储**: __thread / pthread_setspecific

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以创建和销毁协程
- [ ] 协程 yield 功能正常
- [ ] 调度器可以调度多个协程
- [ ] 协程状态管理正确
- [ ] 支持多种调度策略

### 3.2 性能验收
- 协程上下文切换耗时 ≤ 1μs
- 协程创建耗时 ≤ 100μs
- 支持至少 100 个协程并发

### 3.3 异常验收
- [ ] 栈溢出时安全处理
- [ ] 协程异常终止不影响其他协程
- [ ] NULL 参数检查正确

---

## 4. 执行计划

### 4.1 工期
1 天/人

### 4.2 里程碑
- D5-01: 完成协程上下文切换
- D5-03: 完成协程创建和销毁
- D5-05: 完成调度器
- D5-06: 完成测试和验证

### 4.3 人力
1 人（技能要求：C 语言、底层编程）

---

## 5. 工程化要求

### 5.1 编码规范
- 遵循项目 .clang-format 规范

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%

### 5.3 部署指引
- 库文件: libs/idcu-coroutine/

---

## 6. 风险与应对

### 6.1 风险1
描述：跨平台上下文切换差异  
应对：充分测试，准备备选方案

### 6.2 风险2
描述：协程栈溢出  
应对：设置合理默认值，提供配置选项

---

## 7. 详细实现步骤

（保留原文档的详细实现步骤内容）

---

## 8. 验证检查清单

- [ ] 头文件已创建
- [ ] 源文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 代码可以成功编译
- [ ] 基本协程创建和调度功能正常

---

## 9. Git 提交

```bash
git add libs/idcu-coroutine/
git commit -m "feat: add idcu-coroutine library

- Add coroutine context switching
- Add coroutine scheduler with multiple policies
- Add coroutine creation and management
- Add CMake build configuration"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 协程切换失败 | 栈空间不足 | 增加 IDCU_COROUTINE_STACK_SIZE |
| 内存泄漏 | 忘记调用 destroy | 确保所有创建的协程都被销毁 |

