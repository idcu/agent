#!/bin/bash
# 自动格式化代码

echo "=== 自动格式化代码 (clang-format) ==="

# 检查 clang-format 是否安装
if ! command -v clang-format &> /dev/null; then
    echo "错误: 未找到 clang-format，请先安装"
    exit 1
fi

# 定义要格式化的目录
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

echo "正在格式化 ${#FILES[@]} 个文件..."

# 格式化所有文件
for file in "${FILES[@]}"; do
    clang-format -i "$file"
    echo "已格式化: $file"
done

echo ""
echo "✅ 格式化完成"
