# 任务完成标准流程

**每个开发任务完成后，必须按照以下流程操作：**

## 任务完成检查清单

在宣布任务完成前，请确认：

- [ ] 功能实现完整，满足需求
- [ ] **先写了测试**（TDD！）
- [ ] 代码已通过 `clang-format` 格式化
- [ ] 代码可以正常编译，无警告
- [ ] 所有相关单元测试通过
- [ ] 集成测试通过（如适用）
- [ ] `clang-tidy` 静态分析通过
- [ ] 已添加必要的注释和文档
- [ ] 已更新相关的 README 和开发文档
- [ ] 临时文件已清理

## 标准操作流程

### 步骤 1: 运行测试和检查

```bash
# 1. 格式化代码
scripts/format.bat          # Windows
scripts/format.sh           # Linux

# 2. 检查格式（可选，用于验证）
scripts/check_format.bat    # Windows
scripts/check_format.sh     # Linux

# 3. 构建项目（启用测试）
cmake -B build -DBUILD_TESTS=ON
cmake --build build --config Debug

# 4. 运行测试
cd build
ctest --output-on-failure

# 5. 运行静态分析（可选但推荐）
cd ..
scripts/run_clang_tidy.bat  # Windows
scripts/run_clang_tidy.sh   # Linux
```

### 步骤 2: 清理临时文件

```bash
# 删除构建目录
rm -rf build/
rm -rf out/

# 删除备份文件（如适用）
find . -name "*~" -delete
find . -name "*.bak" -delete

# 确保没有提交不必要的文件
# 检查 .gitignore 是否包含了所有应该忽略的文件
```

### 步骤 3: 更新开发文档

- 更新本文档（`step_by_step_development_guide.md`）中的进度
- 如添加了新模块，更新项目架构图
- 更新相关模块的 README.md
- 如果有 API 变更，更新 API 文档

### 步骤 4: Git 提交

```bash
# 1. 查看变更
git status
git diff

# 2. 添加变更
git add .

# 3. 提交（使用规范的 commit message）
git commit -m "feat(scope): description of change

- Detailed change 1
- Detailed change 2

Closes #issue-number"

# 4. 推送到远程（如果是功能分支）
git push origin feature/your-feature-name
```

### 步骤 5: 创建 Pull Request（如适用）

如果是团队协作：
1. 推送到远程功能分支
2. 在 GitHub/GitLab 上创建 Pull Request
3. 填写 PR 模板
4. 等待代码审查
5. 根据审查意见修改
6. 合并到 develop 分支
