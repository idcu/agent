#!/bin/bash
# 运行 clang-tidy 静态分析 (Linux/macOS 版本)

echo "=== 运行 clang-tidy 静态分析 ==="

# 检查 clang-tidy 是否安装
if ! command -v clang-tidy &> /dev/null; then
    echo "错误: 未找到 clang-tidy，请先安装 LLVM/Clang"
    echo "Ubuntu/Debian: sudo apt-get install clang-tidy"
    echo "macOS: brew install llvm"
    exit 1
fi

# 如果未指定构建目录，使用默认值
if [ -z "$BUILD_DIR" ]; then
    BUILD_DIR="build"
fi

# 检查是否存在编译数据库
if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "错误: 未找到编译数据库 compile_commands.json"
    echo ""
    echo "请先运行 CMake 并生成编译数据库:"
    echo "  cmake -B $BUILD_DIR -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    exit 1
fi

echo "使用构建目录: $BUILD_DIR"

# 定义要分析的目录
DIRS="libs modules app"

# 查找所有 C 源文件
echo "正在查找源文件..."
FILES=""
for dir in $DIRS; do
    if [ -d "$dir" ]; then
        while IFS= read -r -d $'\0' file; do
            FILES="$FILES $file"
        done < <(find "$dir" -name "*.c" -print0)
    fi
done

if [ -z "$FILES" ]; then
    echo "未找到源文件"
    exit 1
fi

# 运行 clang-tidy
echo ""
echo "正在运行 clang-tidy..."
echo ""
clang-tidy $FILES -p $BUILD_DIR --header-filter=".*"

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ 静态分析完成，未发现问题"
else
    echo ""
    echo "❌ 静态分析发现问题，请修复"
    exit 1
fi
