# Test Framework 模块

## 模块信息

| 属性 | 值 |
|------|-----|
| **名称** | test_framework_module |
| **类别** | core |
| **版本** | 1.0.0 |
| **描述** | Test framework module providing unit test management, assertions, and report generation |
| **作者** | IDCU Team |
| **许可证** | MIT |
| **优先级** | medium |
| **类型** | builtin |
| **依赖** | common_module, utils_module |

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
