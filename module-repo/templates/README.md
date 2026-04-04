# 模块开发模板

本目录包含 IDCU Agent 模块开发的模板和指南。

## 模块结构

每个模块应包含以下函数：

```c
#include "module/module_def.h"

static int module_init(void) {
    // 初始化模块
    return IDCU_ERR_OK;
}

static int module_start(void) {
    // 启动模块
    return IDCU_ERR_OK;
}

static int module_stop(void) {
    // 停止模块
    return IDCU_ERR_OK;
}

static void module_destroy(void) {
    // 清理资源
}

IDCU_MODULE_DEFINE(
    "module_name",
    "1.0.0",
    "Module description",
    module_init,
    module_start,
    module_stop,
    module_destroy
);
```

## 模板文件

- `module_template.c` - 基础模块模板

## 使用说明

1. 复制模板文件
2. 修改模块名称和描述
3. 实现 init/start/stop/destroy 函数
4. 添加业务逻辑

## 注意事项

- 所有公共符号使用 `idcu_` 前缀
- 遵循项目的代码风格
- 添加必要的错误处理
