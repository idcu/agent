#!/bin/bash
set -e

# ======================================
# IDCU Agent 统一构建脚本 (Linux/macOS)
# ======================================

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 默认参数
PLATFORM="linux"
BUILD_TYPE="Release"
WITH_BENCHMARK=ON
WITH_TESTS=OFF
CLEAN_BUILD=OFF
COMPILER=""
VERBOSE=OFF
HELP=0

# 脚本目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "${SCRIPT_DIR}")"

# 显示帮助信息
show_help() {
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help              显示此帮助信息"
    echo "  -t, --build-type TYPE   设置构建类型 (Debug/Release/RelWithDebInfo) [默认: Release]"
    echo "  -b, --with-benchmark    包含基准测试 [默认: ON]"
    echo "  --no-benchmark          不包含基准测试"
    echo "  -T, --with-tests        包含测试 [默认: OFF]"
    echo "  -c, --clean             清理旧构建后重新构建"
    echo "  -C, --compiler COMPILER 指定编译器 (gcc/clang)"
    echo "  -v, --verbose           显示详细输出"
    echo ""
    echo "示例:"
    echo "  $0                                    # 默认 Release 构建"
    echo "  $0 -t Debug -c                       # Debug 构建，清理旧缓存"
    echo "  $0 -t Release -C clang               # Release 构建，使用 Clang"
    echo "  $0 -t Debug --with-tests             # Debug 构建，包含测试"
    echo ""
    echo "平台兼容性:"
    echo "  - Linux: GCC 7.0+ / Clang 6.0+"
    echo "  - CentOS/Ubuntu 均支持"
    echo "  - macOS: Clang 6.0+"
    echo ""
}

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            HELP=1
            shift
            ;;
        -t|--build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -b|--with-benchmark)
            WITH_BENCHMARK=ON
            shift
            ;;
        --no-benchmark)
            WITH_BENCHMARK=OFF
            shift
            ;;
        -T|--with-tests)
            WITH_TESTS=ON
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=ON
            shift
            ;;
        -C|--compiler)
            COMPILER="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=ON
            shift
            ;;
        *)
            echo -e "${RED}错误: 未知参数 $1${NC}"
            echo ""
            show_help
            exit 1
            ;;
    esac
done

if [[ $HELP -eq 1 ]]; then
    show_help
    exit 0
fi

# 转换构建类型为小写以用于目录名
BUILD_TYPE_LOWER=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')
BUILD_DIR="${PROJECT_ROOT}/build"
OUTPUT_DIR="${PROJECT_ROOT}/build/output/${BUILD_TYPE_LOWER}"

# 检查依赖
check_dependencies() {
    echo -e "${BLUE}检查构建依赖...${NC}"
    
    # 检查 CMake
    if ! command -v cmake &> /dev/null; then
        echo -e "${RED}错误: 未找到 CMake，请先安装 CMake 3.14+${NC}"
        echo "Ubuntu/Debian: sudo apt-get install cmake"
        echo "CentOS/RHEL: sudo yum install cmake"
        echo "macOS: brew install cmake"
        exit 1
    fi
    
    CMAKE_VERSION=$(cmake --version | head -n1 | awk '{print $3}')
    echo -e "${GREEN}✓ CMake 版本: ${CMAKE_VERSION}${NC}"
    
    # 检查最低版本要求
    if ! printf '%s\n%s' "3.14" "$CMAKE_VERSION" | sort -C -V; then
        echo -e "${RED}错误: CMake 版本过低，需要 3.14+，当前为 ${CMAKE_VERSION}${NC}"
        exit 1
    fi
    
    # 检查编译器
    if [[ -z "$COMPILER" ]]; then
        if command -v gcc &> /dev/null; then
            COMPILER="gcc"
        elif command -v clang &> /dev/null; then
            COMPILER="clang"
        else
            echo -e "${RED}错误: 未找到 GCC 或 Clang 编译器${NC}"
            echo "Ubuntu/Debian: sudo apt-get install build-essential"
            echo "CentOS/RHEL: sudo yum groupinstall 'Development Tools'"
            exit 1
        fi
    fi
    
    if [[ "$COMPILER" == "gcc" ]]; then
        if ! command -v gcc &> /dev/null; then
            echo -e "${RED}错误: 未找到 GCC 编译器${NC}"
            exit 1
        fi
        GCC_VERSION=$(gcc -dumpversion)
        echo -e "${GREEN}✓ GCC 版本: ${GCC_VERSION}${NC}"
        if ! printf '%s\n%s' "7.0" "$GCC_VERSION" | sort -C -V; then
            echo -e "${RED}错误: GCC 版本过低，需要 7.0+，当前为 ${GCC_VERSION}${NC}"
            exit 1
        fi
        export CC=gcc
        export CXX=g++
    elif [[ "$COMPILER" == "clang" ]]; then
        if ! command -v clang &> /dev/null; then
            echo -e "${RED}错误: 未找到 Clang 编译器${NC}"
            exit 1
        fi
        CLANG_VERSION=$(clang --version | head -n1 | awk '{print $3}')
        echo -e "${GREEN}✓ Clang 版本: ${CLANG_VERSION}${NC}"
        if ! printf '%s\n%s' "6.0" "$CLANG_VERSION" | sort -C -V; then
            echo -e "${RED}错误: Clang 版本过低，需要 6.0+，当前为 ${CLANG_VERSION}${NC}"
            exit 1
        fi
        export CC=clang
        export CXX=clang++
    else
        echo -e "${RED}错误: 不支持的编译器: ${COMPILER}${NC}"
        exit 1
    fi
    
    # 检查 make
    if ! command -v make &> /dev/null; then
        echo -e "${RED}错误: 未找到 make${NC}"
        exit 1
    fi
    echo -e "${GREEN}✓ make 可用${NC}"
    
    echo ""
}

# 显示构建信息
show_build_info() {
    echo -e "${YELLOW}========================================${NC}"
    echo -e "${YELLOW}  IDCU Agent 构建配置${NC}"
    echo -e "${YELLOW}========================================${NC}"
    echo -e "  项目根目录:    ${PROJECT_ROOT}"
    echo -e "  构建目录:      ${BUILD_DIR}"
    echo -e "  输出目录:      ${OUTPUT_DIR}"
    echo -e "  编译器:        ${COMPILER}"
    echo -e "  构建类型:      ${BUILD_TYPE}"
    echo -e "  包含基准测试:  ${WITH_BENCHMARK}"
    echo -e "  包含测试:      ${WITH_TESTS}"
    echo -e "  清理构建:      ${CLEAN_BUILD}"
    echo -e "${YELLOW}========================================${NC}"
    echo ""
}

# 清理旧构建
clean_build() {
    if [[ $CLEAN_BUILD == ON ]]; then
        echo -e "${BLUE}清理旧构建缓存...${NC}"
        if [[ -d "$BUILD_DIR" ]]; then
            rm -rf "$BUILD_DIR"
            echo -e "${GREEN}✓ 已清理构建目录${NC}"
        fi
    fi
}

# 执行构建
build() {
    echo -e "${BLUE}开始构建...${NC}"
    
    # 创建构建目录
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # CMake 配置
    echo -e "${BLUE}CMake 配置中...${NC}"
    CMAKE_ARGS=(
        "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}"
        "-DBUILD_BENCHMARKS=${WITH_BENCHMARK}"
        "-DBUILD_TESTS=${WITH_TESTS}"
    )
    
    if [[ $VERBOSE == ON ]]; then
        echo "CMake 参数: ${CMAKE_ARGS[*]}"
    fi
    
    cmake "${CMAKE_ARGS[@]}" "$PROJECT_ROOT"
    if [[ $? -ne 0 ]]; then
        echo -e "${RED}CMake 配置失败!${NC}"
        exit 1
    fi
    
    # 编译
    echo -e "${BLUE}编译中...${NC}"
    NUM_JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    if [[ $VERBOSE == ON ]]; then
        make -j"$NUM_JOBS" VERBOSE=1
    else
        make -j"$NUM_JOBS"
    fi
    
    if [[ $? -ne 0 ]]; then
        echo -e "${RED}编译失败!${NC}"
        exit 1
    fi
    
    cd "$PROJECT_ROOT"
}

# 完成
finish() {
    echo ""
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}  构建成功!${NC}"
    echo -e "${GREEN}========================================${NC}"
    echo -e "  主程序: ${OUTPUT_DIR}/bin/idcu_agent"
    
    if [[ -f "${OUTPUT_DIR}/bin/idcu_benchmark" ]]; then
        echo -e "  基准测试: ${OUTPUT_DIR}/bin/idcu_benchmark"
    fi
    
    echo -e "  配置文件: ${OUTPUT_DIR}/bin/config/"
    echo -e "${GREEN}========================================${NC}"
    echo ""
    
    # 设置可执行权限
    chmod +x "${OUTPUT_DIR}/bin/idcu_agent" 2>/dev/null || true
    if [[ -f "${OUTPUT_DIR}/bin/idcu_benchmark" ]]; then
        chmod +x "${OUTPUT_DIR}/bin/idcu_benchmark" 2>/dev/null || true
    fi
}

# 主流程
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  IDCU Agent 统一构建脚本${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

check_dependencies
show_build_info
clean_build
build
finish

exit 0
