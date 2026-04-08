# 任务 5.4: heartbeat - 心跳模块

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建心跳模块，支持：
- 定时心跳发送
- 心跳接收和验证
- 节点状态监控
- 心跳超时检测
- 自动重连
- 心跳历史记录
- 心跳统计
- 多协议支持
- 自定义心跳数据

### 1.2 不做什么
- 不实现复杂的集群管理
- 不实现领导者选举
- 不实现分布式一致性

### 1.3 输入
- 节点配置：节点地址、端口、协议
- 心跳参数：间隔、超时时间
- 网络连接

### 1.4 输出
- 节点状态信息
- 心跳统计数据
- 超时告警通知

### 1.5 前置依赖
- ✅ phase3 完成：idcu-network
- ✅ phase3 完成：idcu-metrics
- ✅ 5.1 完成：core-module

---

## 2. 技术实现方案

### 2.1 核心选型
- 网络通信：idcu-network
- 协议支持：TCP、UDP、HTTP、消息总线
- 指标统计：idcu-metrics

### 2.2 核心逻辑
```
1. 初始化心跳管理器
2. 注册节点信息
3. 启动发送线程
4. 启动接收线程
5. 启动监控线程
6. 定时发送心跳包
7. 接收并验证心跳包
8. 检测心跳超时
9. 更新节点状态
10. 触发状态变更通知
```

### 2.3 数据结构/接口
```c
typedef enum {
    IDCU_HEARTBEAT_STATUS_UNKNOWN = 0,
    IDCU_HEARTBEAT_STATUS_ALIVE,
    IDCU_HEARTBEAT_STATUS_WARNING,
    IDCU_HEARTBEAT_STATUS_DEAD,
    IDCU_HEARTBEAT_STATUS_OFFLINE
} idcu_HeartbeatStatus;

typedef struct {
    idcu_HeartbeatNodeId id;
    char node_id[128];
    char node_name[128];
    idcu_HeartbeatStatus status;
    uint64_t last_heartbeat_at;
    uint64_t interval_ms;
    uint64_t timeout_ms;
    // ... 其他字段
} idcu_HeartbeatNode;

typedef struct {
    idcu_Vector nodes;
    idcu_Thread sender_thread;
    idcu_Thread receiver_thread;
    idcu_Thread monitor_thread;
    // ... 其他字段
} idcu_HeartbeatManager;

int idcu_heartbeat_manager_init(idcu_HeartbeatManager* hb, const idcu_HeartbeatManagerConfig* config);
int idcu_heartbeat_manager_start(idcu_HeartbeatManager* hb);
idcu_HeartbeatNodeId idcu_heartbeat_manager_add_node(idcu_HeartbeatManager* hb, const idcu_HeartbeatNode* node);
```

### 2.4 跨平台适配
- Socket 编程：Windows 使用 Winsock2，Linux 使用 POSIX socket
- 线程：使用统一的线程抽象接口
- 定时器：跨平台定时器实现

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以发送和接收心跳
- [ ] 节点状态监控正常工作
- [ ] 心跳超时检测正常工作
- [ ] 自动重连功能正常
- [ ] 心跳历史记录正常
- [ ] 多协议支持正常

### 3.2 性能验收
- [ ] 心跳发送延迟 ≤ 10ms
- [ ] 支持监控 ≥ 100 个节点
- [ ] 心跳检测响应时间 ≤ 1s
- [ ] 内存占用 ≤ 5MB

### 3.3 异常验收
- [ ] 网络中断时可以检测到
- [ ] 节点恢复时可以自动重连
- [ ] 心跳包格式错误时可以正确处理
- [ ] 单个节点故障不影响其他节点

---

## 4. 执行计划

### 4.1 工期
2 小时

### 4.2 里程碑
- D1：完成心跳模块接口定义
- D1：完成核心心跳功能
- D1：完成节点状态管理
- D1：完成测试和验证

### 4.3 人力
1 人（技能要求：C语言 + 网络编程）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐 .clang-format 规范
- 函数名小写+下划线
- 结构体前缀 idcu_

### 5.2 测试要求
- 单元测试覆盖率 ≥ 70%
- 测试覆盖所有节点状态
- 测试覆盖超时场景

### 5.3 部署指引
- 编译命令：cmake --build build
- 模块路径：modules/heartbeat/

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：网络抖动导致误报节点离线  
应对：设置合理的超时时间和告警阈值，增加连续失败次数检测

### 6.2 风险2
描述：心跳包丢失导致状态不准确  
应对：使用双向心跳，增加序列号验证

---

## 7. 详细实现步骤

（保留原文档的详细实现步骤内容）

---

## 8. 验证检查清单

- [ ] 心跳模块头文件已创建
- [ ] 心跳模块实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以发送和接收心跳
- [ ] 节点状态监控正常工作
- [ ] 心跳超时检测正常工作

---

## 9. Git 提交

```bash
git add modules/heartbeat/
git commit -m "feat: add heartbeat module

- Add periodic heartbeat sending
- Add heartbeat receiving and validation
- Add node status monitoring
- Add heartbeat timeout detection
- Add auto reconnect
- Add heartbeat history
- Add heartbeat statistics
- Add multi-protocol support
- Add custom heartbeat data
- Add CMake build configuration
- Add module.yaml metadata"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 心跳未发送 | 节点未启用 | 确保节点已启用 |
| 心跳超时 | 网络延迟 | 增加超时时间 |
| 节点离线 | 连接断开 | 检查网络连接 |
