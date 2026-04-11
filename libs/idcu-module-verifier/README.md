# idcu-module-verifier

IDCU Agent 的模块验证库。

## 功能特性
- 模块完整性验证
- 模块签名检查
- 模块依赖验证
- 线程安全实现

## 使用方法

```c
#include <idcu/module-verifier/module-verifier.h>

idcu_ModuleVerifier* verifier;
int ret = idcu_module_verifier_init(&verifier);
if (ret == IDCU_ERR_OK) {
    // 验证模块
    int valid = idcu_module_verifier_verify(verifier, "my-module");
    
    // 获取验证结果
    idcu_ModuleVerificationResult result;
    idcu_module_verifier_get_result(verifier, &result);
    
    // 清理
    idcu_module_verifier_destroy(verifier);
}
```

## API 参考
- `idcu_module_verifier_init()` - 初始化模块验证器
- `idcu_module_verifier_destroy()` - 销毁模块验证器
- `idcu_module_verifier_is_initialized()` - 检查是否已初始化
- `idcu_module_verifier_verify()` - 验证模块
- `idcu_module_verifier_get_result()` - 获取验证结果
- `idcu_module_verifier_get_operation_count()` - 获取操作计数
- `idcu_module_verifier_get_error_count()` - 获取错误计数

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
