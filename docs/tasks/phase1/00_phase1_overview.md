# 第一阶段：项目初始化和基础构建

## 阶段里程碑

完成本阶段后，你应该能够：
- ✅ 运行 Hello World 程序
- ✅ 使用 CMake 成功构建项目
- ✅ 使用 idcu-module-build 创建新模块
- ✅ 运行简单的测试

## 任务列表

| 序号 | 任务 | 状态 | 预计时间 | 依赖 |
|-----|------|------|---------|------|
| 1.1 | [创建项目目录结构](./01_create_project_structure.md) | ⏳ 待开始 | 30分钟 | 无 |
| 1.2 | [编写主程序入口](./02_write_main_entry.md) | ⏳ 待开始 | 1小时 | 1.1 |
| 1.3 | [配置 CMake 构建系统](./03_configure_cmake.md) | ⏳ 待开始 | 1.5小时 | 1.2 |
| 1.4 | [验证项目可以编译](./04_verify_build.md) | ⏳ 待开始 | 1小时 | 1.3 |
| 1.5 | [添加代码质量工具](./05_add_code_quality_tools.md) | ⏳ 待开始 | 1小时 | 1.4 |
| 1.6 | [搭建测试框架](./06_setup_test_framework.md) | ⏳ 待开始 | 2小时 | 1.5 |
| 1.7 | [初始化 idcu-module-build](./07_init_module_build.md) | ⏳ 待开始 | 2小时 | 1.6 |

## 完整验收检查清单

在进入阶段 2 之前，请确认：

- [ ] **1.1 - 创建项目结构**
  - [ ] 所有目录已创建
  - [ ] .gitignore 已配置
  - [ ] README.md 已创建
  - [ ] 已提交 Git

- [ ] **1.2 - 编写主程序入口**
  - [ ] main.c 已创建
  - [ ] 支持命令行参数（--help, --version）
  - [ ] 代码已格式化
  - [ ] 已提交 Git

- [ ] **1.3 - 创建 CMake 构建配置**
  - [ ] 根 CMakeLists.txt 已创建
  - [ ] 支持 Debug 和 Release 构建
  - [ ] 有 BUILD_TESTS 和 BUILD_EXAMPLES 选项
  - [ ] 有配置摘要输出
  - [ ] 已提交 Git

- [ ] **1.4 - 验证项目可以编译**
  - [ ] Debug 版本编译成功
  - [ ] Release 版本编译成功
  - [ ] 程序可以正常运行
  - [ ] 命令行参数工作正常
  - [ ] 在至少一个平台上验证过

- [ ] **1.5 - 添加代码质量工具**
  - [ ] .clang-format 已创建
  - [ ] .clang-tidy 已创建
  - [ ] format 脚本已创建
  - [ ] check-format 脚本已创建
  - [ ] 现有代码已格式化
  - [ ] 已提交 Git

- [ ] **1.6 - 搭建测试框架**
  - [ ] 测试框架已创建
  - [ ] 有断言宏
  - [ ] 有测试注册机制
  - [ ] 有示例测试
  - [ ] 示例测试可以运行并通过
  - [ ] 已提交 Git

- [ ] **1.7 - 初始化 idcu-module-build**
  - [ ] 目录结构已创建
  - [ ] 核心 CMake 模块已创建
  - [ ] 有 README 文档
  - [ ] 已提交 Git

## 阶段完成演示

运行以下命令展示成果：

```bash
# 1. 清理并重新构建
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON
cmake --build build -j4

# 2. 运行主程序
echo "=== Running main program ==="
./build/bin/idcu_agent --version
./build/bin/idcu_agent --help

# 3. 运行测试框架示例
echo ""
echo "=== Running test framework example ==="
./build/libs/idcu-testframework/example_simple_test

# 4. 检查代码格式
echo ""
echo "=== Checking code format ==="
scripts/windows/check_format.bat  # 或 scripts/linux/check_format.sh

echo ""
echo "========================================"
echo "Phase 1 is COMPLETE! 🎉"
echo "========================================"
```

## Git 标签（可选但推荐）

如果想标记阶段 1 完成：

```bash
git tag -a v0.1.0-phase1 -m "Complete Phase 1: Project initialization and basic build"
git push origin v0.1.0-phase1
```

---

**恭喜！你已完成阶段 1！现在可以进入阶段 2 了！** 🚀
