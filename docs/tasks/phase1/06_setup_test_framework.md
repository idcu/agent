# 任务 1.6: 搭建测试框架

## 目标

创建轻量级、易用的测试框架，支持 TDD（测试驱动开发）。

## 详细步骤

### 1. 创建测试框架目录结构

```bash
mkdir -p libs/idcu-testframework/include/idcu/testframework
mkdir -p libs/idcu-testframework/src
mkdir -p libs/idcu-testframework/tests
mkdir -p libs/idcu-testframework/examples
```

### 2. 创建测试框架头文件

```c
// libs/idcu-testframework/include/idcu/testframework/testframework.h
#ifndef IDCU_TESTFRAMEWORK_TESTFRAMEWORK_H
#define IDCU_TESTFRAMEWORK_TESTFRAMEWORK_H

#include <stdio.h>
#include <stdbool.h>

// 测试结果统计
typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
} idcu_TestStats;

// 测试函数类型
typedef void (*idcu_TestFunc)(void);

// 测试用例注册
void idcu_test_register(const char* suite_name, const char* test_name, idcu_TestFunc func);

// 断言宏
#define IDCU_TEST_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "ASSERT FAILED: %s:%d - %s\n", __FILE__, __LINE__, #cond); \
            idcu_test_fail_current(); \
            return; \
        } \
    } while (0)

#define IDCU_TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            fprintf(stderr, "ASSERT FAILED: %s:%d - %s != %s (expected %d, got %d)\n", \
                    __FILE__, __LINE__, #expected, #actual, (int)(expected), (int)(actual)); \
            idcu_test_fail_current(); \
            return; \
        } \
    } while (0)

#define IDCU_TEST_ASSERT_STRING_EQUAL(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            fprintf(stderr, "ASSERT FAILED: %s:%d - \"%s\" != \"%s\"\n", \
                    __FILE__, __LINE__, (expected), (actual)); \
            idcu_test_fail_current(); \
            return; \
        } \
    } while (0)

// 测试注册宏
#define IDCU_TEST_CASE(suite, name) \
    static void _test_##suite##_##name(void); \
    static void _register_test_##suite##_##name(void) __attribute__((constructor)); \
    static void _register_test_##suite##_##name(void) { \
        idcu_test_register(#suite, #name, _test_##suite##_##name); \
    } \
    static void _test_##suite##_##name(void)

// 内部函数
void idcu_test_fail_current(void);
int idcu_test_run_all(void);
idcu_TestStats idcu_test_get_stats(void);

#endif
```

### 3. 创建测试框架实现

```c
// libs/idcu-testframework/src/testframework.c
#include "idcu/testframework/testframework.h"
#include <stdlib.h>
#include <string.h>

#define MAX_TESTS 1024

typedef struct {
    const char* suite_name;
    const char* test_name;
    idcu_TestFunc func;
    bool failed;
} idcu_TestCase;

static idcu_TestCase g_tests[MAX_TESTS];
static int g_test_count = 0;
static int g_current_test = -1;
static idcu_TestStats g_stats = {0};

void idcu_test_register(const char* suite_name, const char* test_name, idcu_TestFunc func) {
    if (g_test_count >= MAX_TESTS) {
        fprintf(stderr, "Too many tests! Maximum is %d\n", MAX_TESTS);
        return;
    }
    
    g_tests[g_test_count].suite_name = suite_name;
    g_tests[g_test_count].test_name = test_name;
    g_tests[g_test_count].func = func;
    g_tests[g_test_count].failed = false;
    g_test_count++;
}

void idcu_test_fail_current(void) {
    if (g_current_test >= 0 && g_current_test < g_test_count) {
        g_tests[g_current_test].failed = true;
    }
}

int idcu_test_run_all(void) {
    printf("========================================\n");
    printf("Running %d tests...\n", g_test_count);
    printf("========================================\n\n");
    
    g_stats.total_tests = g_test_count;
    g_stats.passed_tests = 0;
    g_stats.failed_tests = 0;
    
    const char* current_suite = NULL;
    
    for (int i = 0; i < g_test_count; i++) {
        // 打印 suite 名称（如果变化）
        if (current_suite == NULL || strcmp(g_tests[i].suite_name, current_suite) != 0) {
            current_suite = g_tests[i].suite_name;
            printf("[Suite: %s]\n", current_suite);
        }
        
        // 运行测试
        printf("  Running: %s... ", g_tests[i].test_name);
        fflush(stdout);
        
        g_current_test = i;
        g_tests[i].failed = false;
        g_tests[i].func();
        
        if (g_tests[i].failed) {
            printf("FAILED\n");
            g_stats.failed_tests++;
        } else {
            printf("PASSED\n");
            g_stats.passed_tests++;
        }
    }
    
    // 打印总结
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("  Total:  %d\n", g_stats.total_tests);
    printf("  Passed: %d\n", g_stats.passed_tests);
    printf("  Failed: %d\n", g_stats.failed_tests);
    printf("========================================\n");
    
    return g_stats.failed_tests;
}

idcu_TestStats idcu_test_get_stats(void) {
    return g_stats;
}
```

### 4. 创建测试框架 CMakeLists.txt

```cmake
# libs/idcu-testframework/CMakeLists.txt

add_library(idcu_testframework STATIC
    src/testframework.c
)

target_include_directories(idcu_testframework PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(idcu::testframework ALIAS idcu_testframework)

# 示例
if(BUILD_EXAMPLES)
    add_executable(example_simple_test examples/simple_test.c)
    target_link_libraries(example_simple_test PRIVATE idcu::testframework)
endif()
```

### 5. 创建示例测试

```c
// libs/idcu-testframework/examples/simple_test.c
#include "idcu/testframework/testframework.h"
#include <string.h>

// 示例函数，我们将测试它
static int add(int a, int b) {
    return a + b;
}

static const char* get_greeting(void) {
    return "Hello, World!";
}

// 测试用例 1
IDCU_TEST_CASE(Math, AddPositiveNumbers) {
    IDCU_TEST_ASSERT_EQUAL(5, add(2, 3));
    IDCU_TEST_ASSERT_EQUAL(10, add(5, 5));
}

// 测试用例 2
IDCU_TEST_CASE(Math, AddNegativeNumbers) {
    IDCU_TEST_ASSERT_EQUAL(-1, add(2, -3));
    IDCU_TEST_ASSERT_EQUAL(-5, add(-2, -3));
}

// 测试用例 3
IDCU_TEST_CASE(String, Greeting) {
    IDCU_TEST_ASSERT_STRING_EQUAL("Hello, World!", get_greeting());
}

// 主函数
int main(void) {
    return idcu_test_run_all();
}
```

### 6. 创建测试框架的 README

```markdown
# idcu-testframework

轻量级 C 语言测试框架，支持 TDD。

## 快速开始

```c
#include "idcu/testframework/testframework.h"

IDCU_TEST_CASE(MySuite, MyTest) {
    IDCU_TEST_ASSERT_EQUAL(5, 2 + 3);
}

int main(void) {
    return idcu_test_run_all();
}
```

## API 参考

### 断言

- `IDCU_TEST_ASSERT(cond)` - 断言条件为真
- `IDCU_TEST_ASSERT_EQUAL(expected, actual)` - 断言两个值相等
- `IDCU_TEST_ASSERT_STRING_EQUAL(expected, actual)` - 断言两个字符串相等

### 测试注册

- `IDCU_TEST_CASE(suite, name)` - 定义一个测试用例

### 运行测试

- `idcu_test_run_all()` - 运行所有测试
```

### 7. 将测试框架添加到构建中

编辑 `libs/CMakeLists.txt`，添加：

```cmake
add_subdirectory(idcu-testframework)
```

## 验证检查清单

- [ ] 测试框架头文件已创建
- [ ] 测试框架实现已创建
- [ ] CMakeLists.txt 已创建
- [ ] 示例测试已创建
- [ ] 可以编译测试框架
- [ ] 可以运行示例测试
- [ ] 示例测试通过

## 运行测试

```bash
# 构建
cmake -B build -DBUILD_EXAMPLES=ON
cmake --build build

# 运行示例测试
./build/libs/idcu-testframework/example_simple_test
```

## Git 提交

```bash
git add libs/idcu-testframework/
git commit -m "feat(libs): add test framework

- Add test framework header
- Add test framework implementation
- Add example tests
- Add CMakeLists.txt
- Add README"

# 更新 libs/CMakeLists.txt
git add libs/CMakeLists.txt
git commit -m "chore: add testframework to build"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| constructor 属性不支持 | 编译器不支持该属性 | 使用手动注册方式 |
| 测试没有运行 | 测试用例没有正确注册 | 检查 IDCU_TEST_CASE 宏的使用 |

## 经验提示

- TDD 从这里开始！先写测试，再写代码
- 测试框架设计要简单易用
- 提供清晰的测试输出，便于定位问题
- 支持多种断言类型，满足不同测试需求
