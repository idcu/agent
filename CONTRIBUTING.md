# IDCU Agent 贡献指南

感谢你有兴趣为 IDCU Agent 项目做出贡献！这篇指南将帮助你了解如何加入我们的开发社区。

---

## 目录

1. [行为准则](#行为准则)
2. [如何贡献](#如何贡献)
3. [开发环境设置](#开发环境设置)
4. [代码规范](#代码规范)
5. [Issue 和 PR 流程](#issue-和-pr-流程)
6. [提交流程](#提交流程)
7. [报告问题](#报告问题)
8. [获取帮助](#获取帮助)

---

## 行为准则

参与本项目的所有贡献者都需要遵守我们的 [行为准则](./CODE_OF_CONDUCT.md)。请务必阅读并遵守。

---

## 如何贡献

### 1. 找到可以贡献的地方

- 查看 [Issues](../../issues) 列表，寻找标记为 `good first issue` 或 `help wanted` 的问题
- 查看 [DEVELOPMENT_PLAN.md](./DEVELOPMENT_PLAN.md) 中的任务列表
- 改进文档
- 添加测试用例
- 报告 Bug
- 提出新功能建议

### 2. 贡献类型

#### 🐛 修复 Bug
- 确保有对应的 Issue 描述问题
- 提供清晰的复现步骤
- 编写测试用例验证修复

#### ✨ 添加新功能
- 先在 Issues 中讨论，确认功能需求
- 遵循项目的架构设计
- 编写完整的文档和测试

#### 📚 改进文档
- 修正拼写和语法错误
- 补充缺失的说明
- 改进示例代码
- 添加教程

#### 🧪 添加测试
- 单元测试
- 集成测试
- 提高代码覆盖率

---

## 开发环境设置

### 前置要求

#### Windows
- MinGW-w64 或 Visual Studio (带 CMake 支持)
- Git
- CMake 3.10+

#### Linux
- GCC 或 Clang
- Git
- CMake 3.10+
- Make

### 克隆仓库

```bash
git clone https://github.com/your-username/idcu-agent.git
cd idcu-agent
```

### 构建项目

#### Windows
```bash
.\build.bat
```

#### Linux
```bash
./build.sh
```

### 运行测试

```bash
cd build
ctest --output-on-failure
```

---

## 代码规范

### 命名规范

- **文件名**: 全小写，下划线分隔，如 `micro_kernel.c`
- **公共函数**: `idcu_` 前缀，全小写，下划线分隔，如 `idcu_kernel_init()`
- **宏定义**: `IDCU_` 前缀，全大写，下划线分隔，如 `IDCU_MODULE_VERSION()`
- **全局变量**: `g_` 前缀，如 `g_running`
- **静态变量**: `s_` 前缀（可选）
- **类型定义**: `idcu_TypeName` 格式，如 `idcu_Mutex`

### 代码风格

项目使用 Clang-Format 进行代码格式化。提交前请运行：

```bash
clang-format -i 你的文件.c
```

主要风格规则：
- 大括号换行
- 指针靠近类型 (`int* ptr`)
- 运算符两边加空格
- 逗号后面加空格
- 缩进 4 个空格

### 注释规范

- 函数注释：说明功能、参数、返回值
- 复杂逻辑：说明为什么这么做
- 代码本身能说明的不用注释

---

## Issue 和 PR 流程

我们制定了详细的 Issue 和 Pull Request 处理流程，以确保项目协作的顺畅高效。

### 详细流程文档

请查看 [Issue 和 PR 处理流程文档](./docs/issue_pr_workflow.md) 了解完整的处理流程，包括：
- 标签使用规范
- Issue 从创建到关闭的完整流程
- PR 从创建到合并的完整流程
- 维护者和贡献者的职责分工

### Issue 模板

我们提供了以下 Issue 模板：
- 🐛 **Bug 报告** - 用于报告 Bug
- ✨ **功能建议** - 用于提出新功能或改进
- 📚 **文档问题** - 用于报告文档相关问题
- ❓ **问题/咨询** - 用于提问和获取帮助

请选择合适的模板并完整填写所有必填字段，这将帮助我们更快地理解和处理你的问题。

---

## 提交流程

### 1. 创建分支

请使用有意义的分支名称：

```bash
# 新功能
git checkout -b feature/简洁的功能描述

# Bug 修复
git checkout -b fix/简洁的问题描述

# 文档更新
git checkout -b docs/更新内容描述

# 代码重构
git checkout -b refactor/重构内容描述
```

### 2. 提交更改

```bash
git add .
git commit -m "简洁明了的提交信息"
```

**提交信息格式**（遵循 Conventional Commits 规范）：

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Type 类型**：
- `feat`: 添加新功能
- `fix`: 修复 Bug
- `docs`: 更新文档
- `test`: 添加/修改测试
- `refactor`: 重构代码（不改变功能）
- `style`: 代码格式化（不改变代码逻辑）
- `chore`: 构建/工具相关

**Scope 范围**（可选）：
- `kernel`: 微内核相关
- `module`: 模块系统相关
- `network`: 网络相关
- `security`: 安全相关
- 等等...

**示例**：
```
feat(kernel): 添加优雅退出信号处理

- 实现 SIGINT 信号捕获
- 确保所有模块的 stop() 被调用
- 添加单元测试

Closes #123
```

### 3. 推送分支

```bash
git push origin feature/你的功能名
```

### 4. 创建 Pull Request

1. 前往 GitHub 仓库
2. 点击 "Compare & pull request"
3. **完整填写** [PR 模板](./.github/PULL_REQUEST_TEMPLATE.md)：
   - 选择变更类型
   - 描述更改内容
   - 关联相关 Issue（使用 `Closes #XXX` 或 `Fixes #XXX`）
   - 完成检查清单
   - 说明测试方法
4. 等待代码审查

**更多详细信息**：请参阅 [Issue 和 PR 处理流程文档](./docs/issue_pr_workflow.md)

### 5. 代码审查

- 维护者通常会在 24-48 小时内进行审查
- 请及时回应审查意见
- 修改后重新提交并推送
- 确保所有 CI 检查通过
- 至少需要 1 名维护者批准才能合并

---

## 报告问题

我们使用 GitHub Issues 来跟踪所有问题和功能建议。

### 选择合适的 Issue 模板

在创建 Issue 时，请选择最适合的模板：

- 🐛 **Bug 报告** - 如果发现了 Bug 或程序崩溃
- ✨ **功能建议** - 如果想提出新功能或改进现有功能
- 📚 **文档问题** - 如果发现文档有误或需要改进
- ❓ **问题/咨询** - 如果有问题需要帮助

**重要**：请在创建 Issue 前先搜索现有 Issues，看看是否已经有类似问题被报告或讨论。

### Bug 报告

在报告 Bug 时，我们的 [Bug 报告模板](./.github/ISSUE_TEMPLATE/bug_report.yml) 会引导你提供以下信息：

1. **环境信息**
   - 操作系统版本
   - 编译器版本
   - CMake 版本
   - 项目版本/Commit Hash

2. **复现步骤**
   - 清晰的步骤说明
   - 最小复现代码（如果可能）

3. **预期行为**
   - 你期望发生什么

4. **实际行为**
   - 实际发生了什么
   - 错误日志或截图

### 功能建议

在提出新功能建议时，我们的 [功能建议模板](./.github/ISSUE_TEMPLATE/feature_request.yml) 会引导你提供：

1. **功能描述**
   - 这个功能解决什么问题
   - 为什么需要这个功能

2. **建议的解决方案**
   - 你希望看到什么样的解决方案

3. **使用场景**
   - 具体的使用例子

4. **优先级评估**
   - 你认为这个功能的优先级如何

**更多详细信息**：请参阅 [Issue 和 PR 处理流程文档](./docs/issue_pr_workflow.md)

---

## 获取帮助

如果你在贡献过程中遇到问题：

1. 查看文档：
   - [README.md](./README.md)
   - [开发者指引](./docs/developer_guide.md)
   - [用户指引](./docs/user_guide.md)
   - [docs/](./docs/) 目录下的文档
   - [教程系列](./docs/)

2. 加入社区：
   - 查看 [社区指南](./docs/community_guide.md) 了解如何参与社区
   - 在 GitHub Discussions 中提问
   - （待建立）加入 Discord/Slack 实时聊天

3. 创建 Issue 提问

4. 在 PR 中讨论

---

## 开发者认证

通过以下步骤可以成为项目的贡献者：

1. 提交 1-2 个简单的 PR（修复 typo、改进文档等）
2. 提交 1 个有意义的 PR（修复 Bug 或添加小功能）
3. 持续参与代码审查和讨论

---

## 感谢

感谢所有为 IDCU Agent 项目做出贡献的人！🎉

---

**最后更新**: 2026-04-04
