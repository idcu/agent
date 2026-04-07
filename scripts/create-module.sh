#!/bin/bash

# IDCU Agent 模块创建脚本
# 用法: ./create-module.sh <模块名称> [目标目录] [依赖项...]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
TEMPLATE_DIR="$PROJECT_ROOT/templates/module"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_usage() {
    echo -e "${YELLOW}用法: $0 <模块名称> [目标目录] [依赖项...]${NC}"
    echo ""
    echo "示例:"
    echo "  $0 my_module"
    echo "  $0 my_module modules/business"
    echo "  $0 my_module modules/business idcu-common idcu-log"
    echo ""
}

# 检查参数
if [ $# -lt 1 ]; then
    print_usage
    exit 1
fi

MODULE_NAME="$1"
shift

# 将模块名转换为目录名（小写+下划线）
MODULE_DIR=$(echo "$MODULE_NAME" | tr '[:upper:]' '[:lower:]' | tr '-' '_')
MODULE_DIR_UPPER=$(echo "$MODULE_DIR" | tr '[:lower:]' '[:upper:]')

# 目标目录
TARGET_DIR="modules/business/$MODULE_DIR"
if [ $# -gt 0 ] && [[ ! "$1" =~ ^idcu- ]]; then
    TARGET_DIR="$1/$MODULE_DIR"
    shift
fi

# 依赖项
DEPENDENCIES="$@"
DEPENDENCIES_CMAKE=""
DEPENDENCIES_JSON=""
for dep in $DEPENDENCIES; do
    DEPENDENCIES_CMAKE="$DEPENDENCIES_CMAKE    $dep"$'\n'
    DEPENDENCIES_JSON="$DEPENDENCIES_JSON    \"$dep\","$'\n'
done
# 移除最后一个逗号
DEPENDENCIES_JSON=$(echo "$DEPENDENCIES_JSON" | sed '$ s/,$//')

FULL_TARGET_DIR="$PROJECT_ROOT/$TARGET_DIR"

echo -e "${GREEN}正在创建模块: $MODULE_NAME${NC}"
echo -e "${GREEN}目标目录: $FULL_TARGET_DIR${NC}"

# 检查目录是否已存在
if [ -d "$FULL_TARGET_DIR" ]; then
    echo -e "${RED}错误: 目录已存在: $FULL_TARGET_DIR${NC}"
    exit 1
fi

# 创建目录结构
mkdir -p "$FULL_TARGET_DIR/src"
mkdir -p "$FULL_TARGET_DIR/include/idcu/$MODULE_DIR"
mkdir -p "$FULL_TARGET_DIR/tests"

# 复制并替换模板文件
echo -e "${YELLOW}生成文件...${NC}"

# CMakeLists.txt
sed -e "s/@MODULE_NAME@/$MODULE_NAME/g" \
    -e "s/@MODULE_DIR@/$MODULE_DIR/g" \
    -e "s/@DEPENDENCIES@/$DEPENDENCIES_CMAKE/g" \
    "$TEMPLATE_DIR/CMakeLists.txt.template" > "$FULL_TARGET_DIR/CMakeLists.txt"

# README.md
sed -e "s/@MODULE_NAME@/$MODULE_NAME/g" \
    -e "s/@MODULE_DIR@/$MODULE_DIR/g" \
    -e "s/@DEPENDENCIES@/$DEPENDENCIES/g" \
    "$TEMPLATE_DIR/README.md.template" > "$FULL_TARGET_DIR/README.md"

# src/module.c
sed -e "s/@MODULE_NAME@/$MODULE_NAME/g" \
    -e "s/@MODULE_DIR@/$MODULE_DIR/g" \
    "$TEMPLATE_DIR/src/module.c.template" > "$FULL_TARGET_DIR/src/${MODULE_DIR}.c"

# include/idcu/module/module.h
sed -e "s/@MODULE_NAME@/$MODULE_NAME/g" \
    -e "s/@MODULE_DIR@/$MODULE_DIR/g" \
    -e "s/@MODULE_DIR_UPPER@/$MODULE_DIR_UPPER/g" \
    "$TEMPLATE_DIR/include/idcu/module/module.h.template" > "$FULL_TARGET_DIR/include/idcu/$MODULE_DIR/${MODULE_DIR}.h"

# tests/CMakeLists.txt
sed -e "s/@MODULE_NAME@/$MODULE_NAME/g" \
    -e "s/@MODULE_DIR@/$MODULE_DIR/g" \
    "$TEMPLATE_DIR/tests/CMakeLists.txt.template" > "$FULL_TARGET_DIR/tests/CMakeLists.txt"

# tests/test_module.c
sed -e "s/@MODULE_NAME@/$MODULE_NAME/g" \
    -e "s/@MODULE_DIR@/$MODULE_DIR/g" \
    "$TEMPLATE_DIR/tests/test_module.c.template" > "$FULL_TARGET_DIR/tests/test_${MODULE_DIR}.c"

# module.json
sed -e "s/@MODULE_NAME@/$MODULE_NAME/g" \
    -e "s/@DEPENDENCIES_JSON@/$DEPENDENCIES_JSON/g" \
    "$TEMPLATE_DIR/module.json.template" > "$FULL_TARGET_DIR/module.json"

echo -e "${GREEN}模块创建成功!${NC}"
echo ""
echo "已创建的文件:"
echo "  $TARGET_DIR/CMakeLists.txt"
echo "  $TARGET_DIR/README.md"
echo "  $TARGET_DIR/module.json"
echo "  $TARGET_DIR/src/${MODULE_DIR}.c"
echo "  $TARGET_DIR/include/idcu/$MODULE_DIR/${MODULE_DIR}.h"
echo "  $TARGET_DIR/tests/CMakeLists.txt"
echo "  $TARGET_DIR/tests/test_${MODULE_DIR}.c"
echo ""
echo -e "${YELLOW}下一步:${NC}"
echo "  1. 编辑 $TARGET_DIR/README.md 添加模块说明"
echo "  2. 在 $TARGET_DIR/src/${MODULE_DIR}.c 中实现模块功能"
echo "  3. 在 $TARGET_DIR/include/idcu/$MODULE_DIR/${MODULE_DIR}.h 中添加公共 API"
echo "  4. 在主 CMakeLists.txt 中添加模块目录（如需要）"
echo ""
