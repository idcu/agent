#!/bin/bash
mkdir -p out build
cd build
cmake ..
make -j$(nproc)
cd ..
cp build/idcu_agent out/
echo "Build done: out/idcu_agent"