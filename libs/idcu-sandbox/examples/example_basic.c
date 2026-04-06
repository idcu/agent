#include "idcu/sandbox/sandbox.h"
#include &lt;stdio.h&gt;

int main(void) {
    idcu_Sandbox sb;
    
    printf("=== Sandbox Basic Example ===\n\n");
    
    // Initialize sandbox
    int ret = idcu_sandbox_init(&amp;sb, 1001, IDCU_PERM_FILE | IDCU_PERM_NETWORK);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "Failed to initialize sandbox\n");
        return 1;
    }
    printf("Sandbox initialized for module 1001\n");
    
    // Check initial permissions
    printf("\nInitial permissions:\n");
    if (idcu_sandbox_perm_check(&amp;sb, IDCU_PERM_FILE) == IDCU_ERR_OK) {
        printf("  - File access: OK\n");
    }
    if (idcu_sandbox_perm_check(&amp;sb, IDCU_PERM_NETWORK) == IDCU_ERR_OK) {
        printf("  - Network access: OK\n");
    }
    if (idcu_sandbox_perm_check(&amp;sb, IDCU_PERM_RUN) != IDCU_ERR_OK) {
        printf("  - Run access: DENIED\n");
    }
    
    // Add a permission
    printf("\nAdding RUN permission...\n");
    idcu_sandbox_add_perm(&amp;sb, IDCU_PERM_RUN);
    if (idcu_sandbox_perm_check(&amp;sb, IDCU_PERM_RUN) == IDCU_ERR_OK) {
        printf("  - Run access: OK (now granted)\n");
    }
    
    // Set quota
    printf("\nSetting quota to 4096 bytes...\n");
    idcu_sandbox_set_quota(&amp;sb, 4096);
    uint32_t quota = idcu_sandbox_get_quota(&amp;sb);
    printf("  - Current quota: %u bytes\n", quota);
    
    // Remove a permission
    printf("\nRemoving NETWORK permission...\n");
    idcu_sandbox_remove_perm(&amp;sb, IDCU_PERM_NETWORK);
    if (idcu_sandbox_perm_check(&amp;sb, IDCU_PERM_NETWORK) != IDCU_ERR_OK) {
        printf("  - Network access: DENIED (now revoked)\n");
    }
    
    // Show current permissions
    uint32_t current_perm = idcu_sandbox_get_perm(&amp;sb);
    printf("\nCurrent permission mask: 0x%08X\n", current_perm);
    
    // Cleanup
    idcu_sandbox_destroy(&amp;sb);
    printf("\nSandbox destroyed\n");
    
    return 0;
}
