# Test - 测试框架

本模块提供单元测试和集成测试的框架。

## 模块内容

### test_framework.h/c - 测试框架
轻量级的 C 语言测试框架：
- 测试套件 (idcu_TestSuite) 管理
- 测试用例 (idcu_TestCase) 添加和执行
- 测试结果统计 (通过、失败)
- 测试断言宏
- 测试总结输出

## 使用示例

```c
#include "test/test_framework.h"

static void test_example(void) {
    int a = 1;
    int b = 2;
    IDCU_TEST_ASSERT(a + b == 3, "加法测试失败");
    IDCU_TEST_PASS();
}

static void test_another_example(void) {
    const char* str = "hello";
    IDCU_TEST_ASSERT(str != NULL, "字符串不应为 NULL");
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_TestSuite suite;
    idcu_test_suite_init(&suite, "我的测试套件");
    
    // 添加测试用例
    idcu_test_suite_add_test(&suite, "测试示例", test_example);
    idcu_test_suite_add_test(&suite, "另一个测试", test_another_example);
    
    // 运行测试
    idcu_test_suite_run(&suite);
    
    // 打印总结
    idcu_test_suite_print_summary(&suite);
    
    // 检查是否有失败
    int failures = idcu_test_suite_get_failures(&suite);
    
    idcu_test_suite_destroy(&suite);
    return failures > 0 ? 1 : 0;
}
```

## 测试宏

| 宏 | 说明 |
|----|------|
| IDCU_TEST_PASS() | 标记测试通过 |
| IDCU_TEST_FAIL(msg) | 标记测试失败，带消息 |
| IDCU_TEST_ASSERT(cond, msg) | 断言条件为真，否则失败 |
