# 任务 3.13: idcu-msgbus - 消息总线库

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的消息总线库，支持发布/订阅模式、消息队列、主题订阅、消息过滤、异步处理、线程安全、优先级支持，满足消息投递延迟 ≤ 1ms、支持 10000+ 消息/秒吞吐量、支持 1000+ 并发订阅者的性能要求。

### 1.2 不做什么
- 不实现持久化消息队列
- 不实现分布式消息传递
- 不实现消息确认（ACK）机制
- 不实现消息路由（仅主题匹配）
- 不实现消息持久化到磁盘

### 1.3 输入
- 消息主题
- 消息数据
- 消息类型
- 消息优先级
- 订阅回调函数

### 1.4 输出
- 消息 ID
- 消息投递回调
- 错误码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 基础库已可用
- idcu-log 日志库已可用
- phase2 已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **消息模式**: 发布/订阅（Pub/Sub）
- **数据结构**: Vector 存储订阅者，HashMap 按主题索引
- **队列**: 带容量的环形队列或 Vector
- **同步**: Mutex 保证线程安全，Condition 实现队列等待
- **调度**: 后台工作线程处理消息队列

### 2.2 核心逻辑
```
消息发布流程：
1. 加锁
2. 创建消息对象
3. 加入消息队列
4. 通知工作线程
5. 解锁
6. 返回消息 ID

消息投递流程：
1. 工作线程从队列取出消息
2. 查找匹配的订阅者
3. 调用每个订阅者的回调
4. 释放消息
5. 继续处理下一条

订阅流程：
1. 加锁
2. 创建订阅记录
3. 加入订阅列表
4. 按主题索引
5. 解锁

取消订阅流程：
1. 加锁
2. 查找并删除订阅记录
3. 解锁
```

### 2.3 数据结构/接口
```c
#ifndef IDCU_MSGBUS_MSG_BUS_H
#define IDCU_MSGBUS_MSG_BUS_H

#include "idcu/common/config.h"
#include "idcu/common/lock.h"
#include "idcu/coroutine/context.h"

#define IDCU_MSG_BATCH_MAX          32
#define IDCU_MSG_ZEROCOPY_POOL_SIZE 256
#define IDCU_MSG_MOD_INDEX_SIZE     256

typedef enum
{
    IDCU_MSG_PRIO_LOW = 0,
    IDCU_MSG_PRIO_NORMAL,
    IDCU_MSG_PRIO_HIGH,
    IDCU_MSG_PRIO_REALTIME,
    IDCU_MSG_PRIO_COUNT
} idcu_MsgPriority;

typedef struct
{
    uint8_t* data;
    uint32_t size;
    uint32_t ref_count;
} idcu_ZeroCopyPayload;

typedef struct
{
    idcu_StackContext     data;
    idcu_ZeroCopyPayload* payload;
    uint32_t              target_mod_id;
    uint32_t              source_mod_id;
    idcu_MsgPriority      priority;
    uint64_t              timestamp;
    uint32_t              retry_count;
} idcu_Message;

typedef struct
{
    idcu_Message msgs[IDCU_MSG_BATCH_MAX];
    uint32_t     count;
} idcu_MessageBatch;

#define IDCU_MSG_QUEUE_SIZE IDCU_CONFIG_MAX_MSG

typedef struct
{
    idcu_Message queue[IDCU_MSG_QUEUE_SIZE];
    uint32_t     head;
    uint32_t     tail;
    idcu_Mutex   lock;
} idcu_PriorityQueue;

typedef struct
{
    idcu_PriorityQueue   prio_queues[IDCU_MSG_PRIO_COUNT];
    uint32_t             subs[16];
    idcu_ZeroCopyPayload payload_pool[IDCU_MSG_ZEROCOPY_POOL_SIZE];
    uint8_t              payload_in_use[IDCU_MSG_ZEROCOPY_POOL_SIZE];
    uint32_t             payload_free_head;
    idcu_Mutex           payload_lock;
} idcu_MessageBus;

void idcu_msg_bus_init(idcu_MessageBus* bus);
void idcu_msg_bus_destroy(idcu_MessageBus* bus);
int  idcu_msg_send(idcu_MessageBus* bus, uint32_t src_mod, uint32_t dst_mod, idcu_MsgPriority prio,
                   const idcu_StackContext* ctx);
int  idcu_msg_recv(idcu_MessageBus* bus, uint32_t mod_id, idcu_Message* msg);
int  idcu_msg_broadcast(idcu_MessageBus* bus, uint32_t src_mod, idcu_MsgPriority prio,
                        const idcu_StackContext* ctx);
uint32_t idcu_msg_get_count(idcu_MessageBus* bus);
int      idcu_msg_send_zerocopy(idcu_MessageBus* bus, uint32_t src_mod, uint32_t dst_mod,
                                idcu_MsgPriority prio, const uint8_t* data, uint32_t size);
int      idcu_msg_recv_zerocopy(idcu_MessageBus* bus, uint32_t mod_id, idcu_Message* msg);
void     idcu_msg_release_payload(idcu_MessageBus* bus, idcu_ZeroCopyPayload* payload);
int      idcu_msg_send_batch(idcu_MessageBus* bus, idcu_MessageBatch* batch);
int      idcu_msg_recv_batch(idcu_MessageBus* bus, uint32_t mod_id, idcu_MessageBatch* batch,
                             uint32_t max_count);

#endif  // IDCU_MSGBUS_MSG_BUS_H
```

### 2.4 跨平台适配
- **线程同步**: 使用 idcu-common 提供的跨平台互斥锁和条件变量
- **时间戳**: 使用跨平台的时间获取函数
- **内存管理**: 标准 C 内存分配函数
- **无特殊平台差异**: 核心逻辑完全跨平台

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以发送点对点消息
- [ ] 可以发送广播消息
- [ ] 可以接收消息
- [ ] 支持 4 个优先级（低、正常、高、实时）
- [ ] 支持零拷贝消息传输
- [ ] 支持批量消息发送和接收
- [ ] 线程安全，多线程环境下无数据竞争
- [ ] 消息按优先级正确调度

### 3.2 性能验收
- 消息投递延迟 ≤ 1ms（99% 延迟）
- 支持 10000+ 消息/秒吞吐量
- 支持 1000+ 并发订阅者
- 零拷贝消息比普通消息快 50% 以上
- 批量消息比单条消息快 30% 以上
- 内存占用 ≤ 10MB（默认配置）

### 3.3 异常验收
- [ ] 发送到不存在的模块返回错误
- [ ] 队列满时返回错误或阻塞
- [ ] 零拷贝池耗尽时安全处理
- [ ] NULL 参数安全处理
- [ ] 内存不足时安全处理

---

## 4. 执行计划

### 4.1 工期
3 小时/人

### 4.2 里程碑
- D1-00: 完成头文件定义和数据结构（30 分钟）
- D1-30: 完成基础消息发送和接收（1 小时）
- D1-90: 完成优先级队列和调度（30 分钟）
- D2-00: 完成零拷贝和批量消息（30 分钟）
- D2-30: 完成单元测试和文档（30 分钟）

### 4.3 人力
1 人（技能要求：C 语言 + 多线程编程）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，结构体前缀 idcu_
- 所有公共 API 有 Doxygen 风格注释

### 5.2 测试要求
- 单元测试覆盖率 ≥ 85%
- 测试用例覆盖：点对点、广播、优先级、零拷贝、批量、多线程
- 性能测试验证吞吐量和延迟指标

### 5.3 部署指引
- 编译命令：`cmake -B build && cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::msgbus)`

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：多线程环境下锁竞争导致性能下降  
应对：使用细粒度锁，分离优先级队列锁

### 6.2 风险2
描述：消息队列溢出导致消息丢失  
应对：队列满时返回错误，提供可配置的队列大小

### 6.3 风险3
描述：性能不满足预期要求  
应对：进行性能基准测试，优化关键路径代码

### 6.4 风险4
描述：跨平台兼容性问题  
应对：使用跨平台 API，充分测试不同平台

---

## 7. 详细实现步骤

### 步骤 1: 创建目录结构
```bash
mkdir -p libs/idcu-msgbus/include/idcu/msgbus
mkdir -p libs/idcu-msgbus/src/idcu/msgbus
mkdir -p libs/idcu-msgbus/tests
mkdir -p libs/idcu-msgbus/examples
```

### 步骤 2: 创建头文件 msg_bus.h
定义消息优先级、消息结构、消息总线结构、核心 API 函数。

### 步骤 3: 创建实现文件 msg_bus.c
实现消息总线初始化、消息发送接收、广播、零拷贝、批量操作等功能。

### 步骤 4: 创建 CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-msgbus VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-msgbus STATIC src/idcu/msgbus/msg_bus.c)
target_include_directories(idcu-msgbus PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(idcu-msgbus PRIVATE idcu::common idcu::coroutine)
add_library(idcu::msgbus ALIAS idcu-msgbus)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 步骤 5: 创建 module.json
```json
{
  "name": "idcu-msgbus",
  "version": "1.0.0",
  "description": "Message bus library for IDCU Agent",
  "author": "IDCU Team",
  "license": "Apache-2.0",
  "dependencies": ["idcu-common", "idcu-coroutine"]
}
```

### 步骤 6: 创建 README.md
参考 libs/idcu-msgbus/README.md 现有内容。

---

## 8. 验证检查清单

- [ ] 头文件 msg_bus.h 已创建
- [ ] 实现文件 msg_bus.c 已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.json 已创建
- [ ] README.md 已创建
- [ ] 可以正常编译通过
- [ ] 单元测试通过率 100%
- [ ] 性能测试达标（延迟 ≤ 1ms，吞吐量 ≥ 10000 msg/s）
- [ ] 多线程测试通过
- [ ] 代码已通过 clang-format 格式化
- [ ] 代码已通过 clang-tidy 静态分析
- [ ] 符合工程化标准要求
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-msgbus/
git commit -m "feat: add idcu-msgbus library

- Add multi-priority message queues (4 priorities)
- Add point-to-point message passing
- Add broadcast messaging
- Add zero-copy message transfer
- Add batch message operations
- Add thread-safe implementation
- Add CMake build configuration
- Add unit tests and examples"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 消息丢失 | 队列满了 | 增加队列大小或处理速度 |
| 优先级不生效 | 优先级顺序错误 | 检查优先级枚举定义 |
| 零拷贝失败 | 零拷贝池耗尽 | 增加零拷贝池大小 |
| 多线程崩溃 | 锁使用错误 | 检查锁的获取和释放 |
| 性能不达标 | 锁竞争严重 | 使用细粒度锁或优化关键路径 |
| 内存泄漏 | 零拷贝 payload 未释放 | 确保调用 idcu_msg_release_payload |
