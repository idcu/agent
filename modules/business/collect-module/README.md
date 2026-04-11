# idcu-collect-module

IDCU Agent 的采集业务模块，提供：
- 从各种来源采集数据
- 设备数据采集
- 网络数据采集
- 数据聚合
- 数据过滤

## 使用方法

```c
#include <idcu/collect_module/collect_module.h>

int main() {
    idcu_CollectModule cm;
    int ret = idcu_collect_module_init(&cm);
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_collect_module_start(&cm);
    if (ret != IDCU_ERR_OK) {
        idcu_collect_module_destroy(&cm);
        return 1;
    }
    
    idcu_collect_module_stop(&cm);
    idcu_collect_module_destroy(&cm);
    
    return 0;
}
```

## 许可证

本库采用 [Apache License 2.0](../../../LICENSE) 许可证。
