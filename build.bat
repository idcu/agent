@echo off
mkdir out build 2>nul
cd build
cmake ..
cmake --build . --config Release
cd ..
copy build\Release\idcu_agent.exe out\
echo Build done: out/idcu_agent.exe