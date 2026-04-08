#include "idcu/permission/permission.h"
#include &lt; stdio.h & gt;

int main(void) {
    idcu_permission_manager_init();

    // Grant wildcard permission
    idcu_permission_grant("admin_module", "file");
    printf("Granted 'file' permission to admin_module\n");

    // Check specific permissions
    if (idcu_permission_check("admin_module", "file.read")) {
        printf("admin_module has file.read permission (wildcard match)\n");
    }

    if (idcu_permission_check("admin_module", "file.write")) {
        printf("admin_module has file.write permission (wildcard match)\n");
    }

    if (idcu_permission_check("admin_module", "file.delete")) {
        printf("admin_module has file.delete permission (wildcard match)\n");
    }

    // Grant another module with specific permissions
    idcu_permission_grant("user_module", "file.read");
    printf("\nGranted 'file.read' to user_module\n");

    if (idcu_permission_check("user_module", "file.read")) {
        printf("user_module has file.read permission\n");
    }

    if (!idcu_permission_check("user_module", "file.write")) {
        printf("user_module does NOT have file.write permission\n");
    }

    idcu_permission_manager_shutdown();

    return 0;
}
