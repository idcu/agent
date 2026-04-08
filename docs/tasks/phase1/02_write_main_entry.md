# 任务 1.2: 编写主程序入口

## 目标

创建简单但可扩展的主程序入口，为后续集成微内核做准备。

## 详细步骤

### 1. 创建主程序文件

```c
// app/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 版本信息
#define IDCU_AGENT_VERSION_MAJOR 0
#define IDCU_AGENT_VERSION_MINOR 1
#define IDCU_AGENT_VERSION_PATCH 0

static void print_version(void) {
    printf("IDCU Agent v%d.%d.%d\n",
           IDCU_AGENT_VERSION_MAJOR,
           IDCU_AGENT_VERSION_MINOR,
           IDCU_AGENT_VERSION_PATCH);
}

static void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help     Show this help message\n");
    printf("  -v, --version  Show version information\n");
    printf("\n");
}

int main(int argc, char* argv[]) {
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        const char* arg = argv[i];
        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--version") == 0) {
            print_version();
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "Unknown option: %s\n", arg);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    // 正常启动
    printf("========================================\n");
    print_version();
    printf("========================================\n");
    printf("Hello, IDCU Agent!\n");
    printf("System is starting...\n");

    // TODO: 这里将来会集成微内核
    // 目前只是简单的 Hello World

    printf("System started successfully!\n");
    printf("Press Ctrl+C to exit...\n");

    // 简单的等待循环
    while (1) {
        // 什么都不做，等待用户中断
    }

    return EXIT_SUCCESS;
}
```

### 2. 创建头文件（版本信息）

```bash
mkdir -p app/include
```

```c
// app/include/version.h
#ifndef IDCU_APP_VERSION_H
#define IDCU_APP_VERSION_H

#define IDCU_AGENT_VERSION_MAJOR 0
#define IDCU_AGENT_VERSION_MINOR 1
#define IDCU_AGENT_VERSION_PATCH 0

#define IDCU_AGENT_VERSION_STRING "0.1.0"

const char* idcu_agent_get_version(void);

#endif
```

```c
// app/src/version.c
#include "version.h"

const char* idcu_agent_get_version(void) {
    return IDCU_AGENT_VERSION_STRING;
}
```

### 3. 更新 app/main.c 使用版本头文件

```c
// app/main.c (更新版本部分)
#include "version.h"

static void print_version(void) {
    printf("IDCU Agent %s\n", idcu_agent_get_version());
}
```

## 验证检查清单

- [ ] 代码已创建
- [ ] 语法检查通过（可以用编译器检查）
- [ ] 代码已通过 clang-format 格式化
- [ ] 可以正常编译
- [ ] 程序可以运行
- [ ] --help 参数工作正常
- [ ] --version 参数工作正常

## Git 提交（任务完成后）

```bash
git add app/
git commit -m "feat(app): add main program entry

- Add main.c with command line parsing
- Add version information
- Add help message"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 编译错误 | 头文件路径问题 | 检查 include 目录是否正确设置 |
| 运行时找不到版本字符串 | 链接问题 | 确保 version.c 被正确编译和链接 |

## 经验提示

- 保持代码简单，不要过度设计
- 为将来扩展预留接口（如 TODO 注释所示）
- 使用版本号管理，便于后续追踪
