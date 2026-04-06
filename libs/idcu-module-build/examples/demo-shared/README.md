# 通用 C 动态库示例

这是一个独立的通用 C 共享库示例，不依赖任何特定框架。

## 功能特性

- 简单的数学计算函数库
- 跨平台支持
- 不依赖第三方框架

## 目录结构

```
demo-shared/
├── include/
│   └── demo/
│       └── demo.h
├── src/
│   └── demo.c
├── CMakeLists.txt
├── module.json
└── README.md
```

## 构建

```bash
cd ../../
python scripts/build.py release --module examples/demo-shared
```

## 使用

```c
#include <demo/demo.h>

int main() {
    int result = demo_add(2, 3);
    printf("Result: %d\n", result);
    return 0;
}
```

## 说明

本示例展示了：
1. 如何创建独立的 C 共享库
2. 基础的 module.json 配置
3. 跨平台构建的最佳实践
