# Test Framework 模块

## 功能说明

提供单元测试框架支持，包括：
- 测试用例管理
- 测试断言
- 测试报告生成

## API 文档

### 测试框架 (test_framework.h)
- 测试用例注册
- 测试断言宏
- 测试运行器
- 测试结果统计

## 依赖关系

- idcu_core_common
- idcu_core_utils

## 使用示例

```c
#include "test/test_framework.h"

TEST_CASE(my_test) {
    TEST_ASSERT(1 == 1);
}
```
