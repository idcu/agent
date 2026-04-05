# SDK 模块

## 模块信息

| 属性 | 值 |
|------|-----|
| **名称** | sdk_module |
| **类别** | core |
| **版本** | 1.0.0 |
| **描述** | SDK module providing simplified development interfaces and utility wrappers |
| **作者** | IDCU Team |
| **许可证** | MIT |
| **优先级** | medium |
| **类型** | builtin |
| **依赖** | common_module, utils_module |

## 功能说明

提供外部开发接口，封装底层复杂性，包括：
- 简化模块开发接口
- 常用功能封装
- 模块开发辅助工具

## API 文档

### SDK 接口 (sdk.h)
- 模块开发简化接口
- 消息发送/接收封装
- 常用工具函数

## 依赖关系

- idcu_core_common
- idcu_core_utils

## 使用示例

```c
#include "sdk/sdk.h"
```
