# 任务 3.14: idcu-coroutine（独立库）

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-08  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建独立的协程库，为其他模块提供协程支持，包括：
- 协程创建和管理，支持最多 128 个并发协程
- 协程上下文切换，切换耗时 ≤ 1μs
- 协程调度器，支持 Round-Robin 调度策略
- 协程同步原语初步框架

### 1.2 不做什么
- 不实现完整的协程同步原语（mutex、semaphore 等，留待后续任务）
- 不实现多核并行调度
- 不支持协程优先级调度
- 不实现网络 I/O 协程化

### 1.3 输入
- 协程函数指针和参数
- 协程栈大小配置（默认 64KB）
- 调度器配置参数

### 1.4 输出
- 协程创建成功返回 0，失败返回错误码
- 协程状态枚举值
- 调度器运行状态

### 1.5 前置依赖
- idcu-common 库已完成
- CMake 构建系统已配置

---

## 2. 技术实现方案

### 2.1 核心选型
- 协程上下文切换：ucontext（Linux）/ Fibers（Windows）
- 编程语言：C99
- 调度策略：Round-Robin
- 内存管理：手动管理协程栈

### 2.2 核心逻辑
1. 初始化协程调度器
2. 创建协程，分配栈空间和上下文
3. 将协程加入就绪队列
4. 调度器运行，从就绪队列取出协程执行
5. 协程主动 yield 或结束，切换回调度器
6. 循环执行直到所有协程完成

### 2.3 数据结构/接口
```c
typedef enum {
    IDCU_CORO_READY,
    IDCU_CORO_RUNNING,
    IDCU_CORO_WAITING,
    IDCU_CORO_SUSPENDED,
    IDCU_CORO_FINISHED
} idcu_CoroutineState;

typedef struct idcu_Coroutine idcu_Coroutine;
typedef struct idcu_CoroutineScheduler idcu_CoroutineScheduler;

typedef void (*idcu_CoroutineFunc)(void* arg);

// 调度器接口
int  idcu_coro_scheduler_init(idcu_CoroutineScheduler** scheduler);
void idcu_coro_scheduler_destroy(idcu_CoroutineScheduler* scheduler);
int  idcu_coro_scheduler_run(idcu_CoroutineScheduler* scheduler);
int  idcu_coro_scheduler_stop(idcu_CoroutineScheduler* scheduler);

// 协程接口
int  idcu_coro_create(idcu_CoroutineScheduler* scheduler, idcu_Coroutine** coro,
                      idcu_CoroutineFunc func, void* arg);
void idcu_coro_destroy(idcu_Coroutine* coro);
int  idcu_coro_yield(void);
int  idcu_coro_suspend(idcu_Coroutine* coro);
int  idcu_coro_resume(idcu_Coroutine* coro);

// 协程状态
idcu_CoroutineState idcu_coro_get_state(const idcu_Coroutine* coro);
const char* idcu_coro_state_to_str(idcu_CoroutineState state);

// 当前协程
idcu_Coroutine* idcu_coro_current(void);

// 睡眠
int  idcu_coro_sleep_ms(uint32_t ms);
```

### 2.4 跨平台适配
- **Linux**: 使用 ucontext 系列函数（makecontext、swapcontext）
- **Windows**: 使用 Fibers API（CreateFiber、SwitchToFiber）
- 栈大小限制：Windows 默认栈大小可能不同，需显式配置

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- 可成功创建并运行 128 个协程
- 协程可以正确 yield 和 resume
- 调度器可以正确调度多个协程
- 协程状态转换正确（READY → RUNNING → FINISHED）

### 3.2 性能验收
- 协程上下文切换耗时 ≤ 1μs
- 128 个协程同时运行时内存占用 ≤ 8MB（每个协程 64KB 栈）
- 调度器调度开销 ≤ 5% CPU 占用

### 3.3 异常验收
- 协程栈溢出时有错误提示，程序不崩溃
- 创建超过 128 个协程返回错误码
- 销毁调度器时正确清理所有协程资源

---

## 4. 执行计划

### 4.1 工期
3 天/人

### 4.2 里程碑
- D1：完成目录结构、头文件和 CMake 配置
- D2：完成 Linux 下协程上下文切换和调度器核心逻辑
- D3：完成 Windows 适配、测试和文档

### 4.3 人力
1 人（技能要求：C 语言 + Linux/Windows 系统编程）

---

## 5. 工程化要求

### 5.1 编码规范
- 函数名：小写 + 下划线，前缀 `idcu_coro_`
- 结构体名：前缀 `idcu_`，首字母大写后续驼峰
- 宏定义：全大写 + 下划线，前缀 `IDCU_COROUTINE_`

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 测试场景包括：协程创建、yield/resume、调度器运行、异常处理

### 5.3 部署指引
- 编译命令：`cmake .. &amp;&amp; make`（Linux）或 `cmake .. &amp;&amp; msbuild`（Windows）
- 部署路径：链接到 idcu-coroutine 静态库

---

## 6. 风险与应对

### 6.1 风险1
描述：跨平台上下文切换实现复杂  
应对：先完成 Linux 版本验证，再移植到 Windows，复用抽象层接口

### 6.2 风险2
描述：协程栈溢出问题  
应对：设置合理的默认栈大小（64KB），添加栈大小配置选项，编译时开启栈保护

---

## 7. 详细实现步骤

### 7.1 创建目录结构
```bash
mkdir -p libs/idcu-coroutine/include/idcu/coroutine
mkdir -p libs/idcu-coroutine/src/idcu/coroutine
mkdir -p libs/idcu-coroutine/tests
mkdir -p libs/idcu-coroutine/examples
```

### 7.2 创建协程头文件 (coroutine.h)
```c
#ifndef IDCU_COROUTINE_COROUT