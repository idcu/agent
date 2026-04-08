#include "idcu/sandbox/sandbox.h"
#include &lt; stdio.h & gt;

int main(void) {
    idcu_Sandbox sb_web, sb_file, sb_db;

    printf("=== Sandbox Registry Example ===\n\n");

    // Initialize sandboxes
    idcu_sandbox_init(&amp; sb_web, 1, IDCU_PERM_NETWORK | IDCU_PERM_SEND | IDCU_PERM_RECV);
    idcu_sandbox_init(&amp; sb_file, 2, IDCU_PERM_FILE | IDCU_PERM_RUN);
    idcu_sandbox_init(&amp; sb_db, 3, IDCU_PERM_FILE | IDCU_PERM_HW);

    // Initialize registry
    printf("Initializing sandbox registry...\n");
    idcu_sandbox_registry_init();

    // Register sandboxes
    printf("\nRegistering sandboxes...\n");
    idcu_sandbox_registry_add(&amp; sb_web);
    idcu_sandbox_registry_add(&amp; sb_file);
    idcu_sandbox_registry_add(&amp; sb_db);
    printf("  - Web module (1) registered\n");
    printf("  - File module (2) registered\n");
    printf("  - DB module (3) registered\n");

    // Check permissions via registry
    printf("\nChecking permissions via registry:\n");
    if (idcu_sandbox_registry_check_perm(1, IDCU_PERM_NETWORK) == IDCU_ERR_OK) {
        printf("  - Module 1 has network access\n");
    }
    if (idcu_sandbox_registry_check_perm(2, IDCU_PERM_FILE) == IDCU_ERR_OK) {
        printf("  - Module 2 has file access\n");
    }
    if (idcu_sandbox_registry_check_perm(3, IDCU_PERM_HW) == IDCU_ERR_OK) {
        printf("  - Module 3 has hardware access\n");
    }

    // Get a sandbox from registry
    printf("\nGetting sandbox for module 2...\n");
    idcu_Sandbox *sb_ptr = idcu_sandbox_registry_get(2);
    if (sb_ptr) {
        uint32_t perm = idcu_sandbox_get_perm(sb_ptr);
        printf("  - Module 2 permissions: 0x%08X\n", perm);
    }

    // Remove a sandbox
    printf("\nRemoving module 3 from registry...\n");
    idcu_sandbox_registry_remove(3);

    // Try to get removed sandbox
    sb_ptr = idcu_sandbox_registry_get(3);
    if (!sb_ptr) {
        printf("  - Module 3 is no longer in registry\n");
    }

    // Cleanup
    printf("\nCleaning up...\n");
    idcu_sandbox_registry_destroy();
    idcu_sandbox_destroy(&amp; sb_web);
    idcu_sandbox_destroy(&amp; sb_file);
    idcu_sandbox_destroy(&amp; sb_db);

    printf("Done!\n");
    return 0;
}
