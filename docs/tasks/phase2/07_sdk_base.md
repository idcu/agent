# 任务 2.7: SDK 基础

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建软件开发工具包（SDK），简化模块开发，支持模块生命周期管理封装、日志接口封装、配置接口封装、消息发送/接收接口封装、模块定义宏。

### 1.2 不做什么
- 不实现图形化开发工具
- 不实现代码生成器
- 不实现远程调试支持
- 不实现性能分析工具

### 1.3 输入
- idcu-common 库（任务 2.1）
- 微内核（任务 2.6）
- idcu-module-build（任务 2.2）

### 1.4 输出
- 完整的 SDK 库
- 简化的模块开发 API
- 模块定义宏
- 示例代码

### 1.5 前置依赖
- 任务 2.1、2.2、2.6 已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **API 设计**: 简洁易用的 C API
- **封装层次**: 薄封装层
- **宏定义**: 简化模块定义
- **错误处理**: 统一错误码

### 2.2 核心逻辑
```
1. SDK 上下文
   ├── 封装微内核
   ├── 保存用户数据
   └── 提供访问接口

2. 日志封装
   ├── debug/info/warn/error 级别
   ├── 格式化输出
   └── 转发到日志系统

3. 配置封装
   ├── 字符串/整数/布尔配置
   ├── 类型安全访问
   └── 默认值支持

4. 消息封装
   ├── 简化发布接口
   ├── 简化订阅接口
   └── 主题管理

5. 模块生命周期封装
   ├── 简化 init/start/stop/destroy
   ├── 自动 SDK 上下文传递
   └── 模块定义宏
```

### 2.3 数据结构/接口
```c
// SDK 上下文
typedef struct idcu_SdkContext idcu_SdkContext;

// 日志接口
void idcu_sdk_log_debug(idcu_SdkContext* ctx, const char* fmt, ...);
void idcu_sdk_log_info(idcu_SdkContext* ctx, const char* fmt, ...);
void idcu_sdk_log_warn(idcu_SdkContext* ctx, const char* fmt, ...);
void idcu_sdk_log_error(idcu_SdkContext* ctx, const char* fmt, ...);

// 配置接口
idcu_ErrorCode idcu_sdk_get_config_string(idcu_SdkContext* ctx, const char* key, const char** out_value);
idcu_ErrorCode idcu_sdk_get_config_int(idcu_SdkContext* ctx, const char* key, int* out_value);
idcu_ErrorCode idcu_sdk_get_config_bool(idcu_SdkContext* ctx, const char* key, bool* out_value);

// 模块定义宏
#define IDCU_SDK_MODULE_DEFINE(name, ver, desc, init_fn, start_fn, stop_fn, destroy_fn)
```

### 2.4 跨平台适配
- 无特殊跨平台需求，依赖底层组件

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] SDK 上下文创建/销毁功能正常
- [ ] 模块生命周期封装正常
- [ ] 日志接口功能正常
- [ ] 配置接口功能正常
- [ ] 消息接口功能正常
- [ ] 模块定义宏工作正常

### 3.2 性能验收
- SDK 函数调用开销 ≤ 1μs
- 模块定义宏无运行时开销
- 日志格式化性能合理

### 3.3 异常验收
- [ ] NULL 参数检查正确
- [ ] 配置不存在返回错误
- [ ] 消息发送失败返回错误

---

## 4. 执行计划

### 4.1 工期
1 天/人

### 4.2 里程碑
- D8-01: 完成 SDK 上下文
- D8-03: 完成日志和配置封装
- D8-05: 完成消息封装和模块定义宏
- D8-06: 完成测试和示例

### 4.3 人力
1 人（技能要求：C 语言、API 设计）

---

## 5. 工程化要求

### 5.1 编码规范
- 遵循项目 .clang-format 规范

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 提供完整示例代码

### 5.3 部署指引
- 库文件: modules/core/sdk/

---

## 6. 风险与应对

### 6.1 风险1
描述：API 设计不够简洁，使用不便  
应对：早期反馈，多次迭代，提供充分示例

### 6.2 风险2
描述：宏定义复杂，容易出错  
应对：提供详细文档和示例，充分测试

---

## 7. 详细实现步骤

（保留原文档的详细实现步骤内容）

---

## 8. 验证检查清单

- [ ] SDK 上下文创建/销毁功能正常
- [ ] 模块生命周期封装正常
- [ ] 日志接口功能正常
- [ ] 配置接口功能正常
- [ ] 消息接口功能正常
- [ ] 模块定义宏工作正常
- [ ] 代码已格式化（clang-format）
- [ ] 静态分析通过（clang-tidy）
- [ ] 所有单元测试通过
- [ ] YAML 配置示例已创建
- [ ] README.md 已创建

---

## 9. Git 提交

```bash
git add modules/core/sdk/
git add config/default/sdk.yaml
git commit -m "feat(core): add SDK

- Add SDK header with module lifecycle macros
- Add SDK implementation
- Add unit tests (TDD)
- Add example code
- Add CMakeLists.txt using idcu-module-build
- Add YAML config example
- Add README"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 模块定义不工作 | 宏使用错误 | 检查宏参数格式 |
| 日志不输出 | SDK 上下文未初始化 | 确保先创建 SDK 上下文 |
| 配置读取失败 | 配置文件不存在 | 检查配置文件路径 |

