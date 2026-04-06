# idcu-testframework

IDCU 项目的测试框架库，提供单元测试和集成测试支持。

## 特性

- 测试套件和测试用例管理
- 丰富的断言宏
- 测试状态报告
- 测试执行时间统计
- 测试结果汇总
- 跨平台支持（Windows、Linux、macOS）

## 快速开始

### 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 安装

```bash
cmake --install .
```

### 在其他项目中使用

```cmake
find_package(idcu-testframework REQUIRED)
add_executable(my_tests main.c)
target_link_libraries(my_tests PRIVATE idcu::testframework)
```

## API 文档

### 框架初始化和关闭

```c
#include <idcu/testframework/testframework.h>

void idcu_test_framework_init(void);
void idcu_test_framework_shutdown(void);
```

### 测试套件注册

```c
int idcu_test_suite_register(const char* name, idcu_TestSuiteSetupFunc setup, idcu_TestSuiteTeardownFunc teardown);
int idcu_test_suite_unregister(const char* name);
```

### 测试用例注册

```c
int idcu_test_register(const char* suite_name, const char* test_name, idcu_TestFunc func);
```

### 测试执行

```c
int idcu_test_run_all(void);
int idcu_test_run_suite(const char* suite_name);
int idcu_test_run_single(const char* suite_name, const char* test_name);
```

### 测试结果查询

```c
int idcu_test_get_suite_result(const char* suite_name, idcu_TestSuiteResult* result);
int idcu_test_get_total_result(size_t* total_tests, size_t* total_passed, size_t* total_failed, size_t* total_skipped);
```

### 断言和控制

```c
void idcu_test_assert(bool condition, const char* message, ...);
void idcu_test_assert_int_eq(int64_t expected, int64_t actual, const char* message, ...);
void idcu_test_assert_int_ne(int64_t expected, int64_t actual, const char* message, ...);
void idcu_test_assert_str_eq(const char* expected, const char* actual, const char* message, ...);
void idcu_test_assert_str_ne(const char* expected, const char* actual, const char* message, ...);
void idcu_test_assert_ptr_eq(const void* expected, const void* actual, const char* message, ...);
void idcu_test_assert_ptr_ne(const void* expected, const void* actual, const char* message, ...);
void idcu_test_fail(const char* message, ...);
void idcu_test_skip(const char* message, ...);
```

### 便捷宏

```c
#define IDCU_TEST_ASSERT(cond)
#define IDCU_TEST_ASSERT_EQ(expected, actual)
#define IDCU_TEST_ASSERT_STR_EQ(expected, actual)
#define IDCU_TEST_FAIL(msg)
```

## 测试

```bash
cd build
ctest
```

## 许可证

详见项目根目录的 LICENSE 文件。
