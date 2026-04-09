# 任务 1.4: 验证项目可以编译

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
在多个平台（Windows/MSVC、Windows/MinGW、Linux）上验证 CMake 构建系统可以正常工作，确保 Debug 和 Release 版本都能成功编译和运行，验证命令行参数功能正常。

### 1.2 不做什么
- 不进行性能基准测试
- 不进行内存泄漏检测
- 不进行集成测试（仅验证基础功能）

### 1.3 输入
- 已创建的项目源代码
- CMake 构建配置
- 编译器工具链

### 1.4 输出
- 编译成功的可执行文件
- 控制台输出（帮助信息、版本信息、欢迎消息）
- 验证结果报告

### 1.5 前置依赖
- 任务 1.3 已完成（CMake 构建系统已配置）
- 编译器已正确安装并配置
- CMake 3.14+ 已安装

---

## 2. 技术实现方案

### 2.1 核心选型
- Windows/MSVC：Visual Studio 2019+
- Windows/MinGW：MinGW-w64
- Linux：GCC 或 Clang

### 2.2 核心逻辑
```
1. 创建构建目录
2. 配置 CMake
3. 编译项目
4. 运行可执行文件
5. 测试命令行参数
6. 清理并重新构建（验证可靠性）
7. 测试 Release 版本
```

### 2.3 数据结构/接口
- 可执行文件路径：build/bin/idcu_agent（或 build/Debug/bin/idcu_agent.exe）
- 命令行参数：--help、--version
- 返回码：0（成功）、非零（失败）

### 2.4 跨平台适配
- Windows/MSVC：使用 "Visual Studio 16 2019" 生成器，通过 --config 指定构建类型
- Windows/MinGW：使用 "MinGW Makefiles" 生成器，通过 -DCMAKE_BUILD_TYPE 指定构建类型
- Linux：使用默认生成器，通过 -DCMAKE_BUILD_TYPE 指定构建类型
- 路径分隔符：Windows 使用 \，Linux 使用 /

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] `cmake -B build` 配置成功，无错误
- [ ] `cmake --build build` 编译成功
- [ ] Debug 版本可以正常运行
- [ ] Release 版本可以正常运行
- [ ] `./idcu_agent --help` 输出正确的帮助信息
- [ ] `./idcu_agent --version` 输出版本号 "0.1.0"
- [ ] 无参数运行时输出欢迎消息
- [ ] 可以通过 Ctrl+C 正常退出
- [ ] 清理构建目录后重新构建成功

### 3.2 性能验收
- 配置时间 ≤ 5 秒
- 编译时间 ≤ 30 秒（首次）
- 程序启动时间 ≤ 100ms
- 内存占用 ≤ 1MB

### 3.3 异常验收
- [ ] 未知参数时输出错误信息
- [ ] 编译错误时给出明确提示
- [ ] 返回码正确（成功为 0，失败为非零）

---

## 4. 执行计划

### 4.1 工期
1 小时/人

### 4.2 里程碑
- D4-00: 在一个平台上完成验证
- D4-20: 完成 Debug 版本验证
- D4-40: 完成 Release 版本验证
- D4-60: 完成清理重新构建验证

### 4.3 人力
1 人（技能要求：基本命令行操作）

---

## 5. 工程化要求

### 5.1 编码规范
- 无（本任务为验证任务）

### 5.2 测试要求
- 至少在一个平台上完成完整验证
- 测试 Debug 和 Release 两种构建类型
- 测试所有命令行参数
- 测试清理重新构建

### 5.3 部署指引
- 无（本任务为验证任务）

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：编译器未正确安装或不在 PATH 中  
应对：检查编译器是否正确安装，验证 PATH 环境变量

### 6.2 风险2
描述：CMake 缓存导致配置问题  
应对：删除 build 目录后重新配置

### 6.3 风险3
描述：运行时库缺失导致程序无法启动  
应对：检查系统依赖库是否完整安装

### 6.4 风险4
描述：字符编码问题导致中文输出乱码  
应对：确保源文件使用 UTF-8 编码，设置正确的控制台编码

---

## 7. 详细实现步骤

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

---

## 8. 验证检查清单

- [ ] Debug 版本可以正常编译
- [ ] Release 版本可以正常编译
- [ ] 程序可以正常运行
- [ ] `--help` 参数工作正常
- [ ] `--version` 参数工作正常
- [ ] 可以正常退出（Ctrl+C）
- [ ] 在至少一个平台上验证过
- [ ] 清理后重新构建成功
- [ ] 程序退出码正确（成功为 0）
- [ ] 输出信息符合预期格式
- [ ] 已验证无内存泄漏（基础检查）

---

## 9. Git 提交（可选，验证通过后）

```bash
# 如果需要提交验证结果的文档
git add docs/...
git commit -m "docs: add build verification guide"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| CMake 找不到编译器 | 编译器未安装或不在 PATH 中 | 安装编译器并检查 PATH |
| 链接错误 | 缺少依赖库 | 检查依赖是否正确安装 |
| 中文乱码 | 字符编码问题 | 确保源文件是 UTF-8 编码 |
| 权限错误 | 当前用户没有写入权限 | 使用管理员权限或修改目录权限 |
