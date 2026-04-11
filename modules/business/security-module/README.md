# idcu-security-module

Security business module for IDCU Agent, providing:
- Sandbox security
- Permission management
- Data encryption and decryption
- Access control
- Security audit

## Usage

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
    
    char data[] = "secret data";
    char encrypted[1024];
    size_t encrypted_size;
    idcu_security_module_encrypt(&sm, data, strlen(data), encrypted, &encrypted_size);
    
    idcu_security_module_stop(&sm);
    idcu_security_module_destroy(&sm);
    
    return 0;
}
```
