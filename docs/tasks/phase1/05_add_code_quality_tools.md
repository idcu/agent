# 任务 1.5: 添加代码质量工具

## 目标

配置代码格式化和静态分析工具，从第一天就保证代码质量。

## 详细步骤

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

#### Windows (scripts/windows/format.bat)

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

#### Linux (scripts/linux/format.sh)

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

#### Windows (scripts/windows/check_format.bat)

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

#### Linux (scripts/linux/check_format.sh)

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
chmod +x scripts/linux/format.sh
chmod +x scripts/linux/check_format.sh
```

### 6. 运行格式化

```bash
# Windows
scripts/windows/format.bat

# Linux
scripts/linux/format.sh
```

## 验证检查清单

- [ ] .clang-format 已创建
- [ ] .clang-tidy 已创建
- [ ] 格式化脚本已创建
- [ ] 检查脚本已创建
- [ ] 可以运行格式化脚本
- [ ] 可以运行检查脚本
- [ ] 现有代码已格式化

## Git 提交

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
scripts/windows/format.bat  # 或 scripts/linux/format.sh
git add -u
git commit -m "style: format existing code with clang-format"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| clang-format 未找到 | 未安装 clang-format | 安装 LLVM 或 Clang 工具链 |
| 脚本无法运行 | 脚本没有执行权限 | chmod +x (Linux) |
| 格式化结果不一致 | clang-format 版本不同 | 统一团队使用的 clang-format 版本 |

## 经验提示

- 从第一天就启用代码质量工具
- 使用 Google 风格作为基础，根据项目需求调整
- 在提交前运行格式化脚本
- 将格式检查集成到 CI/CD 流程中
