# 任务 3.24: idcu-distributed - 分布式节点支持

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-08  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的分布式节点支持库，支持节点发现和注册、节点间通信、领导者选举、数据一致性、故障检测和恢复、消息广播和组播，满足节点发现延迟 ≤ 5 秒、支持 100+ 节点集群、消息投递延迟 ≤ 100ms 的性能要求。

### 1.2 不做什么
- 不实现完整的 Raft 共识算法（简化版）
- 不实现跨数据中心的分布式支持
- 不实现拜占庭容错
- 不实现分布式事务

### 1.3 输入
- 本地节点配置（节点名称、地址、端口、标签、元数据）
- 种子节点列表
- 分布式配置（一致性级别、心跳间隔、故障超时）
- 消息数据

### 1.4 输出
- 节点 ID（唯一标识）
- 节点列表（在线/离线）
- 领导者信息
- 返回码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 库已实现
- idcu-network 库已实现
- idcu-msgbus 库已实现
- idcu-log 库已实现
- phase3 前 23 个任务已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **节点发现**: UDP 广播 + 种子节点
- **领导者选举**: 简化的 Raft 算法（心跳 + 投票）
- **数据一致性**: 支持强一致性、最终一致性、仲裁一致性
- **消息传递**: TCP 点对点 + UDP 广播/多播
- **故障检测**: 心跳超时机制

### 2.2 核心逻辑
```
节点启动流程：
1. 初始化本地节点配置
2. 连接种子节点
3. 加入集群
4. 开始心跳
5. 参与领导者选举

领导者选举流程：
1. 检测当前领导者是否存活
2. 超时后转为候选者
3. 请求投票
4. 获得多数票成为领导者
5. 发送心跳维持领导地位

消息传递流程：
1. 构造消息
2. 查找目标节点
3. 建立连接（如需要）
4. 发送消息
5. 等待确认（如需要）
```

### 2.3 数据结构/接口
```c
typedef uint64_t idcu_NodeId;

typedef enum
{
    IDCU_NODE_STATE_UNKNOWN = 0,
    IDCU_NODE_STATE_OFFLINE,
    IDCU_NODE_STATE_ONLINE,
    IDCU_NODE_STATE_LEADER,
    IDCU_NODE_STATE_FOLLOWER,
    IDCU_NODE_STATE_CANDIDATE
} idcu_NodeState;

typedef enum
{
    IDCU_CONSISTENCY_STRONG = 0,
    IDCU_CONSISTENCY_EVENTUAL,
    IDCU_CONSISTENCY_QUORUM
} idcu_ConsistencyLevel;

typedef struct
{
    idcu_NodeId id;
    char name[128];
    char address[256];
    uint16_t port;
    idcu_NodeState state;
    uint64_t last_heartbeat;
    uint64_t uptime;
    char metadata[1024];
} idcu_NodeInfo;

int  idcu_distributed_node_init(idcu_DistributedNode* node, const idcu_DistributedConfig* config);
int  idcu_distributed_node_start(idcu_DistributedNode* node);
int  idcu_distributed_node_stop(idcu_DistributedNode* node);
int  idcu_distributed_send_to(idcu_DistributedNode* node, idcu_NodeId target_id, const void* data, size_t data_size);
int  idcu_distributed_broadcast(idcu_DistributedNode* node, const void* data, size_t data_size);
int  idcu_distributed_get_leader(idcu_DistributedNode* node, idcu_NodeId* leader_id);
```

### 2.4 跨平台适配
- **网络通信**: 使用 idcu-network 库封装的跨平台 Socket API
- **时间戳**: 使用 idcu-common 库提供的跨平台时间函数
- **线程同步**: 使用 idcu-common 库提供的跨平台互斥锁

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以发现和注册节点
- [ ] 节点间通信正常（点对点和广播）
- [ ] 领导者选举正常工作
- [ ] 数据一致性正常（三种级别）
- [ ] 故障检测和恢复正常工作
- [ ] 支持 100+ 节点集群

### 3.2 性能验收
- 节点发现延迟 ≤ 5 秒（局域网内）
- 消息投递延迟 ≤ 100ms
- 领导者选举时间 ≤ 5 秒
- 支持 1000+ 消息/秒吞吐量
- 内存占用 ≤ 50MB（100 节点）

### 3.3 异常验收
- [ ] 网络分区后可以自动恢复
- [ ] 节点故障后集群可以继续工作
- [ ] 消息丢失后可以重试
- [ ] 多线程并发操作无数据竞争

---

## 4. 执行计划

### 4.1 工期
4 天/人

### 4.2 里程碑
- D1：完成接口定义、头文件、CMakeLists.txt、module.yaml、README.md
- D2：完成节点发现和通信核心逻辑
- D3：完成领导者选举和数据一致性
- D4：完成故障检测、单元测试和集成测试

### 4.3 人力
1 人（技能要求：C 语言 + 分布式系统 + 网络编程）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名：idcu_distributed_* 小写加下划线

### 5.2 测试要求
- 单元测试覆盖率 ≥ 75%
- 集成测试覆盖：节点发现、通信、领导者选举、故障恢复
- 跨平台测试（Windows + Linux）

### 5.3 部署指引
- 编译命令：`cmake -B build &amp;&amp; cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::distributed)`

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险 1
描述：网络分区导致脑裂  
应对：使用法定人数投票机制，确保只有一个领导者

### 6.2 风险 2
描述：消息丢失导致数据不一致  
应对：支持消息确认和重试机制，提供不同一致性级别

---

## 7. 详细实现步骤

（保留原有详细实现步骤）

---

## 8. 验证检查清单

- [ ] 分布式头文件已创建
- [ ] 分布式实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 节点可以正常启动和停止
- [ ] 节点发现功能正常工作
- [ ] 消息广播功能正常工作
- [ ] 可以正常编译
- [ ] 单元测试通过
- [ ] 性能指标达标

---

## 9. Git 提交

```bash
git add libs/idcu-distributed/
git commit -m "feat: add idcu-distributed library

- Add node discovery and registration
- Add inter-node communication
- Add leader election (Raft-like)
- Add data consistency with multiple levels
- Add failure detection and recovery
- Add message broadcast and multicast
- Add CMake build configuration
- Add module.yaml metadata"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 节点无法发现其他节点 | 网络问题或种子节点配置错误 | 检查网络连接和种子节点配置 |
| 领导者选举失败 | 节点数量不足或网络分区 | 确保至少 3 个节点，检查网络 |
| 数据不同步 | 一致性级别设置或网络延迟 | 调整一致性级别，检查网络 |
