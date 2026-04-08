# 任务 1.4: 验证项目可以编译

## 目标

确保 CMake 构建系统可以正常工作，在多个平台上验证。

## 详细步骤

### 1. Windows (MinGW)

```bash
# 创建构建目录
mkdir -p build
cd build

# 配置 CMake
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug ..

# 编译
mingw32-make -j4

# 运行程序
./bin/idcu_agent.exe

# 测试命令行参数
./bin/idcu_agent.exe --help
./bin/idcu_agent.exe --version
```

### 2. Windows (MSVC)

```bash
# 创建构建目录
mkdir -p build
cd build

# 配置 CMake（Visual Studio 2019）
cmake -G "Visual Studio 16 2019" -A x64 ..

# 编译 Debug 版本
cmake --build . --config Debug

# 运行程序
./Debug/bin/idcu_agent.exe
```

### 3. Linux

```bash
# 创建构建目录
mkdir -p build
cd build

# 配置 CMake
cmake -DCMAKE_BUILD_TYPE=Debug ..

# 编译
make -j4

# 运行程序
./bin/idcu_agent

# 测试命令行参数
./bin/idcu_agent --help
./bin/idcu_agent --version
```

### 4. 清理并重新构建（验证）

```bash
# 清理
cd ..
rm -rf build

# 重新配置和编译
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4

# 验证 Release 版本
./build/bin/idcu_agent --version
```

## 验证检查清单

- [ ] Debug 版本可以正常编译
- [ ] Release 版本可以正常编译
- [ ] 程序可以正常运行
- [ ] `--help` 参数工作正常
- [ ] `--version` 参数工作正常
- [ ] 可以正常退出（Ctrl+C）
- [ ] 在至少一个平台上验证过

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| CMake 找不到编译器 | 编译器未安装或不在 PATH 中 | 安装编译器并检查 PATH |
| 链接错误 | 缺少依赖库 | 检查依赖是否正确安装 |
| 中文乱码 | 字符编码问题 | 确保源文件是 UTF-8 编码 |
| 权限错误 | 当前用户没有写入权限 | 使用管理员权限或修改目录权限 |

## Git 提交（可选，验证通过后）

```bash
# 如果需要提交验证结果的文档
git add docs/...
git commit -m "docs: add build verification guide"
```

## 经验提示

- 先在一个平台上验证通过，再尝试其他平台
- Debug 版本用于开发，Release 版本用于生产
- 使用多核编译（-j4）可以加快构建速度
- 清理构建目录后重新构建可以验证构建系统的可靠性
