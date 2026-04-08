# 任务 3.13: idcu-msgbus - 消息总线库

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-08  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

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
// 主要头文件：idcu/msgbus/msg_bus.h

typedef uint64_t idcu_MessageId;
typedef uint32_t idcu_MessageType;

typedef struct
{
    idcu_MessageId id;
    idcu_MessageType type;
    char topic[256];
    void* data;
    size_t data_size;
    uint64_t timestamp;
    uint32_t priority;
} idcu_Message;

typedef void (*idcu_MessageHandler)(const idcu_Message* message, void* user_data);

typedef struct idcu_Subscription
{
    idcu_MessageHandler handler;
    void* user_data;
    char topic_filter[256];
    idcu_MessageType type_filter;
    int use_type_filter;
} idcu_Subscription;

typedef struct idcu_MessageQueue
{
    idcu_Vector messages;
    idcu_Mutex lock;
    idcu_Condition not_empty;
    idcu_Condition not_full;
    size_t capacity;
    int closed;
} idcu_MessageQueue;

typedef struct
{
    idcu_HashMap subscriptions;
    idcu_MessageQueue queue;
    idcu_Thread worker_thread;
    idcu_Mutex lock