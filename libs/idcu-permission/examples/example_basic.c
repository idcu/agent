#include "idcu/permission/permission.h"
#include &lt;stdio.h&gt;

int main(void) {
    int ret = idcu_permission_manager_init();
    if (ret != IDCU_ERR_SUCCESS) {
        fprintf(stderr, "Failed to initialize permission manager\n");
        return 1;
    }
    
    printf("Permission manager initialized\n");
    
    // Grant some permissions
    idcu_permission_grant("web_server", "network.listen");
    idcu_permission_grant("web_server", "network.accept");
    idcu_permission_grant("file_manager", "file.read");
    idcu_permission_grant("file_manager", "file.write");
    
    printf("Permissions granted\n");
    
    // Check permissions
    if (idcu_permission_check("web_server", "network.listen")) {
        printf("web_server has network.listen permission\n");
    }
    
    if (!idcu_permission_check("web_server", "file.read")) {
        printf("web_server does NOT have file.read permission\n");
    }
    
    // Check any permission
    const char* perms[] = {"file.read", "file.write", "network.connect"};
    if (idcu_permission_check_any("file_manager", perms, 3)) {
        printf("file_manager has at least one of the permissions\n");
    }
    
    // Revoke a permission
    idcu_permission_revoke("file_manager", "file.write");
    printf("Revoked file.write from file_manager\n");
    
    if (!idcu_permission_check("file_manager", "file.write")) {
        printf("file_manager no longer has file.write permission\n");
    }
    
    // List modules
    char* modules[64];
    int module_count;
    idcu_permission_list_modules(modules, &amp;module_count);
    printf("\nRegistered modules (%d):\n", module_count);
    for (int i = 0; i &lt; module_count; i++) {
        printf("  - %s\n", modules[i]);
    }
    
    idcu_permission_manager_shutdown();
    printf("\nPermission manager shutdown\n");
    
    return 0;
}
