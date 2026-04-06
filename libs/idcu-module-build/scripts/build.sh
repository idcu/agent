#!/bin/bash
# IDCU Module Build - Linux/macOS Build Script
# 通用跨平台构建脚本 - Unix 版本

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# 检测操作系统
detect_os() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "linux"
    else
        echo "unknown"
    fi
}

OS_TYPE=$(detect_os)

echo "========================================"
echo "IDCU Module Build - Build Script"
echo "Platform: $OS_TYPE"
echo "========================================"

usage() {
    echo "Usage: $0 [clean|debug|release] [module-dir]"
    echo ""
    echo "Examples:"
    echo "  $0                    - Build all in Release mode"
    echo "  $0 debug              - Build all in Debug mode"
    echo "  $0 clean              - Clean build directory"
    echo "  $0 release examples/demo-shared - Build specific module"
    exit 1
}

BUILD_TYPE="Release"
MODULE_DIR=""
CLEAN_BUILD=0
GENERATOR=""

while [[ $# -gt 0 ]]; do
    case $1 in
        clean)
            CLEAN_BUILD=1
            shift
            ;;
        debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        release)
            BUILD_TYPE="Release"
            shift
            ;;
        *)
            if [ -z "$MODULE_DIR" ]; then
                MODULE_DIR="$1"
            else
                usage
            fi
            shift
            ;;
    esac
done

if [ $CLEAN_BUILD -eq 1 ]; then
    echo "Cleaning build directories..."
    rm -rf "$PROJECT_DIR/build"
    echo "Clean complete!"
    exit 0
fi

# 检测可用的生成器
detect_generator() {
    if command -v ninja &> /dev/null; then
        echo "Ninja"
    elif command -v make &> /dev/null; then
        echo "Unix Makefiles"
    else
        echo ""
    fi
}

GENERATOR=$(detect_generator)

if [ -z "$GENERATOR" ]; then
    echo "ERROR: No suitable generator found!"
    echo "Please install Ninja or Make."
    exit 1
fi

echo "Using generator: $GENERATOR"

# 检测并行构建数量
get_jobs() {
    if [ "$OS_TYPE" = "macos" ]; then
        sysctl -n hw.ncpu 2>/dev/null || echo 4
    else
        nproc 2>/dev/null || echo 4
    fi
}

JOBS=$(get_jobs)
echo "Using $JOBS parallel jobs"

BUILD_DIR="$PROJECT_DIR/build"
mkdir -p "$BUILD_DIR"

if [ -z "$MODULE_DIR" ]; then
    echo "Building all modules..."
    
    cd "$BUILD_DIR"
    cmake -G "$GENERATOR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "$PROJECT_DIR"
    cmake --build . -j"$JOBS"
else
    echo "Building module: $MODULE_DIR"
    
    FULL_MODULE_DIR="$PROJECT_DIR/$MODULE_DIR"
    if [ ! -d "$FULL_MODULE_DIR" ]; then
        echo "Module directory not found: $FULL_MODULE_DIR"
        exit 1
    fi
    
    MODULE_BUILD_DIR="$BUILD_DIR/$MODULE_DIR"
    mkdir -p "$MODULE_BUILD_DIR"
    
    cd "$MODULE_BUILD_DIR"
    cmake -G "$GENERATOR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "$FULL_MODULE_DIR"
    cmake --build . -j"$JOBS"
fi

echo ""
echo "========================================"
echo "Build successful!"
echo "Build type: $BUILD_TYPE"
echo "========================================"
