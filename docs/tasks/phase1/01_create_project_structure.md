# 任务 1.1: 创建项目目录结构

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的、可扩展的项目目录结构，为后续所有模块开发打下基础。创建完成后应包含：应用程序目录、配置目录、文档目录、脚本目录、库目录、模块目录、测试目录等标准结构。

### 1.2 不做什么
- 不创建具体的源代码文件（仅目录结构和初始配置文件）
- 不实现任何业务逻辑
- 不配置复杂的构建系统（仅创建基础框架）

### 1.3 输入
- 无（从零开始创建）

### 1.4 输出
- 完整的目录结构
- .gitignore 文件
- README.md 项目总览文件
- 空的 CMakeLists.txt 占位文件

### 1.5 前置依赖
- Git 已安装（用于初始化仓库）
- 当前工作目录已准备好

---

## 2. 技术实现方案

### 2.1 核心选型
- 使用标准的 Unix/Linux 风格目录结构
- Git 作为版本控制系统
- Markdown 作为文档格式

### 2.2 核心逻辑
```
1. 创建根目录（如需要）
2. 初始化 Git 仓库（如需要）
3. 创建标准目录结构树
4. 创建 .gitignore 排除文件
5. 创建 README.md 项目说明
6. 创建空的 CMakeLists.txt
```

### 2.3 数据结构/接口
目录结构定义：
```
idcu-agent/
├── app/                 # 应用程序入口
├── config/              # 配置文件
│   └── default/        # 默认配置
├── docs/                # 文档
│   ├── api/            # API 文档
│   └── design/         # 设计文档
├── libs/                # 独立库
│   └── idcu-module-build/  # 模块构建系统
├── modules/             # 模块
│   ├── core/           # 核心模块
│   ├── integrations/   # 集成模块
│   └── business/       # 业务模块
├── scripts/             # 脚本
│   ├── windows/        # Windows 脚本
│   └── linux/          # Linux 脚本
├── tests/               # 测试
│   ├── unit/           # 单元测试
│   ├── integration/    # 集成测试
│   ├── benchmarks/     # 性能测试
│   └── data/           # 测试数据
├── build/               # 构建输出（gitignore）
└── out/                 # 其他输出（gitignore）
```

### 2.4 跨平台适配
- 使用 `mkdir -p`（Linux）或 `mkdir`（Windows）创建目录
- .gitignore 文件同时支持 Windows 和 Linux 系统文件
- 目录分隔符使用正斜杠 `/`，Git 会自动处理

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 所有目录已创建（共 18+ 个目录）
- [ ] .gitignore 已创建，包含标准排除规则
- [ ] README.md 已创建，包含项目结构说明
- [ ] 可以使用 `tree -L 2`（Linux）或 `dir /s`（Windows）查看完整目录结构
- [ ] Git 仓库已初始化（如需要）

### 3.2 性能验收
- 目录创建过程耗时 ≤ 10 秒

### 3.3 异常验收
- [ ] 目录已存在时不会报错（使用 `mkdir -p`）
- [ ] 权限不足时有明确的错误提示

---

## 4. 执行计划

### 4.1 工期
0.5 小时/人

### 4.2 里程碑
- D1-00: 完成目录结构创建
- D1-05: 完成配置文件创建
- D1-10: 完成验证和提交

### 4.3 人力
1 人（技能要求：基础命令行操作）

---

## 5. 工程化要求

### 5.1 编码规范
- 目录命名使用小写字母和连字符（kebab-case）
- 文档使用 Markdown 格式，遵循项目文档规范

### 5.2 测试要求
- 手动验证所有目录存在
- 验证 .gitignore 排除规则正确

### 5.3 部署指引
- 无特殊部署要求，目录创建即完成

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：权限不足，无法创建目录  
应对：使用管理员权限或修改父目录权限

### 6.2 风险2
描述：某些目录已存在，可能与预期不符  
应对：检查现有目录结构，确认是否需要清理

### 6.3 风险3
描述：.gitignore 配置错误，导致敏感文件被提交  
应对：仔细检查 .gitignore 内容，使用标准模板，并进行预提交验证

### 6.4 风险4
描述：目录权限设置不当，导致后续操作失败  
应对：使用适当的权限（755 目录，644 文件），并验证权限设置

---

## 7. 详细实现步骤

### 1. 创建根目录（如需要）

```bash
# 如果还没有项目目录，先创建
mkdir -p idcu-agent
cd idcu-agent

# 初始化 Git 仓库（如果还没有）
git init
```

### 2. 创建完整的目录结构

```bash
# 应用程序目录
mkdir -p app

# 配置文件目录
mkdir -p config
mkdir -p config/default

# 文档目录
mkdir -p docs
mkdir -p docs/api
mkdir -p docs/design

# 脚本目录
mkdir -p scripts
mkdir -p scripts/windows
mkdir -p scripts/linux

# 库目录
mkdir -p libs
mkdir -p libs/idcu-module-build
mkdir -p libs/idcu-module-build/include
mkdir -p libs/idcu-module-build/src
mkdir -p libs/idcu-module-build/tests

# 模块目录
mkdir -p modules
mkdir -p modules/core
mkdir -p modules/integrations
mkdir -p modules/business

# 测试目录
mkdir -p tests
mkdir -p tests/unit
mkdir -p tests/integration
mkdir -p tests/benchmarks
mkdir -p tests/data

# 构建输出目录（会被 .gitignore 忽略）
mkdir -p build
mkdir -p out
```

### 3. 创建初始文件

#### 创建 .gitignore

```bash
cat > .gitignore << 'EOF'
# Build directories
build/
out/
bin/
lib/

# IDE files
.idea/
.vscode/
*.swp
*.swo
*~

# OS files
.DS_Store
Thumbs.db

# Temporary files
*.tmp
*.bak
*.old

# Log files
*.log
logs/
EOF
```

#### 创建 README.md（项目总览）

```bash
cat > README.md << 'EOF'
# IDCU Agent

基于微内核架构的实时代理程序。

## 快速开始

详见 [docs/step_by_step_development_guide.md](docs/step_by_step_development_guide.md)

## 项目结构

```
idcu-agent/
├── app/                 # 应用程序入口
├── config/              # 配置文件
├── docs/                # 文档
├── libs/                # 独立库
├── modules/             # 模块
├── scripts/             # 构建脚本
└── tests/               # 测试
```
EOF
```

#### 创建空的 CMakeLists.txt（稍后填充）

```bash
touch CMakeLists.txt
```

---

## 8. 验证检查清单

- [ ] 所有目录都已创建
- [ ] .gitignore 已创建，包含标准排除规则
- [ ] README.md 已创建，包含项目结构说明
- [ ] 可以使用 `tree -L 2` 查看目录结构（Linux）或 `dir /s`（Windows）
- [ ] 代码符合工程化标准（见[工程化标准与CI/CD指南](../reference/engineering_standards.md)）
- [ ] Git 仓库已初始化（如需要）
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add .
git commit -m "chore: initialize project structure

- Create complete directory structure
- Add .gitignore
- Add initial README.md"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 权限不足 | 当前用户没有创建目录的权限 | 使用管理员权限或修改目录权限 |
| 目录已存在 | 某些目录已经存在 | 检查是否需要删除或重新创建 |
