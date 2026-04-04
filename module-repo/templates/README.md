# 模块开发模板

本目录包含 IDCU Agent 模块开发的模板和指南。

## 模块结构

每个模块应包含以下基本生命周期函数：

```c
#include <stdio.h>

static int module_init(void) {
    // 初始化模块
    printf("[module] initialized\n");
    return 0;
}

static int module_run(void) {
    // 运行模块（主循环中反复调用）
    return 0;
}

static int module_stop(void) {
    // 停止模块
    printf("[module] stopped\n");
    return 0;
}
```

## 模板文件

- `module_template.c` - 基础模块模板

## 使用说明

1. 复制模板文件到目标位置
2. 修改模块名称和描述
3. 实现 init/run/stop 函数
4. 添加业务逻辑
5. 创建 CMakeLists.txt

## 注意事项

- 所有公共符号使用 `idcu_` 前缀
- 遵循项目的代码风格
- 添加必要的错误处理
- 参考现有模块的实现方式
