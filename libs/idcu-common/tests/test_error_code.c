#include "idcu/common/error_code.h"
#include <stdio.h>
#include <string.h>

static void test_err_to_str(void) {
    const char* str;
    
    str = idcu_err_to_str(IDCU_ERR_OK);
    if (str == NULL) {
        printf("Test failed: Error string should not be NULL for OK\n");
        return;
    }
    if (strlen(str) <= 0) {
        printf("Test failed: Error string should be non-empty\n");
        return;
    }
    
    str = idcu_err_to_str(IDCU_ERR_INVALID_PARAM);
    if (str == NULL) {
        printf("Test failed: Error string should not be NULL\n");
        return;
    }
    if (strlen(str) <= 0) {
        printf("Test failed: Error string should be non-empty\n");
        return;
    }
    
    str = idcu_err_to_str(IDCU_ERR_NO_MEMORY);
    if (str == NULL) {
        printf("Test failed: Error string should not be NULL\n");
        return;
    }
    
    str = idcu_err_to_str(-9999); // Unknown error code
    if (str == NULL) {
        printf("Test failed: Error string should not be NULL for unknown code\n");
        return;
    }
    
    printf("Test passed: err_to_str\n");
}

static void test_err_is_ok(void) {
    if (idcu_err_is_ok(IDCU_ERR_OK) == 0) {
        printf("Test failed: OK should be is_ok\n");
        return;
    }
    if (idcu_err_is_ok(IDCU_ERR_SUCCESS) == 0) {
        printf("Test failed: SUCCESS should be is_ok\n");
        return;
    }
    
    if (idcu_err_is_ok(IDCU_ERR_INVALID_PARAM) != 0) {
        printf("Test failed: Invalid param should not be is_ok\n");
        return;
    }
    if (idcu_err_is_ok(IDCU_ERR_NO_MEMORY) != 0) {
        printf("Test failed: No memory should not be is_ok\n");
        return;
    }
    
    printf("Test passed: err_is_ok\n");
}

static void test_error_code_aliases(void) {
    if (IDCU_ERR_OK != IDCU_SUCCESS) {
        printf("Test failed: OK and SUCCESS should be equal\n");
        return;
    }
    if (IDCU_ERR_INVALID_PARAM != IDCU_ERROR_INVALID_PARAM) {
        printf("Test failed: Invalid param aliases should match\n");
        return;
    }
    if (IDCU_ERR_MEMORY != IDCU_ERROR_MEMORY) {
        printf("Test failed: Memory error aliases should match\n");
        return;
    }
    
    printf("Test passed: error_code_aliases\n");
}

int main(void) {
    printf("Running Error Code Tests...\n\n");
    
    test_err_to_str();
    test_err_is_ok();
    test_error_code_aliases();
    
    printf("\nAll tests completed!\n");
    return 0;
}
