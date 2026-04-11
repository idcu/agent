# idcu-security-module

IDCU Agent 的安全业务模块，提供：
- 沙箱安全
- 权限管理
- 数据加密和解密
- 访问控制
- 安全审计

## 使用方法

```c
#include <idcu/security_module/security_module.h>

int main() {
    idcu_SecurityModule sm;
    int ret = idcu_security_module_init(&sm);
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_security_module_start(&sm);
    if (ret != IDCU_ERR_OK) {
        idcu_security_module_destroy(&sm);
        return 1;
    }
    
    idcu_security_module_check_permission(&sm, "read_file");
    
    char data[] = "秘密数据";
    char encrypted[1024];
    size_t encrypted_size;
    idcu_security_module_encrypt(&sm, data, strlen(data), encrypted, &encrypted_size);
    
    idcu_security_module_stop(&sm);
    idcu_security_module_destroy(&sm);
    
    return 0;
}
```

## 许可证

本库采用 [Apache License 2.0](../../../LICENSE) 许可证。
