#!/bin/bash
# 使用 ThreadSanitizer 构建项目 (Linux/macOS 版本)

echo "=== 使用 ThreadSanitizer 构建项目 ==="
echo ""

# 检查是否使用 GCC 或 Clang
echo "注意: ThreadSanitizer 需要 GCC 或 Clang 编译器"
echo ""

# 设置构建目录
BUILD_DIR="build_tsan"

# 清理旧的构建目录
if [ -d "$BUILD_DIR" ]; then
    echo "清理旧的构建目录..."
    rm -rf "$BUILD_DIR"
fi

# 创建构建目录
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 运行 CMake 并启用 ThreadSanitizer
echo "正在配置 CMake..."
cmake .. -DENABLE_THREAD_SANITIZER=ON -DCMAKE_BUILD_TYPE=Debug
if [ $? -ne 0 ]; then
    echo "CMake 配置失败"
    cd ..
    exit 1
fi

# 构建项目
echo ""
echo "正在构建项目..."
cmake --build . --config Debug
if [ $? -ne 0 ]; then
    echo "构建失败"
    cd ..
    exit 1
fi

cd ..
echo ""
echo "✅ 构建完成！"
echo ""
echo "可执行文件位于: $BUILD_DIR/bin/"
echo ""
echo "运行程序时，ThreadSanitizer 会自动检测数据竞争问题"
echo "建议运行并发相关的测试来检测数据竞争"
