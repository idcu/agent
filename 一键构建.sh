#!/bin/bash

echo "========================================"
echo "  IDCU Agent 一键构建工具"
echo "========================================"
echo ""
echo "正在检查环境..."
echo ""

# 检查 CMake
if ! command -v cmake &> /dev/null; then
    echo "❌ 错误: 未找到 CMake"
    echo ""
    echo "请先安装 CMake 3.14 或更高版本:"
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "  macOS: brew install cmake"
    else
        echo "  Ubuntu/Debian: sudo apt-get install cmake"
        echo "  CentOS/RHEL: sudo yum install cmake"
    fi
    echo ""
    exit 1
fi
echo "✓ CMake 已安装"

# 检查编译器
COMPILER=""
if command -v gcc &> /dev/null; then
    COMPILER="gcc"
    echo "✓ GCC 编译器可用"
elif command -v clang &> /dev/null; then
    COMPILER="clang"
    echo "✓ Clang 编译器可用"
fi

if [ -z "$COMPILER" ]; then
    echo "❌ 错误: 未找到编译器"
    echo ""
    echo "请安装以下之一:"
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "  macOS: xcode-select --install"
    else
        echo "  Ubuntu/Debian: sudo apt-get install build-essential"
        echo "  CentOS/RHEL: sudo yum groupinstall 'Development Tools'"
    fi
    echo ""
    exit 1
fi

# 检查 make
if ! command -v make &> /dev/null; then
    echo "❌ 错误: 未找到 make"
    exit 1
fi
echo "✓ make 可用"

echo ""
echo "========================================"
echo "开始构建..."
echo "========================================"
echo ""

# 创建并进入构建目录
mkdir -p build
cd build

# 配置 CMake
echo "正在配置 CMake..."
if [ "$COMPILER" = "gcc" ]; then
    export CC=gcc
    export CXX=g++
else
    export CC=clang
    export CXX=clang++
fi

cmake -DCMAKE_BUILD_TYPE=Release ..
if [ $? -ne 0 ]; then
    echo "❌ CMake 配置失败"
    exit 1
fi

echo "✓ CMake 配置完成"
echo ""

# 编译
echo "正在编译项目..."
NUM_JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
make -j$NUM_JOBS

if [ $? -ne 0 ]; then
    echo ""
    echo "❌ 编译失败，请查看上面的错误信息"
    exit 1
fi

cd ..

echo ""
echo "========================================"
echo "  ✓ 构建成功!"
echo "========================================"
echo ""
echo "输出位置:"
echo "  主程序: build/bin/idcu_agent"
echo "  配置文件: build/bin/config/"
echo ""
echo "你可以运行 ./build/bin/idcu_agent 来启动程序"
echo ""

# 设置可执行权限
chmod +x build/bin/idcu_agent 2>/dev/null || true
if [ -f build/bin/idcu_benchmark ]; then
    chmod +x build/bin/idcu_benchmark 2>/dev/null || true
fi
