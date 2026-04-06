#!/bin/bash
# 代码覆盖率生成脚本 - 用于 Linux/macOS
# 使用方法: ./scripts/generate_coverage.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

BUILD_DIR="${PROJECT_ROOT}/build"

echo "========================================="
echo "  IDCU Agent - 代码覆盖率生成工具"
echo "========================================="
echo ""
echo "覆盖率目标:"
echo "  - 微内核 (modules/core/micro-kernel): ≥85%"
echo "  - 模块系统 (modules/core/module-system): ≥80%"
echo "  - 调度器 (modules/core/scheduler): ≥80%"
echo "  - 核心库 (libs/): ≥75%"
echo ""

# 检查系统是否支持
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    echo "错误: 代码覆盖率功能仅支持 Linux/macOS"
    exit 1
fi

# 检查是否安装了 lcov
if ! command -v lcov &> /dev/null; then
    echo "错误: lcov 未安装。请安装 lcov:"
    echo "  Ubuntu/Debian: sudo apt-get install lcov"
    echo "  macOS: brew install lcov"
    exit 1
fi

# 检查是否安装了 genhtml
if ! command -v genhtml &> /dev/null; then
    echo "警告: genhtml 未安装，将无法生成 HTML 报告"
fi

echo ""
echo "1. 配置 CMake 项目..."
cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTS=ON \
    -DENABLE_CODE_COVERAGE=ON \
    "$PROJECT_ROOT"

echo ""
echo "2. 编译项目..."
cmake --build "$BUILD_DIR" --config Debug

echo ""
echo "3. 运行测试并生成覆盖率报告..."
cmake --build "$BUILD_DIR" --target coverage

echo ""
echo "========================================="
echo "  覆盖率报告生成完成！"
echo "========================================="
echo ""
if [ -d "$BUILD_DIR/coverage_report" ]; then
    echo "HTML 报告位置: $BUILD_DIR/coverage_report/index.html"
    echo "你可以在浏览器中打开该文件查看报告"
    echo ""
    echo "请检查核心模块的覆盖率是否达到目标:"
    echo "  - modules/core/micro-kernel: ≥85%"
    echo "  - modules/core/module-system: ≥80%"
    echo "  - modules/core/scheduler: ≥80%"
    echo "  - libs/: ≥75%"
else
    echo "覆盖率数据文件: $BUILD_DIR/coverage_filtered.info"
fi
echo ""

