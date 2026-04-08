# 任务 1.1: 创建项目目录结构

## 目标

创建完整的、可扩展的项目目录结构，为后续所有模块开发打下基础。

## 详细步骤

### 1. 创建根目录

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

## 验证检查清单

- [ ] 所有目录都已创建
- [ ] .gitignore 已创建
- [ ] README.md 已创建
- [ ] 可以使用 `tree -L 2` 查看目录结构（Linux）或 `dir /s`（Windows）

## Git 提交

```bash
git add .
git commit -m "chore: initialize project structure

- Create complete directory structure
- Add .gitignore
- Add initial README.md"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 权限不足 | 当前用户没有创建目录的权限 | 使用管理员权限或修改目录权限 |
| 目录已存在 | 某些目录已经存在 | 检查是否需要删除或重新创建 |

## 经验提示

- 参考现有项目结构，但保持更精简
- 注意目录命名的一致性
- 使用 `mkdir -p` 命令可以安全地创建多级目录
