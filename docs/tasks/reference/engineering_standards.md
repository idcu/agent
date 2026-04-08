# 工程化标准与CI/CD指南

> **文档版本**: v1.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ✅ 已完成

---

## 1. 工程化标准

### 1.1 编码规范

#### C 语言编码规范
- **代码风格**: 遵循项目的 .clang-format 规范
- **命名约定**:
  - 函数名: 小写 + 下划线 (snake_case)
  - 变量名: 小写 + 下划线 (snake_case)
  - 结构体名: 前缀 `idcu_` + 驼峰命名 (Idcu_Example)
  - 枚举名: 前缀 `idcu_` + 大写 + 下划线 (IDCU_ERROR_CODE)
  - 常量名: 大写 + 下划线 (MAX_BUFFER_SIZE)
- **缩进**: 4 个空格
- **行宽**: 不超过 80 字符
- **注释**: 函数、结构体、枚举等都需要详细注释
- **头文件保护**: 使用 `#ifndef` 和 `#define` 防止重复包含

#### 目录结构规范
```
project/
├── CMakeLists.txt          # 根构建配置
├── include/                # 公共头文件
│   └── idcu/               # 命名空间
│       ├── common/         # 通用组件
│       ├── log/            # 日志系统
│       └── ...             # 其他模块
├── src/                    # 源代码
│   └── idcu/               # 命名空间
│       ├── common/         # 通用组件
│       ├── log/            # 日志系统
│       └── ...             # 其他模块
├── tests/                  # 测试代码
├── examples/               # 示例代码
├── scripts/                # 脚本文件
│   ├── linux/              # Linux 脚本
│   └── windows/            # Windows 脚本
└── CMakeModules/           # CMake 模块
```

### 1.2 测试标准

#### 单元测试
- **覆盖率要求**: ≥ 80%
- **测试框架**: 自研轻量级测试框架
- **测试命名**: `test_<模块名>_<功能名>`
- **测试文件**: 与被测试文件放在同一目录下，以 `test_` 前缀命名
- **测试内容**: 包括正常场景、边界条件和异常场景

#### 集成测试
- **测试场景**: 覆盖模块间交互
- **测试环境**: 模拟真实运行环境
- **测试数据**: 使用真实或模拟的测试数据

#### 性能测试
- **测试工具**: 自定义性能测试框架
- **测试指标**: 响应时间、QPS、内存占用等
- **测试报告**: 生成详细的性能测试报告

### 1.3 构建标准

#### CMake 配置
- **最低版本**: 3.15
- **构建类型**: 支持 Debug 和 Release
- **选项**: 支持 `BUILD_TESTS`、`BUILD_EXAMPLES` 等选项
- **输出**: 提供配置摘要输出
- **依赖管理**: 使用 `find_package` 或子模块管理依赖

#### 编译选项
- **警告级别**: 开启所有警告 (`-Wall -Wextra`)
- **错误处理**: 将警告视为错误 (`-Werror`)
- **优化级别**: Debug 模式 `-O0`，Release 模式 `-O2`
- **调试信息**: Debug 模式生成调试信息 (`-g`)

### 1.4 代码质量标准

#### 静态分析
- **工具**: clang-tidy
- **配置**: 使用项目的 .clang-tidy 配置
- **检查范围**: 所有源代码文件

#### 代码格式化
- **工具**: clang-format
- **配置**: 使用项目的 .clang-format 配置
- **执行时机**: 提交代码前执行

#### 内存检测
- **工具**: Valgrind (Linux) 或 AddressSanitizer
- **检测范围**: 所有测试用例
- **报告**: 生成内存检测报告

## 2. CI/CD 指南

### 2.1 CI/CD 流程

#### 持续集成流程
1. **代码提交**: 开发者提交代码到 Git 仓库
2. **触发构建**: CI 系统自动触发构建
3. **代码检查**: 执行代码格式化检查和静态分析
4. **编译构建**: 编译项目，生成可执行文件和库
5. **运行测试**: 执行单元测试、集成测试和性能测试
6. **生成报告**: 生成测试覆盖率和性能测试报告
7. **通知结果**: 通知开发者构建结果

#### 持续部署流程
1. **构建成功**: CI 构建成功后触发部署
2. **打包发布**: 打包构建产物
3. **部署测试**: 部署到测试环境
4. **运行验收测试**: 执行验收测试
5. **部署生产**: 部署到生产环境
6. **监控运行**: 监控系统运行状态

### 2.2 CI/CD 配置示例

#### GitHub Actions 配置

**文件**: `.github/workflows/ci.yml`

```yaml
name: CI

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest]
        build_type: [Debug, Release]

    steps:
    - uses: actions/checkout@v3

    - name: Set up CMake
      uses: actions/setup-cmake@v3
      with:
        cmake-version: '3.15'

    - name: Configure CMake
      run: cmake -B build -DCMAKE_BUILD_TYPE=${{ matrix.build_type }} -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON

    - name: Build
      run: cmake --build build --config ${{ matrix.build_type }}

    - name: Run tests
      run: cd build && ctest -C ${{ matrix.build_type }}

    - name: Check code format
      if: matrix.os == 'ubuntu-latest'
      run: ./scripts/linux/check_format.sh

    - name: Static analysis
      if: matrix.os == 'ubuntu-latest' && matrix.build_type == 'Debug'
      run: ./scripts/linux/run_clang_tidy.sh

    - name: Memory check
      if: matrix.os == 'ubuntu-latest' && matrix.build_type == 'Debug'
      run: ./scripts/linux/run_valgrind.sh
```

#### Jenkins 配置

**文件**: `Jenkinsfile`

```groovy
pipeline {
    agent {
        docker {
            image 'gcc:9'
        }
    }
    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }
        stage('Configure') {
            steps {
                sh 'cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON'
            }
        }
        stage('Build') {
            steps {
                sh 'cmake --build build'
            }
        }
        stage('Test') {
            steps {
                sh 'cd build && ctest'
            }
        }
        stage('Code Quality') {
            steps {
                sh './scripts/linux/check_format.sh'
                sh './scripts/linux/run_clang_tidy.sh'
            }
        }
        stage('Memory Check') {
            steps {
                sh './scripts/linux/run_valgrind.sh'
            }
        }
    }
    post {
        success {
            echo 'Build and tests passed!'
        }
        failure {
            echo 'Build or tests failed!'
        }
    }
}
```

### 2.3 CI/CD 工具链

#### 推荐工具

| 类别 | 工具 | 用途 | 配置文件 |
|-----|------|------|---------|
| 版本控制 | Git | 代码版本管理 | .gitignore |
| 构建工具 | CMake | 跨平台构建系统 | CMakeLists.txt |
| 构建加速 | Ninja | 快速构建 | - |
| 代码质量 | clang-format | 代码格式化 | .clang-format |
| 代码质量 | clang-tidy | 静态代码分析 | .clang-tidy |
| 内存检测 | Valgrind | 内存泄漏检测 | - |
| 内存检测 | AddressSanitizer | 内存错误检测 | - |
| CI/CD | GitHub Actions | 持续集成 | .github/workflows/ci.yml |
| CI/CD | Jenkins | 企业级 CI/CD | Jenkinsfile |
| 容器化 | Docker | 容器化部署 | Dockerfile |
| 容器编排 | Kubernetes | 容器编排 | k8s/deployment.yaml |

### 2.4 自动化脚本

#### 代码格式化检查脚本

**文件**: `scripts/linux/check_format.sh`

```bash
#!/bin/bash

# 检查代码格式
echo "Checking code format..."
find . -name "*.c" -o -name "*.h" | xargs clang-format -i

git diff --exit-code
if [ $? -ne 0 ]; then
    echo "Code format check failed. Please run clang-format."
    exit 1
else
    echo "Code format check passed."
    exit 0
fi
```

#### 静态分析脚本

**文件**: `scripts/linux/run_clang_tidy.sh`

```bash
#!/bin/bash

# 运行静态分析
echo "Running static analysis..."
clang-tidy -p build $(find . -name "*.c" | grep -v "tests")

if [ $? -ne 0 ]; then
    echo "Static analysis failed."
    exit 1
else
    echo "Static analysis passed."
    exit 0
fi
```

#### 内存检测脚本

**文件**: `scripts/linux/run_valgrind.sh`

```bash
#!/bin/bash

# 运行内存检测
echo "Running memory check..."
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./build/tests/test_all

if [ $? -ne 0 ]; then
    echo "Memory check failed."
    exit 1
else
    echo "Memory check passed."
    exit 0
fi
```

## 3. 部署指南

### 3.1 构建与安装

#### Linux 平台

```bash
# 克隆仓库
git clone <repository-url>
cd <project-directory>

# 构建
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j4

# 安装
sudo cmake --install .
```

#### Windows 平台

```powershell
# 克隆仓库
git clone <repository-url>
cd <project-directory>

# 构建
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# 安装
cmake --install . --config Release
```

### 3.2 容器化部署

#### Dockerfile

```dockerfile
FROM gcc:9 AS builder

WORKDIR /app
COPY . .

RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . -j4

FROM debian:stable-slim

WORKDIR /app
COPY --from=builder /app/build/bin/idcu_agent /app/
COPY --from=builder /app/build/libs/ /app/libs/
COPY config/ /app/config/

RUN apt-get update && apt-get install -y libc6-dev

CMD ["./idcu_agent"]
```

#### Kubernetes 部署

**文件**: `k8s/deployment.yaml`

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: idcu-agent
  namespace: default
spec:
  replicas: 3
  selector:
    matchLabels:
      app: idcu-agent
  template:
    metadata:
      labels:
        app: idcu-agent
    spec:
      containers:
      - name: idcu-agent
        image: idcu-agent:latest
        ports:
        - containerPort: 8080
        volumeMounts:
        - name: config
          mountPath: /app/config
      volumes:
      - name: config
        configMap:
          name: idcu-agent-config
---
apiVersion: v1
kind: ConfigMap
metadata:
  name: idcu-agent-config
  namespace: default
data:
  config.yaml: |
    # 配置内容
```

### 3.3 监控与维护

#### 日志管理
- **日志文件**: `/var/log/idcu/`
- **日志级别**: 可配置（DEBUG, INFO, WARN, ERROR）
- **日志轮转**: 自动轮转，保留 7 天

#### 监控指标
- **指标收集**: 使用 idcu-metrics 模块
- **监控工具**: Prometheus + Grafana
- **告警规则**: 配置基于指标的告警规则

#### 健康检查
- **健康检查端点**: `/health`
- **检查频率**: 每 30 秒
- **检查内容**: 模块状态、系统资源、网络连接

## 4. 代码审查流程

### 4.1 审查标准

- **代码质量**: 符合编码规范，无明显缺陷
- **功能正确性**: 实现符合需求，无逻辑错误
- **性能**: 代码性能合理，无明显瓶颈
- **安全性**: 无安全漏洞，符合安全最佳实践
- **可维护性**: 代码清晰，易于理解和维护

### 4.2 审查流程

1. **提交代码**: 开发者提交代码到 feature 分支
2. **创建 PR**: 创建 Pull Request 到 develop 分支
3. **自动检查**: CI 系统执行构建、测试和代码质量检查
4. **代码审查**: 至少 2 名 reviewers 进行代码审查
5. **解决问题**: 开发者根据审查意见修改代码
6. **合并代码**: 审查通过后合并到 develop 分支
7. **发布版本**: 从 develop 分支合并到 main 分支，发布版本

### 4.3 审查工具

- **GitHub/GitLab**: 代码审查平台
- **SonarQube**: 代码质量分析
- **Codecov**: 测试覆盖率分析

## 5. 版本管理

### 5.1 版本号格式

遵循语义化版本规范：`MAJOR.MINOR.PATCH`
- **MAJOR**: 不兼容的 API 变更
- **MINOR**: 向后兼容的功能添加
- **PATCH**: 向后兼容的 bug 修复

### 5.2 分支管理

- **main**: 主分支，包含稳定版本
- **develop**: 开发分支，包含最新开发代码
- **feature/**: 功能分支，开发新功能
- **bugfix/**:  bug 修复分支
- **release/**: 发布分支，准备发布版本

### 5.3 发布流程

1. **创建 release 分支**: 从 develop 分支创建 release 分支
2. **版本号更新**: 更新版本号，准备发布
3. **最终测试**: 执行最终测试，确保质量
4. **合并到 main**: 合并 release 分支到 main 分支
5. **创建标签**: 在 main 分支创建版本标签
6. **合并回 develop**: 将 release 分支合并回 develop 分支
7. **发布通知**: 发布版本，通知相关人员

## 6. 总结

通过遵循上述工程化标准和 CI/CD 流程，可以提高项目的代码质量、开发效率和系统可靠性。工程化标准为开发团队提供了统一的规范，CI/CD 流程自动化了构建、测试和部署过程，容器化部署简化了环境管理，监控和维护确保了系统的稳定运行。

这些标准和流程将帮助 IDCU Agent 项目实现更高效、更可靠的开发和运维，为项目的成功奠定坚实的基础。

---

**审核人**：项目负责人  
**批准日期**：2026-04-08