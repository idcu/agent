#include "idcu/common/atomic.h"
#include <stdio.h>

static void test_atomic_inc_dec_int32(void) {
    volatile int32_t val = 0;
    
    int32_t prev = idcu_atomic_inc_int32(&val);
    if (prev != 0) {
        printf("Test failed: Previous value should be 0\n");
        return;
    }
    if (idcu_atomic_load_int32(&val) != 1) {
        printf("Test failed: Value should be 1 after increment\n");
        return;
    }
    
    prev = idcu_atomic_dec_int32(&val);
    if (prev != 1) {
        printf("Test failed: Previous value should be 1\n");
        return;
    }
    if (idcu_atomic_load_int32(&val) != 0) {
        printf("Test failed: Value should be 0 after decrement\n");
        return;
    }
    
    printf("Test passed: atomic_inc_dec_int32\n");
}

static void test_atomic_fetch_add_int32(void) {
    volatile int32_t val = 5;
    
    int32_t prev = idcu_atomic_fetch_add_int32(&val, 3);
    if (prev != 5) {
        printf("Test failed: Previous value should be 5\n");
        return;
    }
    if (idcu_atomic_load_int32(&val) != 8) {
        printf("Test failed: Value should be 8\n");
        return;
    }
    
    printf("Test passed: atomic_fetch_add_int32\n");
}

static void test_atomic_fetch_sub_int32(void) {
    volatile int32_t val = 10;
    
    int32_t prev = idcu_atomic_fetch_sub_int32(&val, 4);
    if (prev != 10) {
        printf("Test failed: Previous value should be 10\n");
        return;
    }
    if (idcu_atomic_load_int32(&val) != 6) {
        printf("Test failed: Value should be 6\n");
        return;
    }
    
    printf("Test passed: atomic_fetch_sub_int32\n");
}

static void test_atomic_exchange_int32(void) {
    volatile int32_t val = 100;
    
    int32_t prev = idcu_atomic_exchange_int32(&val, 200);
    if (prev != 100) {
        printf("Test failed: Previous value should be 100\n");
        return;
    }
    if (idcu_atomic_load_int32(&val) != 200) {
        printf("Test failed: Value should be 200\n");
        return;
    }
    
    printf("Test passed: atomic_exchange_int32\n");
}

static void test_atomic_compare_exchange_int32(void) {
    volatile int32_t val = 50;
    int32_t expected = 50;
    
    int result = idcu_atomic_compare_exchange_int32(&val, &expected, 100);
    if (result != 1) {
        printf("Test failed: Compare exchange should succeed\n");
        return;
    }
    if (idcu_atomic_load_int32(&val) != 100) {
        printf("Test failed: Value should be 100\n");
        return;
    }
    
    expected = 50;
    result = idcu_atomic_compare_exchange_int32(&val, &expected, 200);
    if (result != 0) {
        printf("Test failed: Compare exchange should fail\n");
        return;
    }
    if (expected != 100) {
        printf("Test failed: Expected should be updated to actual value\n");
        return;
    }
    
    printf("Test passed: atomic_compare_exchange_int32\n");
}

static void test_atomic_load_store_int32(void) {
    volatile int32_t val;
    
    idcu_atomic_store_int32(&val, 42);
    if (idcu_atomic_load_int32(&val) != 42) {
        printf("Test failed: Stored value should be retrievable\n");
        return;
    }
    
    printf("Test passed: atomic_load_store_int32\n");
}

static void test_atomic_bitwise_int32(void) {
    volatile int32_t val = 0b1010; // 10 in decimal
    
    int32_t prev = idcu_atomic_fetch_or_int32(&val, 0b0100); // OR with 4
    if (prev != 0b1010) {
        printf("Test failed: Previous value should be 1010\n");
        return;
    }
    if (idcu_atomic_load_int32(&val) != 0b1110) {
        printf("Test failed: Value should be 1110 after OR\n");
        return;
    }
    
    prev = idcu_atomic_fetch_and_int32(&val, 0b1100); // AND with 12
    if (prev != 0b1110) {
        printf("Test failed: Previous value should be 1110\n");
        return;
    }
    if (idcu_atomic_load_int32(&val) != 0b1100) {
        printf("Test failed: Value should be 1100 after AND\n");
        return;
    }
    
    prev = idcu_atomic_fetch_xor_int32(&val, 0b1010); // XOR with 10
    if (prev != 0b1100) {
        printf("Test failed: Previous value should be 1100\n");
        return;
    }
    if (idcu_atomic_load_int32(&val) != 0b0110) {
        printf("Test failed: Value should be 0110 after XOR\n");
        return;
    }
    
    printf("Test passed: atomic_bitwise_int32\n");
}

static void test_atomic_int64(void) {
    volatile int64_t val = 1000;
    
    int64_t prev = idcu_atomic_fetch_add_int64(&val, 200);
    if (prev != 1000) {
        printf("Test failed: Previous value should be 1000\n");
        return;
    }
    if (idcu_atomic_load_int64(&val) != 1200) {
        printf("Test failed: Value should be 1200 after add\n");
        return;
    }
    
    prev = idcu_atomic_fetch_sub_int64(&val, 300);
    if (prev != 1200) {
        printf("Test failed: Previous value should be 1200\n");
        return;
    }
    if (idcu_atomic_load_int64(&val) != 900) {
        printf("Test failed: Value should be 900 after sub\n");
        return;
    }
    
    prev = idcu_atomic_exchange_int64(&val, 5000);
    if (prev != 900) {
        printf("Test failed: Previous value should be 900\n");
        return;
    }
    if (idcu_atomic_load_int64(&val) != 5000) {
        printf("Test failed: Value should be 5000 after exchange\n");
        return;
    }
    
    int64_t expected = 5000;
    int result = idcu_atomic_compare_exchange_int64(&val, &expected, 10000);
    if (result != 1) {
        printf("Test failed: Compare exchange should succeed\n");
        return;
    }
    if (idcu_atomic_load_int64(&val) != 10000) {
        printf("Test failed: Value should be 10000\n");
        return;
    }
    
    idcu_atomic_store_int64(&val, 7777);
    if (idcu_atomic_load_int64(&val) != 7777) {
        printf("Test failed: Stored value should be retrievable\n");
        return;
    }
    
    printf("Test passed: atomic_int64\n");
}

int main(void) {
    printf("Running Atomic Tests...\n\n");
    
    test_atomic_inc_dec_int32();
    test_atomic_fetch_add_int32();
    test_atomic_fetch_sub_int32();
    test_atomic_exchange_int32();
    test_atomic_compare_exchange_int32();
    test_atomic_load_store_int32();
    test_atomic_bitwise_int32();
    test_atomic_int64();
    
    printf("\nAll tests completed!\n");
    return 0;
}
