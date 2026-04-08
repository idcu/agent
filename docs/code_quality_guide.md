# Code Quality Guide

This guide covers the code quality tools and practices integrated into the IDCU Agent project as part of Phase 3.

## Table of Contents
1. [Code Formatting](#code-formatting)
2. [Static Analysis](#static-analysis)
3. [Memory Safety](#memory-safety)
4. [Thread Safety](#thread-safety)

## Code Formatting

We use `clang-format` for consistent code formatting across the project.

### Configuration
The formatting rules are defined in `.clang-format` at the project root.

### Usage

#### Check Format
Check if code is properly formatted without making changes:

**Windows:**
```cmd
scripts\check_format.bat
```

**Linux/macOS:**
```bash
chmod +x scripts/check_format.sh
scripts/check_format.sh
```

#### Auto Format
Automatically format all source files:

**Windows:**
```cmd
scripts\format.bat
```

**Linux/macOS:**
```bash
chmod +x scripts/format.sh
scripts/format.sh
```

## Static Analysis

We use `clang-tidy` for static code analysis to catch potential bugs, security issues, and code smells.

### Configuration
The analysis rules are defined in `.clang-tidy` at the project root.

### Prerequisites
- Install LLVM/Clang (includes clang-tidy)
  - **Windows**: Download from https://llvm.org/builds/
  - **Ubuntu/Debian**: `sudo apt-get install clang-tidy`
  - **macOS**: `brew install llvm`

### Usage

#### Step 1: Generate Compile Database
First, configure CMake with compile commands export:

```bash
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

#### Step 2: Run clang-tidy

**Windows:**
```cmd
scripts\run_clang_tidy.bat
```

**Linux/macOS:**
```bash
chmod +x scripts/run_clang_tidy.sh
scripts/run_clang_tidy.sh
```

#### Optional: CMake Integration
You can also enable clang-tidy during CMake configuration:

```bash
cmake -B build -DENABLE_CLANG_TIDY=ON
```

## Memory Safety

We use AddressSanitizer (ASAN) to detect memory issues like:
- Memory leaks
- Use-after-free
- Buffer overflows
- Use of uninitialized memory

### Prerequisites
- GCC or Clang compiler (MSVC not fully supported)

### Usage

#### Build with ASAN

**Windows:**
```cmd
scripts\build_with_asan.bat
```

**Linux/macOS:**
```bash
chmod +x scripts/build_with_asan.sh
scripts/build_with_asan.sh
```

#### Manual CMake Configuration
```bash
cmake -B build_asan -DENABLE_ADDRESS_SANITIZER=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build_asan --config Debug
```

#### Run the Program
When you run the built executable, ASAN will automatically detect and report memory issues.

## Thread Safety

We use ThreadSanitizer (TSAN) to detect data races and other threading issues.

### Prerequisites
- GCC or Clang compiler (MSVC not fully supported)

### Usage

#### Build with TSAN

**Windows:**
```cmd
scripts\build_with_tsan.bat
```

**Linux/macOS:**
```bash
chmod +x scripts/build_with_tsan.sh
scripts/build_with_tsan.sh
```

#### Manual CMake Configuration
```bash
cmake -B build_tsan -DENABLE_THREAD_SANITIZER=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build_tsan --config Debug
```

#### Run the Program
When you run the built executable, TSAN will automatically detect and report data race issues.

## CMake Options Summary

| Option | Description | Default |
|--------|-------------|---------|
| `ENABLE_CLANG_TIDY` | Enable clang-tidy during build | OFF |
| `ENABLE_ADDRESS_SANITIZER` | Enable AddressSanitizer | OFF |
| `ENABLE_THREAD_SANITIZER` | Enable ThreadSanitizer | OFF |

## Best Practices

1. **Format Code Before Committing**: Always run `format.bat` or `format.sh` before committing changes
2. **Run Static Analysis Regularly**: Use clang-tidy to catch issues early
3. **Test with Sanitizers**: Run tests with ASAN and TSAN before major releases
4. **Fix Warnings**: Treat warnings as errors and fix them promptly

## Troubleshooting

### clang-format not found
Install LLVM/Clang and make sure it's in your PATH.

### clang-tidy not finding compile_commands.json
Make sure you ran CMake with `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`.

### Sanitizer build fails
Ensure you're using GCC or Clang, not MSVC, for ASAN/TSAN builds.

### Encoding issues on Windows
All scripts use English to avoid encoding problems.
