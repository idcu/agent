#!/bin/bash
set -e

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

# 创建目录
mkdir -p out build

# 进入构建目录
cd build

# 生成Makefile
cmake -DCMAKE_BUILD_TYPE=Release ..
if [ $? -ne 0 ]; then
    echo -e "${RED}CMake generate failed!${NC}"
    exit 1
fi

# 编译
make -j$(nproc)
if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

# 复制产物
cd ..
cp -f build/idcu_agent out/
cp -f config/agent.cfg out/

# 权限设置
chmod +x out/idcu_agent

echo -e "${GREEN}======================================"
echo -e "Build success: out/idcu_agent"
echo -e "Config file: out/agent.cfg"
echo -e "======================================${NC}"