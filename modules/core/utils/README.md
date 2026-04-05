# Utils 模块

## 模块信息

| 属性 | 值 |
|------|-----|
| **名称** | utils_module |
| **类别** | core |
| **版本** | 1.0.0 |
| **描述** | Utilities module providing logging, config management, JSON parser, memory pool, and permission management |
| **作者** | IDCU Team |
| **许可证** | MIT |
| **优先级** | high |
| **类型** | builtin |
| **依赖** | common_module |

## 功能说明

提供项目通用的工具组件，包括：
- 日志系统 (log)
- 配置管理 (config_manager)
- JSON 解析器 (json_parser)
- 内存池 (memory_pool)
- 权限管理 (permission_manager)

## API 文档

### 日志系统 (log.h)
- 多级别日志 (DEBUG/INFO/WARN/ERROR)
- 日志格式化
- 日志输出管理

### 配置管理 (config_manager.h)
- 配置文件读取
- 配置项查询
- 配置更新

### JSON 解析器 (json_parser.h)
- JSON 解析
- JSON 生成
- JSON 数据操作

### 内存池 (memory_pool.h)
- 高效内存分配
- 内存复用
- 内存泄漏检测

### 权限管理 (permission_manager.h)
- 权限定义
- 权限检查
- 权限管理

## 依赖关系

- idcu_core_common

## 使用示例

```c
#include "utils/log.h"
#include "utils/config_manager.h"
#include "utils/json_parser.h"
```
