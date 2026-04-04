# Module System 模块

## 功能说明

模块管理系统，提供：
- 模块定义
- 模块注册表
- 模块分类管理
- 动态模块加载
- 模块版本管理

## API 文档

### 模块定义 (module_def.h)
- 模块接口定义
- 模块状态管理
- 模块注册宏

### 模块注册表 (module_registry.h)
- 模块注册和管理
- 依赖关系图构建
- 拓扑排序
- 配置应用

### 模块分类 (module_category.h)
- 模块分类和层级管理
- 模块配置加载

### 动态模块 (dynamic_module.h)
- 运行时加载 DLL/SO
- 热插拔支持

### 模块版本 (module_version.h)
- 版本号解析和比较
- 依赖版本检查

## 依赖关系

- idcu_core_common
- idcu_core_utils

## 使用示例

```c
#include "module/module_def.h"
#include "module/module_registry.h"
```
