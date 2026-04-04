# Plugin - 插件系统

本模块提供插件生态系统的管理功能。

## 模块内容

### plugin_ecosystem.h/c - 插件生态系统
完整的插件管理系统：
- 插件信息管理 (名称、版本、描述、状态)
- 插件状态 (AVAILABLE, INSTALLED, ACTIVE, ERROR)
- 插件依赖管理
- 插件评分系统
- 版本兼容性检查
- 最大支持 64 个插件

## 插件状态

| 状态 | 说明 |
|------|------|
| IDCU_PLUGIN_STATUS_AVAILABLE | 插件可用但未安装 |
| IDCU_PLUGIN_STATUS_INSTALLED | 插件已安装但未激活 |
| IDCU_PLUGIN_STATUS_ACTIVE | 插件正在运行 |
| IDCU_PLUGIN_STATUS_ERROR | 插件出错 |

## 使用示例

```c
#include "plugin/plugin_ecosystem.h"

idcu_PluginEcosystem eco;
idcu_plugin_ecosystem_init(&eco);

// 添加插件
idcu_plugin_ecosystem_add_plugin(&eco, "my_plugin", "1.0.0", "我的测试插件");

// 安装和激活插件
idcu_plugin_ecosystem_install_plugin(&eco, "my_plugin");
idcu_plugin_ecosystem_activate_plugin(&eco, "my_plugin");

// 设置插件依赖
idcu_PluginDependency deps[] = {
    {"dependency_a", "2.0.0", 2, 0, 0},
    {"dependency_b", "1.5.0", 1, 5, 0}
};
idcu_plugin_ecosystem_set_dependencies(&eco, "my_plugin", deps, 2);

// 检查版本兼容性
idcu_plugin_ecosystem_check_compatibility(&eco, "my_plugin", ">=1.0.0");

// 给插件评分
idcu_plugin_ecosystem_rate_plugin(&eco, "my_plugin", 4.5f);

// 查找插件
idcu_PluginInfo* info = idcu_plugin_ecosystem_find_plugin(&eco, "my_plugin");
if (info) {
    printf("插件: %s, 版本: %s\n", info->name, info->version);
}

idcu_plugin_ecosystem_destroy(&eco);
```
