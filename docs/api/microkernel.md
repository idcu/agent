# idcu-microkernel API 文档

用于管理模块和应用程序生命周期的微内核。

## 微内核

```c
typedef struct idcu_MicroKernel idcu_MicroKernel;
```

## 内核配置

```c
typedef struct {
    const char* config_path;
    int enable_logging;
    int log_level;
    size_t max_modules;
} idcu_KernelConfig;
```

### 内核函数

```c
int idcu_kernel_init(idcu_MicroKernel** out_kernel, const idcu_KernelConfig* config);
void idcu_kernel_destroy(idcu_MicroKernel* kernel);

int idcu_kernel_start(idcu_MicroKernel* kernel);
int idcu_kernel_stop(idcu_MicroKernel* kernel);
int idcu_kernel_run(idcu_MicroKernel* kernel);

idcu_KernelState idcu_kernel_get_state(const idcu_MicroKernel* kernel);
```

## 内核状态

```c
typedef enum {
    IDCU_KERNEL_STOPPED,
    IDCU_KERNEL_STARTING,
    IDCU_KERNEL_RUNNING,
    IDCU_KERNEL_STOPPING,
    IDCU_KERNEL_ERROR
} idcu_KernelState;
```

## 模块管理

```c
int idcu_kernel_load_module(idcu_MicroKernel* kernel, const char* module_path);
int idcu_kernel_unload_module(idcu_MicroKernel* kernel, const char* module_name);

int idcu_kernel_get_module_count(idcu_MicroKernel* kernel);
int idcu_kernel_get_module_info(idcu_MicroKernel* kernel, size_t index, idcu_ModuleInfo* out_info);
```

## 配置

```c
int idcu_kernel_reload_config(idcu_MicroKernel* kernel);
int idcu_kernel_get_config(idcu_MicroKernel* kernel, idcu_KernelConfig* out_config);
```

## 示例

```c
#include <idcu/microkernel/kernel.h>
#include <stdio.h>

int main(void) {
    idcu_MicroKernel* kernel = NULL;
    
    idcu_KernelConfig config = {
        .config_path = NULL,
        .enable_logging = 1,
        .log_level = IDCU_LOG_INFO,
        .max_modules = 32
    };
    
    // 初始化内核
    int ret = idcu_kernel_init(&kernel, &config);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "初始化内核失败\n");
        return 1;
    }
    
    printf("内核已初始化，状态: %d\n", idcu_kernel_get_state(kernel));
    
    // 启动内核
    ret = idcu_kernel_start(kernel);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "启动内核失败\n");
        idcu_kernel_destroy(kernel);
        return 1;
    }
    
    printf("内核已启动\n");
    
    // 运行主循环
    ret = idcu_kernel_run(kernel);
    
    // 停止并清理
    idcu_kernel_stop(kernel);
    idcu_kernel_destroy(kernel);
    
    printf("内核关闭完成\n");
    return 0;
}
```
