# 任务 1.5: 添加代码质量工具

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
配置代码格式化和静态分析工具，从第一天就保证代码质量。创建 clang-format 和 clang-tidy 配置文件，以及跨平台的格式化脚本。

### 1.2 不做什么
- 不集成其他代码质量工具（如 cppcheck、valgrind）
- 不配置 CI/CD 流程（仅配置本地工具）
- 不创建代码审查流程

### 1.3 输入
- 项目源代码
- LLVM/Clang 工具链（含 clang-format、clang-tidy）

### 1.4 输出
- `.clang-format` 配置文件
- `.clang-tidy` 配置文件
- 格式化脚本（Windows/Linux）
- 格式检查脚本（Windows/Linux）
- 格式化后的代码

### 1.5 前置依赖
- 任务 1.4 已完成（项目可以正常编译）
- LLVM/Clang 工具链已安装（含 clang-format、clang-tidy）

---

## 2. 技术实现方案

### 2.1 核心选型
- 代码格式化：clang-format
- 静态分析：clang-tidy
- 基础风格：Google Style

### 2.2 核心逻辑
```
1. 创建 .clang-format 配置
   ├─ 基于 Google Style
   ├─ 调整缩进（4 空格）
   ├─ 配置列宽（100 字符）
   └─ 配置指针对齐方式
2. 创建 .clang-tidy 配置
   ├─ 启用 clang-analyzer、performance、modernize、readability 检查
   ├─ 禁用不适用的检查
   └─ 配置命名规则
3. 创建格式化脚本（Windows/Linux）
4. 创建格式检查脚本（Windows/Linux）
5. 运行格式化脚本，格式化现有代码
```

### 2.3 数据结构/接口
- `.clang-format`：YAML 格式的配置文件
- `.clang-tidy`：YAML 格式的配置文件
- 脚本文件：`.bat`（Windows）、`.sh`（Linux）

### 2.4 跨平台适配
- Windows：使用 `.bat` 批处理脚本，使用 `for /r` 遍历文件
- Linux：使用 `.sh` Shell 脚本，使用 `find` 遍历文件
- 脚本编码：Windows 使用 GBK 或 UTF-8 with BOM，Linux 使用 UTF-8

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] `.clang-format` 已创建
- [ ] `.clang-tidy` 已创建
- [ ] Windows 格式化脚本已创建
- [ ] Linux 格式化脚本已创建
- [ ] Windows 格式检查脚本已创建
- [ ] Linux 格式检查脚本已创建
- [ ] 格式化脚本可以正常运行
- [ ] 格式检查脚本可以正常运行
- [ ] 现有代码已格式化

### 3.2 性能验收
- 格式化所有代码时间 ≤ 30 秒
- 格式检查所有代码时间 ≤ 30 秒

### 3.3 异常验收
- [ ] clang-format 未安装时给出明确提示
- [ ] 脚本权限不足时给出明确提示
- [ ] 格式检查失败时返回非零退出码

---

## 4. 执行计划

### 4.1 工期
1.5 小时/人

### 4.2 里程碑
- D5-00: 创建 .clang-format 配置
- D5-20: 创建 .clang-tidy 配置
- D5-40: 创建格式化脚本
- D5-60: 创建格式检查脚本
- D5-90: 运行格式化并提交

### 4.3 人力
1 人（技能要求：了解 clang-format/clang-tidy）

---

## 5. 工程化要求

### 5.1 编码规范
- 脚本文件使用清晰的注释
- 脚本提供友好的输出信息

### 5.2 测试要求
- 在 Windows 上测试脚本（如果可用）
- 在 Linux 上测试脚本（如果可用）
- 验证格式化后代码可以正常编译
- 验证格式检查脚本可以检测未格式化的代码

### 5.3 部署指引
- 无（工具为本地开发工具）

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：clang-format/clang-tidy 未安装  
应对：在脚本中检查工具是否存在，给出明确的安装指引

### 6.2 风险2
描述：clang-format 版本差异导致格式化结果不一致  
应对：在文档中说明推荐的版本，团队统一使用相同版本

### 6.3 风险3
描述：格式化后代码出现功能性错误  
应对：格式化前先提交代码，格式化后运行测试验证

### 6.4 风险4
描述：脚本在特定平台上脚本执行失败  
应对：提供跨平台的脚本，充分测试

---

## 7. 详细实现步骤

### 1. 创建 .clang-format（更详细的配置）

```yaml
---
Language: Cpp
BasedOnStyle: Google
IndentWidth: 4
TabWidth: 4
UseTab: Never
ColumnLimit: 100

# 指针和引用对齐
PointerAlignment: Left

# 括号风格
BreakBeforeBraces: Attach

# 空行
MaxEmptyLinesToKeep: 2
KeepEmptyLinesAtTheStartOfBlocks: false

# 包含排序
IncludeBlocks: Regroup
IncludeCategories:
  - Regex: '^<.*\.h>'
    Priority: 1
  - Regex: '^".*'
    Priority: 2

# 其他设置
SpaceBeforeParens: ControlStatements
SpaceInEmptyParentheses: false
SpacesInAngles: false
SpacesInCStyleCastParentheses: false
SpacesInContainerLiterals: true
SpacesInParentheses: false
SpacesInSquareBrackets: false
...
```

### 2. 创建 .clang-tidy（更详细的配置）

```yaml
---
Checks: >
  -*,
  clang-analyzer-*,
  performance-*,
  modernize-*,
  readability-*,
  -modernize-use-trailing-return-type,
  -readability-magic-numbers

WarningsAsErrors: ''
HeaderFilterRegex: ''
FormatStyle: file

CheckOptions:
  - key: readability-identifier-naming.FunctionCase
    value: lower_case
  - key: readability-identifier-naming.VariableCase
    value: lower_case
  - key: readability-identifier-naming.ParameterCase
    value: lower_case
  - key: readability-identifier-naming.MemberCase
    value: lower_case
  - key: readability-identifier-naming.GlobalConstantCase
    value: UPPER_CASE
  - key: readability-identifier-naming.EnumConstantCase
    value: UPPER_CASE
  - key: readability-identifier-naming.TypeAliasCase
    value: CamelCase
  - key: readability-identifier-naming.TypedefCase
    value: CamelCase
...
```

### 3. 创建格式化脚本

#### Windows (scripts/format.bat)

```batch
@echo off
echo Formatting C/C++ files...

REM 格式化所有 .c 和 .h 文件
for /r %%f in (*.c *.h) do (
    echo Formatting: %%f
    clang-format -i "%%f"
)

echo Formatting complete!
```

#### Linux (scripts/format.sh)

```bash
#!/bin/bash

echo "Formatting C/C++ files..."

# 格式化所有 .c 和 .h 文件
find . -name "*.c" -o -name "*.h" | while read -r file; do
    echo "Formatting: $file"
    clang-format -i "$file"
done

echo "Formatting complete!"
```

### 4. 创建格式检查脚本

#### Windows (scripts/check_format.bat)

```batch
@echo off
echo Checking code format...

set HAS_ERRORS=0

REM 检查所有 .c 和 .h 文件
for /r %%f in (*.c *.h) do (
    clang-format --dry-run --Werror "%%f" >nul 2>&1
    if errorlevel 1 (
        echo File needs formatting: %%f
        set HAS_ERRORS=1
    )
)

if %HAS_ERRORS% equ 0 (
    echo All files are properly formatted!
) else (
    echo Some files need formatting!
    exit /b 1
)
```

#### Linux (scripts/check_format.sh)

```bash
#!/bin/bash

echo "Checking code format..."

HAS_ERRORS=0

# 检查所有 .c 和 .h 文件
find . -name "*.c" -o -name "*.h" | while read -r file; do
    if ! clang-format --dry-run --Werror "$file" > /dev/null 2>&1; then
        echo "File needs formatting: $file"
        HAS_ERRORS=1
    fi
done

if [ "$HAS_ERRORS" -eq 0 ]; then
    echo "All files are properly formatted!"
else
    echo "Some files need formatting!"
    exit 1
fi
```

### 5. 给脚本添加执行权限（Linux）

```bash
chmod +x scripts/format.sh
chmod +x scripts/check_format.sh
```

### 6. 运行格式化

```bash
# Windows
scripts/format.bat

# Linux
scripts/format.sh
```

---

## 8. 验证检查清单

- [ ] .clang-format 已创建
- [ ] .clang-tidy 已创建
- [ ] 格式化脚本已创建
- [ ] 检查脚本已创建
- [ ] 可以运行格式化脚本
- [ ] 可以运行检查脚本
- [ ] 现有代码已格式化
- [ ] 格式化后代码可以正常编译
- [ ] 脚本在目标平台可以正常执行
- [ ] clang-tidy 检查无关键警告
- [ ] 符合工程化标准中的代码质量要求
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add .clang-format
git add .clang-tidy
git add scripts/
git commit -m "chore: add code quality tools

- Add .clang-format configuration
- Add .clang-tidy configuration
- Add format scripts for Windows and Linux
- Add check-format scripts for Windows and Linux"

# 格式化现有代码并提交
scripts/format.bat  # 或 scripts/format.sh
git add -u
git commit -m "style: format existing code with clang-format"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| clang-format 未找到 | 未安装 clang-format | 安装 LLVM 或 Clang 工具链 |
| 脚本无法运行 | 脚本没有执行权限 | chmod +x (Linux) |
| 格式化结果不一致 | clang-format 版本不同 | 统一团队使用的 clang-format 版本 |
