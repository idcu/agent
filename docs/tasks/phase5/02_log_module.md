# 任务 5.2: log-module - 日志业务模块

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建日志业务模块，支持：
- 统一日志接口
- 多级别日志
- 多输出目标
- 日志轮转
- 日志过滤
- 日志查询
- 日志归档
- 日志分析
- 日志告警

### 1.2 不做什么
- 不实现分布式日志收集
- 不实现复杂的日志分析算法
- 不实现实时日志流

### 1.3 输入
- 日志配置：config/log.yaml
- 日志输出：控制台、文件、消息总线
- 日志级别：DEBUG、INFO、WARN、ERROR、FATAL

### 1.4 输出
- 日志文件：logs/app.log
- 日志查询结果
- 日志统计数据
- 日志告警通知

### 1.5 前置依赖
- ✅ phase3 完成：idcu-log 核心库
- ✅ phase3 完成：idcu-msgbus 消息总线
- ✅ 5.1 完成：core-module

---

## 2. 技术实现方案

### 2.1 核心选型
- 日志核心：idcu-log
- 消息总线：idcu-msgbus
- 存储：idcu-storage
- JSON：idcu-json

### 2.2 核心逻辑
```
1. 初始化日志模块配置
2. 创建日志输出目标
3. 设置日志格式
4. 添加日志过滤器
5. 启动日志模块
6. 接收日志输入
7. 过滤和格式化日志
8. 输出到多个目标
9. 定期轮转日志
10. 支持日志查询和统计
```

### 2.3 数据结构/接口
```c
typedef enum {
    IDCU_LOG_OUTPUT_CONSOLE = 0,
    IDCU_LOG_OUTPUT_FILE,
    IDCU_LOG_OUTPUT_SYSLOG,
    IDCU_LOG_OUTPUT_NETWORK,
    IDCU_LOG_OUTPUT_MSGBUS
} idcu_LogOutputType;

typedef struct {
    char name[128];
    idcu_LogOutputType type;
    idcu_LogLevel min_level;
    idcu_LogLevel max_level;
    // ... 其他配置
} idcu_LogOutput;

typedef struct {
    idcu_LogModuleId id;
    idcu_Vector outputs;
    idcu_Vector filters;
    idcu_Vector log_buffer;
    // ... 其他字段
} idcu_LogModule;

int idcu_log_module_init(idcu_LogModule* lm, const char* name);
int idcu_log_module_start(idcu_LogModule* lm);
void idcu_log_module_log(idcu_LogModule* lm, idcu_LogLevel level, ...);
int idcu_log_module_query(idcu_LogModule* lm, const idcu_LogQuery* query, idcu_Vector* results);
```

### 2.4 跨平台适配
- 文件路径：Windows 使用反斜杠，Linux 使用正斜杠
- 系统日志：Windows 使用 EventLog，Linux 使用 syslog
- 统一的日志接口

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以记录日志到多个输出目标
- [ ] 日志级别过滤正常工作
- [ ] 日志轮转功能正常
- [ ] 日志查询功能正常
- [ ] 日志统计功能正常
- [ ] 消息总线输出正常

### 3.2 性能验收
- [ ] 单条日志写入时间 ≤ 1ms
- [ ] 支持并发写入 ≥ 1000 QPS
- [ ] 日志查询响应时间 ≤ 100ms（1000条）
- [ ] 内存占用 ≤ 10MB

### 3.3 异常验收
- [ ] 日志文件写入失败不影响程序运行
- [ ] 磁盘空间不足时有告警
- [ ] 日志缓冲区满时可以丢弃旧日志

---

## 4. 执行计划

### 4.1 工期
3 小时

### 4.2 里程碑
- D1：完成日志模块接口定义
- D1：完成核心日志功能实现
- D1：完成日志查询和统计
- D1：完成测试和验证

### 4.3 人力
1 人（技能要求：C语言 + 文件操作）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐 .clang-format 规范
- 函数名小写+下划线
- 结构体前缀 idcu_

### 5.2 测试要求
- 单元测试覆盖率 ≥ 75%
- 测试覆盖所有日志级别
- 测试覆盖所有输出类型

### 5.3 部署指引
- 编译命令：cmake --build build
- 模块路径：modules/log-module/
- 日志路径：logs/

---

## 6. 风险与应对

### 6.1 风险1
描述：日志写入影响主程序性能  
应对：使用异步日志写入，设置合理的缓冲区大小

### 6.2 风险2
描述：日志文件过大导致磁盘空间不足  
应对：实现日志轮转，设置文件大小限制和文件数量限制

---

## 7. 详细实现步骤

### 1. 创建目录结构
```bash
mkdir -p modules/log-module/include/idcu/log_module
mkdir -p modules/log-module/src/idcu/log_module
mkdir -p modules/log-module/tests
mkdir -p modules/log-module/examples
```

### 2. 创建日志业务模块头文件
创建 `modules/log-module/include/idcu/log_module/log_module.h`

### 3. 创建 CMakeLists.txt
创建 `modules/log-module/CMakeLists.txt`

### 4. 创建模块配置文件
创建 `modules/log-module/module.yaml`

### 5. 创建 README.md
创建 `modules/log-module/README.md`

### 6. 实现日志模块功能
- 日志输出管理
- 日志过滤
- 日志格式化
- 日志轮转
- 日志查询
- 日志统计
- 日志告警

---

## 8. 验证检查清单

- [ ] 日志业务模块头文件已创建
- [ ] 日志业务模块实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以记录日志到多个输出
- [ ] 日志查询正常工作
- [ ] 日志告警正常工作

---

## 9. Git 提交

```bash
git add modules/log-module/
git commit -m "feat: add log-module module

- Add unified log interface
- Add multi-level logging
- Add multiple output targets
- Add log rotation
- Add log filtering
- Add log query
- Add log archive
- Add log analysis
- Add log alert
- Add message bus integration
- Add CMake build configuration
- Add module.yaml metadata"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 日志未输出 | 输出被禁用 | 确保输出已启用 |
| 日志级别不对 | 级别设置错误 | 检查日志级别配置 |
| 轮转不工作 | 文件权限问题 | 检查文件权限 |
