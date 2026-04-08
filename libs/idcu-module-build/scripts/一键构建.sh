#!/bin/bash

echo "=========================================="
echo "  IDCU 一键构建系统 - 小白友好版"
echo "=========================================="
echo ""

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"

echo "[1/4] 检查环境..."
if ! command -v cmake &> /dev/null; then
    echo "❌ 错误：找不到 CMake！请先安装 CMake。"
    echo "   Ubuntu/Debian: sudo apt install cmake"
    echo "   CentOS/RHEL: sudo yum install cmake"
    echo "   macOS: brew install cmake"
    exit 1
fi
echo "✅ CMake 已安装"

if ! command -v gcc &> /dev/null && ! command -v clang &> /dev/null; then
    echo "❌ 错误：找不到编译器！请安装 GCC 或 Clang。"
    echo "   Ubuntu/Debian: sudo apt install build-essential"
    echo "   CentOS/RHEL: sudo yum groupinstall 'Development Tools'"
    echo "   macOS: xcode-select --install"
    exit 1
fi
echo "✅ 编译器已找到"

echo ""
echo "[2/4] 创建构建目录..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
echo "✅ 构建目录已准备"

echo ""
echo "[3/4] 配置 CMake 项目..."
if ! cmake ..; then
    echo "❌ CMake 配置失败！"
    exit 1
fi
echo "✅ CMake 配置成功"

echo ""
echo "[4/4] 编译项目..."
if ! cmake --build . --config Release; then
    echo "❌ 编译失败！"
    exit 1
fi

echo ""
echo "=========================================="
echo "  ✅ 构建成功！"
echo "=========================================="
echo ""
echo "输出文件在：$BUILD_DIR"
echo ""
