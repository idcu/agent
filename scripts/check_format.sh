#!/bin/bash
# 检查代码格式

echo "=== 检查代码格式 (clang-format) ==="

# 检查 clang-format 是否安装
if ! command -v clang-format &> /dev/null; then
    echo "错误: 未找到 clang-format，请先安装"
    exit 1
fi

# 定义要检查的目录
DIRS=(
    "libs"
    "modules"
    "app"
    "tests"
)

# 收集所有源文件
FILES=()
for dir in "${DIRS[@]}"; do
    if [ -d "$dir" ]; then
        while IFS= read -r -d $'\0' file; do
            FILES+=("$file")
        done < <(find "$dir" -type f \( -name "*.c" -o -name "*.h" \) -print0)
    fi
done

if [ ${#FILES[@]} -eq 0 ]; then
    echo "警告: 未找到任何源文件"
    exit 0
fi

# 运行 clang-format 检查
echo "正在检查 ${#FILES[@]} 个文件..."

# 检查哪些文件需要格式化
NEED_FORMAT=()
for file in "${FILES[@]}"; do
    if ! clang-format --dry-run --Werror "$file" > /dev/null 2>&1; then
        NEED_FORMAT+=("$file")
    fi
done

if [ ${#NEED_FORMAT[@]} -eq 0 ]; then
    echo "✅ 所有文件格式正确"
    exit 0
else
    echo "❌ ${#NEED_FORMAT[@]} 个文件需要格式化:"
    for file in "${NEED_FORMAT[@]}"; do
        echo "  - $file"
    done
    echo ""
    echo "运行以下命令自动修复:"
    echo "  ./scripts/format.sh"
    exit 1
fi
