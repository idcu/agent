# IDCU Agent 开发工作流程指南

本文档介绍 IDCU Agent 项目的推荐开发工作流程。

## 目录

- [开发前准备](#开发前准备)
- [日常开发流程](#日常开发流程)
- [代码提交前检查](#代码提交前检查)
- [分支策略](#分支策略)
- [代码审查](#代码审查)

---

## 开发前准备

### 1. 环境设置

确保你的开发环境已配置好：

**必需工具：**
- CMake (3.15+)
- 编译器 (MSVC/GCC/Clang)
- Git

**推荐工具：**
- LLVM/Clang (用于 clang-format 和 clang-tidy)
- Visual Studio Code / CLion / Visual Studio

### 2. 克隆项目

```bash
git clone https://github.com/your-org/idcu-agent.git
cd idcu-agent
```

### 3. 初始构建

```bash
# Windows
scripts\build.bat Debug

# Linux/macOS
scripts/build.sh Debug
```

---

## 日常开发流程

### 1. 创建功能分支

```bash
git checkout main
git pull origin main
git checkout -b feature/your-feature-name
```

### 2. 编码

按照项目的编码规范进行开发：

- 遵循 `.clang-format` 定义的代码风格
- 使用有意义的变量和函数名
- 添加必要的注释
- 编写单元测试

### 3. 定期格式化代码

在编写代码过程中，定期运行格式化：

```bash
# Windows
format_all.bat

# 或使用项目脚本
scripts\format.bat

# Linux/macOS
scripts/format.sh
```

### 4. 构建和测试

确保代码可以正常编译和通过测试：

```bash
# 配置项目（启用测试）
cmake -B build -DBUILD_TESTS=ON

# 构建
cmake --build build --config Debug

# 运行测试
cd build
ctest --output-on-failure
```

### 5. 运行静态分析（可选但推荐）

```bash
# Windows
scripts\run_clang_tidy.bat

# Linux/macOS
scripts/run_clang_tidy.sh
```

---

## 代码提交前检查

在提交代码前，请确保完成以下检查：

### ✅ 检查清单

- [ ] 代码已格式化（运行 `format.bat`）
- [ ] 代码可以正常编译
- [ ] 所有测试通过
- [ ] 没有新增的编译警告
- [ ] 静态分析没有发现严重问题
- [ ] 更新了相关文档（如需要）
- [ ] 添加了单元测试（如需要）

### 快速检查命令

```bash
# 1. 检查格式（不修改文件）
scripts\check_format.bat

# 2. 构建项目
cmake -B build -DBUILD_TESTS=ON
cmake --build build --config Debug

# 3. 运行测试
cd build
ctest --output-on-failure
```

---

## 分支策略

项目采用 Git Flow 风格的分支策略：

### 主要分支

- **main** - 稳定的生产版本
- **develop** - 开发集成分支

### 功能分支

- 命名格式：`feature/描述性名称`
- 从 `develop` 分支创建
- 合并回 `develop` 分支

### 发布分支

- 命名格式：`release/vx.y.z`
- 从 `develop` 分支创建
- 用于准备发布版本

### 热修复分支

- 命名格式：`hotfix/描述性名称`
- 从 `main` 分支创建
- 合并回 `main` 和 `develop` 分支

### 重构分支

- 命名格式：`refactor/描述性名称`
- 用于大规模重构工作

---

## 代码审查

### 提交 Pull Request

1. 推送你的分支到远程仓库
2. 创建 Pull Request 到 `develop` 分支
3. 填写 PR 模板，包括：
   - 更改的描述
   - 相关的 Issue（如有）
   - 测试结果
   - 截图（如适用）

### PR 审查清单

审查者应该检查：

- [ ] 代码符合项目风格
- [ ] 没有明显的 bug
- [ ] 测试覆盖充分
- [ ] 文档已更新
- [ ] 没有安全问题
- [ ] 性能考虑合理

---

## 常见开发场景

### 添加新功能

1. 创建功能分支
2. 实现功能
3. 编写测试
4. 格式化代码
5. 运行测试
6. 提交 PR

### 修复 Bug

1. 创建 hotfix 或 feature 分支
2. 复现 bug
3. 修复 bug
4. 添加回归测试
5. 验证修复
6. 提交 PR

### 重构代码

1. 创建 refactor 分支
2. 进行重构
3. 确保所有测试通过
4. 运行性能测试（如需要）
5. 提交 PR

---

## 调试技巧

### 使用 AddressSanitizer

**注意：** 需要 GCC 或 Clang 编译器

```bash
# Windows (MinGW)
scripts\build_with_asan.bat

# Linux/macOS
scripts/build_with_asan.sh
```

### 使用 ThreadSanitizer

```bash
# Windows (MinGW)
scripts\build_with_tsan.bat

# Linux/macOS
scripts/build_with_tsan.sh
```

### 生成覆盖率报告

**注意：** 需要 GCC 或 Clang，仅支持 Unix-like 系统

```bash
cmake -B build -DBUILD_TESTS=ON -DENABLE_CODE_COVERAGE=ON
cmake --build build
cd build
make coverage
```

---

## 性能基准测试

### 运行基准测试

```bash
cmake -B build -DBUILD_BENCHMARKS=ON
cmake --build build --config Release

# 运行基准测试
cd build/output/Release/bin
./idcu_benchmark
```

### 比较性能结果

项目使用 `scripts/compare_benchmarks.py` 来比较基准测试结果：

```bash
python scripts/compare_benchmarks.py baseline.json new.json
```

---

## 获得帮助

如果遇到问题：

1. 查看 `docs/` 目录下的文档
2. 检查 `CODE_QUALITY_TOOLS.md` 了解工具使用
3. 查看 GitHub Issues 中是否有类似问题
4. 创建新的 Issue 描述你的问题

---

## 参考资源

- [项目 README](./README.md)
- [代码质量工具指南](./CODE_QUALITY_TOOLS.md)
- [快速入门指南](./docs/quick_start.md)
- [调试指南](./docs/debugging_guide.md)
- [常见问题 FAQ](./docs/faq.md)

---

**祝开发愉快！** 🚀
