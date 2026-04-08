# IDCU Agent 代码质量工具使用指南

本文档介绍 IDCU Agent 项目的代码质量工具链使用方法。

## 目录

- [代码格式化](#代码格式化)
- [静态分析](#静态分析)
- [内存检测](#内存检测)
- [测试覆盖](#测试覆盖)

---

## 代码格式化

### clang-format

项目使用 clang-format 来统一代码风格。

#### 配置文件

- `.clang-format` - 主配置文件，基于 Google 风格

#### 格式化所有代码

**Windows:**
```cmd
scripts\format.bat
```

或使用新创建的一键格式化脚本：
```cmd
format_all.bat
```

**Linux/macOS:**
```bash
scripts/format.sh
```

#### 检查格式（不修改文件）

**Windows:**
```cmd
scripts\check_format.bat
```

**Linux/macOS:**
```bash
scripts/check_format.sh
```

#### 手动格式化单个文件

```bash
clang-format -i path/to/file.c
```

---

## 静态分析

### clang-tidy

项目使用 clang-tidy 进行静态代码分析。

#### 配置文件

- `.clang-tidy` - 静态分析配置文件

#### 运行静态分析

**前置条件：**
1. 安装 LLVM/Clang 工具链
2. 生成编译数据库

**生成编译数据库：**
```bash
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

**运行静态分析：**

**Windows:**
```cmd
scripts\run_clang_tidy.bat
```

**Linux/macOS:**
```bash
scripts/run_clang_tidy.sh
```

#### 运行单个文件的静态分析

```bash
clang-tidy path/to/file.c -p build
```

#### 抑制特定警告

在代码中使用 `// NOLINT` 注释来抑制特定警告：

```c
#define MY_MACRO 123  // NOLINT(cppcoreguidelines-macro-to-enum)
```

---

## 内存检测

### AddressSanitizer (ASAN)

AddressSanitizer 用于检测内存错误，如内存泄漏、缓冲区溢出等。

**注意：** AddressSanitizer 需要 GCC 或 Clang 编译器，不支持 MSVC。

#### 使用 ASAN 编译

**Windows (MinGW):**
```bash
cmake -B build -DENABLE_ADDRESS_SANITIZER=ON
cmake --build build
```

**Linux/macOS:**
```bash
cmake -B build -DENABLE_ADDRESS_SANITIZER=ON
cmake --build build
```

#### 运行程序

运行编译后的程序，ASAN 会自动检测并报告内存错误。

#### 可用脚本

项目已提供便捷脚本：

**Windows:**
```cmd
scripts\build_with_asan.bat
```

**Linux/macOS:**
```bash
scripts/build_with_asan.sh
```

### ThreadSanitizer (TSAN)

ThreadSanitizer 用于检测数据竞争问题。

#### 使用 TSAN 编译

**Windows (MinGW):**
```bash
cmake -B build -DENABLE_THREAD_SANITIZER=ON
cmake --build build
```

**Linux/macOS:**
```bash
cmake -B build -DENABLE_THREAD_SANITIZER=ON
cmake --build build
```

#### 可用脚本

**Windows:**
```cmd
scripts\build_with_tsan.bat
```

**Linux/macOS:**
```bash
scripts/build_with_tsan.sh
```

---

## 测试覆盖

### 代码覆盖率

项目支持使用 gcov/lcov 生成代码覆盖率报告。

**注意：** 代码覆盖率需要 GCC 或 Clang 编译器，且仅支持 Unix-like 系统。

#### 生成覆盖率报告

```bash
cmake -B build -DBUILD_TESTS=ON -DENABLE_CODE_COVERAGE=ON
cmake --build build
cd build
make coverage
```

#### 查看报告

覆盖率报告会生成在 `build/coverage_report/` 目录下，用浏览器打开 `index.html` 即可查看。

---

## 工作流程建议

### 日常开发

1. **编码前**：确保了解项目的编码规范
2. **编码后**：
   - 运行 `format.bat` 格式化代码
   - 运行 `check_format.bat` 验证格式
   - 运行 `run_clang_tidy.bat` 进行静态分析
3. **提交前**：修复所有警告和错误

### 持续集成

项目的 GitHub Actions 工作流会自动：
- 检查代码格式
- 运行静态分析
- 运行测试
- 生成覆盖率报告（Linux）

---

## 工具安装

### Windows

#### LLVM/Clang

1. 下载 LLVM：https://llvm.org/builds/
2. 或使用 MSYS2：
   ```bash
   pacman -S mingw-w64-x86_64-clang
   ```

#### MinGW-w64

推荐使用 MSYS2：https://www.msys2.org/

### Linux (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install -y clang clang-format clang-tidy lcov
```

### macOS

```bash
brew install llvm
```

---

## 常见问题

### Q: clang-format 找不到怎么办？

A: 确保 LLVM/Clang 的 bin 目录已添加到 PATH 环境变量中。

### Q: 静态分析报告很多警告怎么办？

A: 
1. 优先修复高优先级警告
2. 对于 C 项目特有的模式，可以使用 `// NOLINT` 注释抑制
3. 参考 `.clang-tidy` 配置文件调整检查规则

### Q: AddressSanitizer 在 Windows 上不工作？

A: AddressSanitizer 在 Windows 上需要 MinGW-w64 或 Clang，不支持 MSVC。

---

## 参考资料

- [Clang-Format 文档](https://clang.llvm.org/docs/ClangFormat.html)
- [Clang-Tidy 文档](https://clang.llvm.org/extra/clang-tidy/)
- [AddressSanitizer 文档](https://github.com/google/sanitizers/wiki/AddressSanitizer)
- [ThreadSanitizer 文档](https://github.com/google/sanitizers/wiki/ThreadSanitizer)

---

**祝编码愉快！** 🚀
