# 任务 2.5: 消息总线

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建高效的消息总线，为 IDCU Agent 提供模块间通信支持，包括消息发送和接收、消息队列管理、消息优先级、消息路由、消息持久化（可选）。

### 1.2 不做什么
- 不实现分布式消息传递
- 不实现消息持久化（初始版本）
- 不实现消息加密
- 不实现消息压缩

### 1.3 输入
- idcu-common 库（任务 2.1 完成）

### 1.4 输出
- 完整的消息总线库
- 消息发布/订阅 API
- 消息队列管理
- 优先级消息处理

### 1.5 前置依赖
- 任务 2.1 已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **消息模式**: 发布-订阅 (Publish-Subscribe)
- **队列实现**: 向量 + 优先级排序
- **最大主题数**: 64
- **最大消息大小**: 4KB

### 2.2 核心逻辑
```
1. 消息发布
   ├── 创建消息对象
   ├── 设置优先级
   ├── 加入对应主题队列
   └── 按优先级排序

2. 消息订阅
   ├── 注册订阅者
   ├── 保存回调函数
   └── 保存用户数据

3. 消息处理
   ├── 遍历所有主题
   ├── 按优先级取出消息
   ├── 调用所有订阅者回调
   └── 释放消息
```

### 2.3 数据结构/接口
```c
// 消息优先级
typedef enum {
    IDCU_MSG_PRIORITY_LOW = 0,
    IDCU_MSG_PRIORITY_NORMAL = 1,
    IDCU_MSG_PRIORITY_HIGH = 2,
    IDCU_MSG_PRIORITY_CRITICAL = 3
} idcu_MsgPriority;

// 消息总线
int idcu_msgbus_init(idcu_MsgBus** bus);
int idcu_msgbus_publish(idcu_MsgBus* bus, idcu_MsgTopic topic,
                         const void* data, size_t data_size,
                         idcu_MsgPriority priority);
int idcu_msgbus_subscribe(idcu_MsgBus* bus, idcu_MsgTopic topic,
                           idcu_MsgHandler handler, void* user_data,
                           idcu_MsgSubscriber** subscriber);
int idcu_msgbus_process(idcu_MsgBus* bus);
```

### 2.4 跨平台适配
- 无特殊跨平台需求，使用标准 C

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 消息发布功能正常
- [ ] 消息订阅功能正常
- [ ] 多个订阅者可以收到同一消息
- [ ] 消息优先级功能正常
- [ ] 消息处理功能正常

### 3.2 性能验收
- 消息发布 QPS ≥ 10000
- 消息投递延迟 ≤ 1ms
- 支持至少 64 个主题

### 3.3 异常验收
- [ ] 消息过大返回错误
- [ ] 主题过多返回错误
- [ ] NULL 参数检查正确

---

## 4. 执行计划

### 4.1 工期
1 天/人

### 4.2 里程碑
- D6-01: 完成消息定义和队列
- D6-03: 完成发布订阅
- D6-05: 完成优先级处理
- D6-06: 完成测试和验证

### 4.3 人力
1 人（技能要求：C 语言、数据结构）

---

## 5. 工程化要求

### 5.1 编码规范
- 遵循项目 .clang-format 规范

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%

### 5.3 部署指引
- 库文件: libs/idcu-msgbus/

---

## 6. 风险与应对

### 6.1 风险1
描述：消息队列满导致消息丢失  
应对：设置合理队列大小，提供丢弃策略

### 6.2 风险2
描述：消息处理回调耗时过长  
应对：异步处理或限制回调执行时间

---

## 7. 详细实现步骤

（保留原文档的详细实现步骤内容）

---

## 8. 验证检查清单

- [ ] 头文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 代码可以成功编译
- [ ] 基本消息发布和订阅功能正常

---

## 9. Git 提交

```bash
git add libs/idcu-msgbus/
git commit -m "feat: add idcu-msgbus library

- Add message publishing and subscription
- Add priority-based message queuing
- Add topic-based message routing
- Add CMake build configuration"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 消息未收到 | 订阅失败 | 检查订阅返回值 |
| 优先级失效 | 队列未正确排序 | 检查优先级比较逻辑 |

