# 任务 1.2: 编写主程序入口

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建简单但可扩展的主程序入口，为后续集成微内核做准备。支持命令行参数解析（--help、--version），输出欢迎信息，为将来扩展预留接口。

### 1.2 不做什么
- 不实现微内核功能（仅预留接口）
- 不实现复杂的业务逻辑
- 不集成第三方库（仅使用标准 C 库）

### 1.3 输入
- 命令行参数：-h/--help、-v/--version

### 1.4 输出
- 控制台输出：帮助信息、版本信息、欢迎消息
- 返回码：0（成功）、1（失败）

### 1.5 前置依赖
- 任务 1.1 已完成（项目目录结构已创建）
- C 编译器可用（GCC、Clang 或 MSVC）

---

## 2. 技术实现方案

### 2.1 核心选型
- C 语言标准库（stdio.h、stdlib.h、string.h）
- 命令行参数手动解析（避免引入复杂依赖）

### 2.2 核心逻辑
```
1. 解析命令行参数
2. 如果是 --help/-h，输出帮助信息并退出
3. 如果是 --version/-v，输出版本信息并退出
4. 否则，输出欢迎信息
5. 进入等待循环（可通过 Ctrl+C 退出）
```

### 2.3 数据结构/接口
版本信息定义：
```c
#define IDCU_AGENT_VERSION_MAJOR 0
#define IDCU_AGENT_VERSION_MINOR 1
#define IDCU_AGENT_VERSION_PATCH 0
#define IDCU_AGENT_VERSION_STRING "0.1.0"

const char* idcu_agent_get_version(void);
```

### 2.4 跨平台适配
- 使用标准 C 库函数，确保跨平台兼容
- Windows 和 Linux 下的 Ctrl+C 处理由操作系统默认行为处理
- 换行符使用 `\n`，标准库会自动处理

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] `./idcu_agent --help` 输出正确的帮助信息
- [ ] `./idcu_agent --version` 输出版本号 "0.1.0"
- [ ] 无参数运行时输出欢迎消息
- [ ] 可以通过 Ctrl+C 正常退出
- [ ] 未知参数时输出错误信息并显示帮助

### 3.2 性能验收
- 程序启动时间 ≤ 100ms
- 内存占用 ≤ 1MB

### 3.3 异常验收
- [ ] 无效参数时返回非零退出码
- [ ] 语法错误时编译器给出明确提示

---

## 4. 执行计划

### 4.1 工期
1 小时/人

### 4.2 里程碑
- D2-00: 完成 main.c 基础框架
- D2-15: 完成命令行参数解析
- D2-30: 完成版本信息模块
- D2-45: 完成测试验证
- D2-60: 完成代码格式化和提交

### 4.3 人力
1 人（技能要求：C 语言基础）

---

## 5. 工程化要求

### 5.1 编码规范
- 遵循项目 .clang-format 规范
- 函数名使用小写+下划线（snake_case）
- 宏定义使用大写+下划线（UPPER_SNAKE_CASE）
- 每行代码不超过 100 字符

### 5.2 测试要求
- 手动测试所有命令行参数
- 验证程序可以正常启动和退出
- 验证返回码正确

### 5.3 部署指引
- 编译命令：由 CMake 统一管理（见任务 1.3）
- 可执行文件位置：build/bin/idcu_agent

---

## 6. 风险与应对

### 6.1 风险1
描述：编译器版本不兼容  
应对：使用 C99 标准，兼容性好

### 6.2 风险2
描述：命令行参数解析错误  
应对：充分测试各种参数组合

---

## 7. 详细实现步骤

### 1. 创建主程序文件

```c
// app/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "version.h"

static void print_version(void) {
    printf("IDCU Agent %s\n", idcu_agent_get_version());
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

---

## 8. 验证检查清单

- [ ] 代码已创建
- [ ] 语法检查通过（可以用编译器检查）
- [ ] 代码已通过 clang-format 格式化
- [ ] 可以正常编译
- [ ] 程序可以运行
- [ ] --help 参数工作正常
- [ ] --version 参数工作正常
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add app/
git commit -m "feat(app): add main program entry

- Add main.c with command line parsing
- Add version information
- Add help message"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 编译错误 | 头文件路径问题 | 检查 include 目录是否正确设置 |
| 运行时找不到版本字符串 | 链接问题 | 确保 version.c 被正确编译和链接 |
