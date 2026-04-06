#include "../include/sandbox.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <string.h>

static void test_sandbox_init_destroy(void) {
    idcu_Sandbox sb;
    
    int ret = idcu_sandbox_init(&sb, 1, IDCU_PERM_SEND | IDCU_PERM_RECV);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_init_destroy: FAIL - idcu_sandbox_init should succeed\n");
        return;
    }
    if (sb.module_id != 1) {
        printf("test_sandbox_init_destroy: FAIL - Module ID should be 1\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    if (sb.perm != (IDCU_PERM_SEND | IDCU_PERM_RECV)) {
        printf("test_sandbox_init_destroy: FAIL - Permissions should be SEND|RECV\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    if (sb.quota != 0) {
        printf("test_sandbox_init_destroy: FAIL - Quota should be 0 initially\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    idcu_sandbox_destroy(&sb);
    printf("test_sandbox_init_destroy: PASS\n");
}

static void test_sandbox_init_null_param(void) {
    int ret = idcu_sandbox_init(NULL, 1, 0);
    if (ret != IDCU_ERR_INVALID_PARAM) {
        printf("test_sandbox_init_null_param: FAIL - idcu_sandbox_init with NULL should fail\n");
        return;
    }
    printf("test_sandbox_init_null_param: PASS\n");
}

static void test_sandbox_perm_check(void) {
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, IDCU_PERM_SEND | IDCU_PERM_RECV);
    
    int ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_SEND);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_perm_check: FAIL - Should have PERM_SEND permission\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_RECV);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_perm_check: FAIL - Should have PERM_RECV permission\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_RUN);
    if (ret != IDCU_ERR_PERM_DENIED) {
        printf("test_sandbox_perm_check: FAIL - Should not have PERM_RUN permission\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_SEND | IDCU_PERM_RECV);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_perm_check: FAIL - Should have both SEND and RECV permissions\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_SEND | IDCU_PERM_RUN);
    if (ret != IDCU_ERR_PERM_DENIED) {
        printf("test_sandbox_perm_check: FAIL - Should not have both SEND and RUN permissions\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    idcu_sandbox_destroy(&sb);
    printf("test_sandbox_perm_check: PASS\n");
}

static void test_sandbox_perm_check_null_param(void) {
    int ret = idcu_sandbox_perm_check(NULL, IDCU_PERM_SEND);
    if (ret != IDCU_ERR_INVALID_PARAM) {
        printf("test_sandbox_perm_check_null_param: FAIL - idcu_sandbox_perm_check with NULL should fail\n");
        return;
    }
    printf("test_sandbox_perm_check_null_param: PASS\n");
}

static void test_sandbox_set_perm(void) {
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, 0);
    
    int ret = idcu_sandbox_set_perm(&sb, IDCU_PERM_RUN | IDCU_PERM_HW);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_set_perm: FAIL - idcu_sandbox_set_perm should succeed\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    if (sb.perm != (IDCU_PERM_RUN | IDCU_PERM_HW)) {
        printf("test_sandbox_set_perm: FAIL - Permissions should be RUN|HW\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_RUN);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_set_perm: FAIL - Should have PERM_RUN permission\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    idcu_sandbox_destroy(&sb);
    printf("test_sandbox_set_perm: PASS\n");
}

static void test_sandbox_set_perm_null_param(void) {
    int ret = idcu_sandbox_set_perm(NULL, IDCU_PERM_SEND);
    if (ret != IDCU_ERR_INVALID_PARAM) {
        printf("test_sandbox_set_perm_null_param: FAIL - idcu_sandbox_set_perm with NULL should fail\n");
        return;
    }
    printf("test_sandbox_set_perm_null_param: PASS\n");
}

static void test_sandbox_add_perm(void) {
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, IDCU_PERM_SEND);
    
    int ret = idcu_sandbox_add_perm(&sb, IDCU_PERM_RECV);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_add_perm: FAIL - idcu_sandbox_add_perm should succeed\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    if (sb.perm != (IDCU_PERM_SEND | IDCU_PERM_RECV)) {
        printf("test_sandbox_add_perm: FAIL - Permissions should be SEND|RECV\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    ret = idcu_sandbox_add_perm(&sb, IDCU_PERM_RUN);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_add_perm: FAIL - idcu_sandbox_add_perm should succeed again\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    if (sb.perm != (IDCU_PERM_SEND | IDCU_PERM_RECV | IDCU_PERM_RUN)) {
        printf("test_sandbox_add_perm: FAIL - Permissions should be SEND|RECV|RUN\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    idcu_sandbox_destroy(&sb);
    printf("test_sandbox_add_perm: PASS\n");
}

static void test_sandbox_add_perm_null_param(void) {
    int ret = idcu_sandbox_add_perm(NULL, IDCU_PERM_SEND);
    if (ret != IDCU_ERR_INVALID_PARAM) {
        printf("test_sandbox_add_perm_null_param: FAIL - idcu_sandbox_add_perm with NULL should fail\n");
        return;
    }
    printf("test_sandbox_add_perm_null_param: PASS\n");
}

static void test_sandbox_remove_perm(void) {
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, IDCU_PERM_SEND | IDCU_PERM_RECV | IDCU_PERM_RUN);
    
    int ret = idcu_sandbox_remove_perm(&sb, IDCU_PERM_RUN);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_remove_perm: FAIL - idcu_sandbox_remove_perm should succeed\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    if (sb.perm != (IDCU_PERM_SEND | IDCU_PERM_RECV)) {
        printf("test_sandbox_remove_perm: FAIL - Permissions should be SEND|RECV\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_RUN);
    if (ret != IDCU_ERR_PERM_DENIED) {
        printf("test_sandbox_remove_perm: FAIL - Should not have PERM_RUN permission after removal\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    idcu_sandbox_destroy(&sb);
    printf("test_sandbox_remove_perm: PASS\n");
}

static void test_sandbox_remove_perm_null_param(void) {
    int ret = idcu_sandbox_remove_perm(NULL, IDCU_PERM_SEND);
    if (ret != IDCU_ERR_INVALID_PARAM) {
        printf("test_sandbox_remove_perm_null_param: FAIL - idcu_sandbox_remove_perm with NULL should fail\n");
        return;
    }
    printf("test_sandbox_remove_perm_null_param: PASS\n");
}

static void test_sandbox_get_perm(void) {
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, IDCU_PERM_SEND | IDCU_PERM_FILE);
    
    uint32_t perm = idcu_sandbox_get_perm(&sb);
    if (perm != (IDCU_PERM_SEND | IDCU_PERM_FILE)) {
        printf("test_sandbox_get_perm: FAIL - get_perm should return correct permissions\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    perm = idcu_sandbox_get_perm(NULL);
    if (perm != 0) {
        printf("test_sandbox_get_perm: FAIL - get_perm with NULL should return 0\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    idcu_sandbox_destroy(&sb);
    printf("test_sandbox_get_perm: PASS\n");
}

static void test_sandbox_quota(void) {
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, 0);
    
    int ret = idcu_sandbox_set_quota(&sb, 1000);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_quota: FAIL - idcu_sandbox_set_quota should succeed\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    if (sb.quota != 1000) {
        printf("test_sandbox_quota: FAIL - Quota should be 1000\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    uint32_t quota = idcu_sandbox_get_quota(&sb);
    if (quota != 1000) {
        printf("test_sandbox_quota: FAIL - get_quota should return 1000\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    quota = idcu_sandbox_get_quota(NULL);
    if (quota != 0) {
        printf("test_sandbox_quota: FAIL - get_quota with NULL should return 0\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    idcu_sandbox_destroy(&sb);
    printf("test_sandbox_quota: PASS\n");
}

static void test_sandbox_set_quota_null_param(void) {
    int ret = idcu_sandbox_set_quota(NULL, 100);
    if (ret != IDCU_ERR_INVALID_PARAM) {
        printf("test_sandbox_set_quota_null_param: FAIL - idcu_sandbox_set_quota with NULL should fail\n");
        return;
    }
    printf("test_sandbox_set_quota_null_param: PASS\n");
}

static void test_sandbox_all_permissions(void) {
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, IDCU_PERM_ALL);
    
    int ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_SEND);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_all_permissions: FAIL - Should have SEND permission\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_RECV);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_all_permissions: FAIL - Should have RECV permission\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_RUN);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_all_permissions: FAIL - Should have RUN permission\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_HW);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_all_permissions: FAIL - Should have HW permission\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_FILE);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_all_permissions: FAIL - Should have FILE permission\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_NETWORK);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_all_permissions: FAIL - Should have NETWORK permission\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    idcu_sandbox_destroy(&sb);
    printf("test_sandbox_all_permissions: PASS\n");
}

static void test_sandbox_registry_init_destroy(void) {
    int ret = idcu_sandbox_registry_init();
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_registry_init_destroy: FAIL - idcu_sandbox_registry_init should succeed\n");
        return;
    }
    
    ret = idcu_sandbox_registry_init();
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_registry_init_destroy: FAIL - idcu_sandbox_registry_init should succeed when already initialized\n");
        idcu_sandbox_registry_destroy();
        return;
    }
    
    idcu_sandbox_registry_destroy();
    printf("test_sandbox_registry_init_destroy: PASS\n");
}

static void test_sandbox_registry_add_remove(void) {
    idcu_Sandbox sb1, sb2;
    idcu_sandbox_init(&sb1, 100, IDCU_PERM_SEND);
    idcu_sandbox_init(&sb2, 200, IDCU_PERM_RECV);
    
    idcu_sandbox_registry_init();
    
    int ret = idcu_sandbox_registry_add(&sb1);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_registry_add_remove: FAIL - Add sandbox 1 should succeed\n");
        idcu_sandbox_registry_destroy();
        idcu_sandbox_destroy(&sb1);
        idcu_sandbox_destroy(&sb2);
        return;
    }
    
    ret = idcu_sandbox_registry_add(&sb2);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_registry_add_remove: FAIL - Add sandbox 2 should succeed\n");
        idcu_sandbox_registry_destroy();
        idcu_sandbox_destroy(&sb1);
        idcu_sandbox_destroy(&sb2);
        return;
    }
    
    ret = idcu_sandbox_registry_add(&sb1);
    if (ret != IDCU_ERR_ALREADY_EXISTS) {
        printf("test_sandbox_registry_add_remove: FAIL - Add duplicate sandbox should fail\n");
        idcu_sandbox_registry_destroy();
        idcu_sandbox_destroy(&sb1);
        idcu_sandbox_destroy(&sb2);
        return;
    }
    
    ret = idcu_sandbox_registry_remove(100);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_registry_add_remove: FAIL - Remove sandbox 1 should succeed\n");
        idcu_sandbox_registry_destroy();
        idcu_sandbox_destroy(&sb1);
        idcu_sandbox_destroy(&sb2);
        return;
    }
    
    ret = idcu_sandbox_registry_remove(100);
    if (ret != IDCU_ERR_NOT_FOUND) {
        printf("test_sandbox_registry_add_remove: FAIL - Remove non-existent sandbox should fail\n");
        idcu_sandbox_registry_destroy();
        idcu_sandbox_destroy(&sb1);
        idcu_sandbox_destroy(&sb2);
        return;
    }
    
    idcu_sandbox_registry_destroy();
    idcu_sandbox_destroy(&sb1);
    idcu_sandbox_destroy(&sb2);
    printf("test_sandbox_registry_add_remove: PASS\n");
}

static void test_sandbox_registry_add_null_param(void) {
    idcu_sandbox_registry_init();
    
    int ret = idcu_sandbox_registry_add(NULL);
    if (ret != IDCU_ERR_INVALID_PARAM) {
        printf("test_sandbox_registry_add_null_param: FAIL - Add NULL sandbox should fail\n");
        idcu_sandbox_registry_destroy();
        return;
    }
    
    idcu_sandbox_registry_destroy();
    printf("test_sandbox_registry_add_null_param: PASS\n");
}

static void test_sandbox_registry_add_without_init(void) {
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, 0);
    
    int ret = idcu_sandbox_registry_add(&sb);
    if (ret != IDCU_ERR_NOT_INITIALIZED) {
        printf("test_sandbox_registry_add_without_init: FAIL - Add sandbox without init should fail\n");
        idcu_sandbox_destroy(&sb);
        return;
    }
    
    idcu_sandbox_destroy(&sb);
    printf("test_sandbox_registry_add_without_init: PASS\n");
}

static void test_sandbox_registry_get(void) {
    idcu_Sandbox sb1, sb2;
    idcu_sandbox_init(&sb1, 100, IDCU_PERM_SEND);
    idcu_sandbox_init(&sb2, 200, IDCU_PERM_RECV);
    
    idcu_sandbox_registry_init();
    idcu_sandbox_registry_add(&sb1);
    idcu_sandbox_registry_add(&sb2);
    
    idcu_Sandbox* sb = idcu_sandbox_registry_get(100);
    if (sb == NULL) {
        printf("test_sandbox_registry_get: FAIL - Get sandbox 100 should not return NULL\n");
        idcu_sandbox_registry_destroy();
        idcu_sandbox_destroy(&sb1);
        idcu_sandbox_destroy(&sb2);
        return;
    }
    if (sb->module_id != 100) {
        printf("test_sandbox_registry_get: FAIL - Module ID should be 100\n");
        idcu_sandbox_registry_destroy();
        idcu_sandbox_destroy(&sb1);
        idcu_sandbox_destroy(&sb2);
        return;
    }
    
    sb = idcu_sandbox_registry_get(200);
    if (sb == NULL) {
        printf("test_sandbox_registry_get: FAIL - Get sandbox 200 should not return NULL\n");
        idcu_sandbox_registry_destroy();
        idcu_sandbox_destroy(&sb1);
        idcu_sandbox_destroy(&sb2);
        return;
    }
    if (sb->module_id != 200) {
        printf("test_sandbox_registry_get: FAIL - Module ID should be 200\n");
        idcu_sandbox_registry_destroy();
        idcu_sandbox_destroy(&sb1);
        idcu_sandbox_destroy(&sb2);
        return;
    }
    
    sb = idcu_sandbox_registry_get(999);
    if (sb != NULL) {
        printf("test_sandbox_registry_get: FAIL - Get non-existent sandbox should return NULL\n");
        idcu_sandbox_registry_destroy();
        idcu_sandbox_destroy(&sb1);
        idcu_sandbox_destroy(&sb2);
        return;
    }
    
    idcu_sandbox_registry_destroy();
    idcu_sandbox_destroy(&sb1);
    idcu_sandbox_destroy(&sb2);
    printf("test_sandbox_registry_get: PASS\n");
}

static void test_sandbox_registry_get_without_init(void) {
    idcu_Sandbox* sb = idcu_sandbox_registry_get(1);
    if (sb != NULL) {
        printf("test_sandbox_registry_get_without_init: FAIL - Get sandbox without init should return NULL\n");
        return;
    }
    printf("test_sandbox_registry_get_without_init: PASS\n");
}

static void test_sandbox_registry_check_perm(void) {
    idcu_Sandbox sb1, sb2;
    idcu_sandbox_init(&sb1, 100, IDCU_PERM_SEND | IDCU_PERM_RECV);
    idcu_sandbox_init(&sb2, 200, IDCU_PERM_RUN);
    
    idcu_sandbox_registry_init();
    idcu_sandbox_registry_add(&sb1);
    idcu_sandbox_registry_add(&sb2);
    
    int ret = idcu_sandbox_registry_check_perm(100, IDCU_PERM_SEND);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_registry_check_perm: FAIL - Check perm SEND for module 100 should succeed\n");
        idcu_sandbox_registry_destroy();
        idcu_sandbox_destroy(&sb1);
        idcu_sandbox_destroy(&sb2);
        return;
    }
    
    ret = idcu_sandbox_registry_check_perm(100, IDCU_PERM_RUN);
    if (ret != IDCU_ERR_PERM_DENIED) {
        printf("test_sandbox_registry_check_perm: FAIL - Check perm RUN for module 100 should fail\n");
        idcu_sandbox_registry_destroy();
        idcu_sandbox_destroy(&sb1);
        idcu_sandbox_destroy(&sb2);
        return;
    }
    
    ret = idcu_sandbox_registry_check_perm(200, IDCU_PERM_RUN);
    if (ret != IDCU_ERR_OK) {
        printf("test_sandbox_registry_check_perm: FAIL - Check perm RUN for module 200 should succeed\n");
        idcu_sandbox_registry_destroy();
        idcu_sandbox_destroy(&sb1);
        idcu_sandbox_destroy(&sb2);
        return;
    }
    
    ret = idcu_sandbox_registry_check_perm(999, IDCU_PERM_SEND);
    if (ret != IDCU_ERR_NOT_FOUND) {
        printf("test_sandbox_registry_check_perm: FAIL - Check perm for non-existent module should fail\n");
        idcu_sandbox_registry_destroy();
        idcu_sandbox_destroy(&sb1);
        idcu_sandbox_destroy(&sb2);
        return;
    }
    
    idcu_sandbox_registry_destroy();
    idcu_sandbox_destroy(&sb1);
    idcu_sandbox_destroy(&sb2);
    printf("test_sandbox_registry_check_perm: PASS\n");
}

static void test_sandbox_registry_full(void) {
    idcu_Sandbox sandboxes[IDCU_MAX_SANDBOXES];
    for (int i = 0; i < IDCU_MAX_SANDBOXES; i++) {
        idcu_sandbox_init(&sandboxes[i], i + 1, 0);
    }
    
    idcu_sandbox_registry_init();
    
    int all_succeeded = 1;
    for (int i = 0; i < IDCU_MAX_SANDBOXES; i++) {
        int ret = idcu_sandbox_registry_add(&sandboxes[i]);
        if (ret != IDCU_ERR_OK) {
            all_succeeded = 0;
            break;
        }
    }
    if (!all_succeeded) {
        printf("test_sandbox_registry_full: FAIL - All sandboxes should be added successfully\n");
        idcu_sandbox_registry_destroy();
        for (int i = 0; i < IDCU_MAX_SANDBOXES; i++) {
            idcu_sandbox_destroy(&sandboxes[i]);
        }
        return;
    }
    
    idcu_Sandbox extra_sb;
    idcu_sandbox_init(&extra_sb, IDCU_MAX_SANDBOXES + 1, 0);
    int ret = idcu_sandbox_registry_add(&extra_sb);
    if (ret != IDCU_ERR_NO_MEMORY) {
        printf("test_sandbox_registry_full: FAIL - Add extra sandbox should fail with NO_MEMORY\n");
        idcu_sandbox_registry_destroy();
        for (int i = 0; i < IDCU_MAX_SANDBOXES; i++) {
            idcu_sandbox_destroy(&sandboxes[i]);
        }
        idcu_sandbox_destroy(&extra_sb);
        return;
    }
    
    idcu_sandbox_registry_destroy();
    for (int i = 0; i < IDCU_MAX_SANDBOXES; i++) {
        idcu_sandbox_destroy(&sandboxes[i]);
    }
    idcu_sandbox_destroy(&extra_sb);
    printf("test_sandbox_registry_full: PASS\n");
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    printf("=== Sandbox Tests ===\n");
    
    test_sandbox_init_destroy();
    test_sandbox_init_null_param();
    test_sandbox_perm_check();
    test_sandbox_perm_check_null_param();
    test_sandbox_set_perm();
    test_sandbox_set_perm_null_param();
    test_sandbox_add_perm();
    test_sandbox_add_perm_null_param();
    test_sandbox_remove_perm();
    test_sandbox_remove_perm_null_param();
    test_sandbox_get_perm();
    test_sandbox_quota();
    test_sandbox_set_quota_null_param();
    test_sandbox_all_permissions();
    test_sandbox_registry_init_destroy();
    test_sandbox_registry_add_remove();
    test_sandbox_registry_add_null_param();
    test_sandbox_registry_add_without_init();
    test_sandbox_registry_get();
    test_sandbox_registry_get_without_init();
    test_sandbox_registry_check_perm();
    test_sandbox_registry_full();
    
    printf("=== All Tests Completed ===\n");
    
    return 0;
}
