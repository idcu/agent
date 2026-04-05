# 重构贡献指南

> 文档状态: 活跃
> 版本: v1.0
> 创建日期: 2026-04-05

---

## 目录

1. [概述](#1-概述)
2. [代码提交规范](#2-代码提交规范)
3. [开发流程](#3-开发流程)
4. [代码审查规范](#4-代码审查规范)
5. [编译和测试要求](#5-编译和测试要求)
6. [常见问题](#6-常见问题)

---

## 1. 概述

本文档旨在规范 IDCU Agent 模块架构重构期间的开发工作，确保重构过程高效、有序、可追踪。

重构的主要目标是将核心模块提取为可独立复用的库，详细计划请参考 [refactoring_plan.md](./refactoring_plan.md)。

---

## 2. 代码提交规范

### 2.1 分支命名规范

- **主重构分支**: `refactor/module-split`
- **任务分支**: `refactor/task-{编号}-{描述}`，例如：`refactor/task-0.1-setup`、`refactor/task-1.1-common-lib`

### 2.2 提交消息规范

使用以下格式：

```
<type>(<scope>): <subject>

<body>

<footer>
```

**类型（type）**:

- `refactor`: 重构代码
- `feat`: 新功能
- `fix`: 修复bug
- `docs`: 文档更新
- `test`: 测试相关
- `build`: 构建系统相关
- `chore`: 其他杂项

**范围（scope）**:

- `task-0.1`, `task-1.1`, 等：按重构计划的任务编号
- `common`, `log`, `config` 等：按涉及的库或模块

**示例**:

```
refactor(task-1.1): create idcu-common library structure

- Move atomic.h, lock.h, error_code.h to libs/idcu-common/include/idcu/common/
- Move source files to libs/idcu-common/src/
- Add CMakeLists.txt with install targets
```

### 2.3 提交频率

- **小步快跑**：每次提交对应一个小的、可验证的变更
- **每完成一个子步骤提交一次**
- **确保每次提交后代码可以编译**

---

## 3. 开发流程

### 3.1 开始新任务

1. 从 `refactor/module-split` 分支创建任务分支
2. 阅读重构计划中对应任务的详细说明
3. 明确验收标准

### 3.2 开发过程

1. 按照任务步骤执行
2. 每次小变更后提交
3. 定期运行编译和测试
4. 确保无乱码问题（使用 UTF-8 编码）

### 3.3 完成任务

1. 确保所有验收标准满足
2. 完整编译项目，确保无错误
3. 运行所有测试，确保通过
4. 将任务分支合并到 `refactor/module-split`
5. 更新 [refactoring_plan.md](./refactoring_plan.md) 中的任务状态

---

## 4. 代码审查规范

### 4.1 审查清单

- [ ] 代码可以完整编译
- [ ] 所有测试通过
- [ ] 无乱码问题（UTF-8）
- [ ] 符合 `.clang-format` 规范
- [ ] 提交消息符合规范
- [ ] 变更范围合理，没有引入意外的修改
- [ ] 验收标准已满足

### 4.2 代码风格

- 使用 `.clang-format` 自动格式化代码
- 遵循 Google C++ Style Guide（适用于 C）
- 头文件使用 `#pragma once` 或 include guards
- 保持 API 兼容性（除非任务明确说明可以打破）

---

## 5. 编译和测试要求

### 5.1 编译

```bash
# Windows
scripts\build.bat

# Linux/Mac
./scripts/build.sh
```

或使用 CMake 直接构建：

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build build --config Debug
```

### 5.2 测试

```bash
cd build
ctest --output-on-failure --verbose
```

### 5.3 强制要求

- **每次提交前必须确保代码可以完整编译**
- **每次提交前必须运行相关测试并确保通过**
- **重构期间不引入新的 bug**

---

## 6. 常见问题

### 6.1 乱码问题

- 确保所有源文件使用 UTF-8 编码
- Windows 下使用 Visual Studio 时设置 `/utf-8` 编译选项
- Git 配置：`git config --global core.autocrlf true`

### 6.2 兼容性问题

- 重构初期不考虑向后兼容
- 但尽量保持 API 稳定，便于后续迁移

### 6.3 循环依赖

- 严格按照重构计划的依赖关系图开发
- 如有疑问，先讨论再实施

---

**文档结束**

> 如有疑问，请联系项目维护者。
