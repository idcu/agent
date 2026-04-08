# IDCU Agent 快速构建指南

## 小白一键构建（推荐）

### Windows 用户
直接双击项目根目录下的 `一键构建.bat` 文件即可！

### Linux/macOS 用户
在项目根目录执行：
```bash
chmod +x 一键构建.sh
./一键构建.sh
```

---

## 详细构建步骤

### 环境要求
- CMake 3.14+
- C 编译器：
  - Windows：Visual Studio 2017+ 或 MinGW-w64
  - Linux：GCC 7.0+ 或 Clang 6.0+
  - macOS：Clang 6.0+

### 手动构建

#### Windows (MinGW)
```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
mingw32-make -j4
```

#### Windows (MSVC)
```bash
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
```

#### Linux/macOS
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4
```

---

## 输出说明

构建完成后，产物位于：
- 可执行文件：`build/bin/`
- 静态库：`build/lib/`
- 配置文件：`build/bin/config/`

---

## 构建选项

- `BUILD_TESTS`：构建测试（默认 OFF）
- `BUILD_EXAMPLES`：构建示例（默认 OFF）
- `BUILD_BENCHMARKS`：构建基准测试（默认 ON）

使用示例：
```bash
cmake -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON ..
```

---

## 已完成的优化

1. **简化了主 CMakeLists.txt**：结构更清晰，易于理解
2. **小白友好的一键构建脚本**：无需学习 CMake 即可编译
3. **逐步简化核心库的构建配置**：使用简洁直接的 CMake 语法
4. **保持与 idcu-module-build 的兼容性**：可以继续使用其功能
