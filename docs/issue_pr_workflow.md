# Issue 和 Pull Request 处理流程

本文档详细说明了 IDCU Agent 项目中 Issue 和 Pull Request (PR) 的处理流程和规范。

---

## 目录

1. [标签规范](#标签规范)
2. [Issue 处理流程](#issue-处理流程)
3. [Pull Request 处理流程](#pull-request-处理流程)
4. [维护者指南](#维护者指南)
5. [贡献者指南](#贡献者指南)

---

## 标签规范

### 类型标签

| 标签 | 说明 |
|------|------|
| `bug` | Bug 报告 |
| `enhancement` | 功能请求/改进 |
| `documentation` | 文档相关 |
| `question` | 问题/咨询 |
| `test` | 测试相关 |
| `refactor` | 代码重构 |
| `performance` | 性能优化 |

### 状态标签

| 标签 | 说明 |
|------|------|
| `needs-triage` | 需要分类（新 Issue/PR 的初始状态） |
| `needs-info` | 需要更多信息 |
| `needs-review` | 需要代码审查 |
| `in-progress` | 正在处理中 |
| `blocked` | 被阻塞 |
| `on-hold` | 暂缓 |

### 优先级标签

| 标签 | 说明 |
|------|------|
| `P0 - Critical` | 紧急，需要立即处理 |
| `P1 - High` | 高优先级 |
| `P2 - Medium` | 中优先级 |
| `P3 - Low` | 低优先级 |

### 其他标签

| 标签 | 说明 |
|------|------|
| `good first issue` | 适合新手的 Issue |
| `help wanted` | 需要帮助 |
| `duplicate` | 重复 Issue |
| `wontfix` | 不会修复 |
| `invalid` | 无效的 Issue |

---

## Issue 处理流程

### 1. Issue 创建（贡献者）

贡献者创建 Issue 时：

1. 选择合适的 Issue 模板：
   - Bug 报告 → 使用 `bug_report.yml`
   - 功能建议 → 使用 `feature_request.yml`
   - 文档问题 → 使用 `documentation.yml`
   - 问题/咨询 → 使用 `question.yml`

2. 完整填写模板中的所有必填字段

3. 自动分配标签：`needs-triage` 和对应类型标签

### 2. Issue 分类（维护者）

维护者在 24-48 小时内进行分类：

1. 检查 Issue 是否完整
   - 信息不足 → 添加 `needs-info` 标签，请求补充信息
   - 重复 Issue → 添加 `duplicate` 标签，关闭并引用原 Issue
   - 无效 Issue → 添加 `invalid` 标签，关闭并说明原因

2. 分配优先级标签（P0-P3）

3. 分配状态标签（如适用）：
   - 适合新手 → 添加 `good first issue`
   - 需要帮助 → 添加 `help wanted`

4. 移除 `needs-triage` 标签

5. 分配给合适的维护者（如适用）

### 3. Issue 处理

**Bug 修复流程：**

1. 确认 Bug 可复现
2. 添加 `in-progress` 标签
3. 修复 Bug
4. 编写测试用例验证
5. 创建 PR 关联 Issue
6. PR 合并后关闭 Issue

**功能开发流程：**

1. 讨论并确认功能需求
2. 添加 `in-progress` 标签
3. 实现功能
4. 编写文档和测试
5. 创建 PR 关联 Issue
6. PR 合并后关闭 Issue

### 4. Issue 关闭

Issue 可以通过以下方式关闭：

1. PR 合并时自动关闭（使用 `Closes #XXX` 或 `Fixes #XXX`）
2. 维护者手动关闭（需要说明原因）
3. 贡献者自行关闭

---

## Pull Request 处理流程

### 1. PR 创建（贡献者）

贡献者创建 PR 时：

1. 确保基于最新的 `main` 分支
2. 使用有意义的分支名：
   - `feature/功能描述`
   - `fix/问题描述`
   - `docs/文档更新`
   - `refactor/重构描述`

3. 完整填写 PR 模板

4. 提交信息遵循约定格式：
   ```
   <type>(<scope>): <subject>
   
   <body>
   
   <footer>
   ```
   - type: feat, fix, docs, test, refactor, style, chore
   - scope: 可选，如 kernel, module, network 等

### 2. PR 初审（维护者）

维护者在 24-48 小时内进行初审：

1. 检查 PR 描述是否完整
2. 检查是否关联了相关 Issue
3. 检查 CI 是否通过
   - CI 失败 → 请求修复
4. 分配 `needs-review` 标签
5. 分配给合适的审查者

### 3. 代码审查

审查者进行代码审查：

1. 审查代码质量和规范
2. 检查是否有测试覆盖
3. 检查文档是否更新
4. 提出修改意见（使用 GitHub 的 Review 功能）
5. 批准或请求修改

**审查通过标准：**
- 至少 1 名维护者批准
- 所有 CI 检查通过
- 无未解决的讨论
- 代码符合项目规范

### 4. PR 合并

维护者合并 PR：

1. 选择合适的合并方式：
   - **Squash and Merge**：适用于小功能或 Bug 修复
   - **Rebase and Merge**：适用于有清晰提交历史的 PR
   - **Create a Merge Commit**：适用于大功能或需要保留完整历史的 PR

2. 合并 PR

3. 删除源分支（可选）

4. 关闭相关 Issue（如果 PR 描述中有关联）

---

## 维护者指南

### 日常职责

1. **Issue 分类**：每天查看新 Issue，进行分类和标签管理
2. **PR 审查**：及时审查分配的 PR，提供建设性反馈
3. **社区互动**：回答问题，参与讨论
4. **发布管理**：准备和发布新版本

### 响应时间目标

- **P0 问题**：2 小时内响应，24 小时内解决
- **P1 问题**：24 小时内响应，3 天内解决
- **P2 问题**：48 小时内响应，1 周内解决
- **P3 问题**：1 周内响应，根据进度安排

### 沟通原则

1. **友好和尊重**：始终保持礼貌和专业
2. **清晰明确**：表达要清晰，避免歧义
3. **建设性反馈**：批评时提供改进建议
4. **及时回应**：尽量在合理时间内回应

---

## 贡献者指南

### 提交 PR 前检查清单

- [ ] 代码已通过 `clang-format` 格式化
- [ ] 代码已通过 `clang-tidy` 检查
- [ ] 所有现有测试通过
- [ ] 已添加必要的测试用例
- [ ] 已更新相关文档
- [ ] 提交信息遵循约定格式
- [ ] PR 描述完整填写

### 获得帮助

如果你在贡献过程中遇到问题：

1. 查看文档：README、开发者指引、用户指引
2. 创建 Issue 提问（使用 `question` 模板）
3. 在相关 Issue/PR 中讨论

### 成为维护者

贡献者可以通过以下方式成为维护者：

1. 持续贡献高质量的代码
2. 积极参与代码审查
3. 帮助社区成员解决问题
4. 由现有维护者提名和投票通过

---

**文档版本**：v1.0  
**创建日期**：2026-04-04  
**最后更新**：2026-04-04
