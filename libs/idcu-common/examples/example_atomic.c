#include <stdio.h>
#include <idcu/common/atomic.h>
#include <idcu/common/error_code.h>

int main(void) {
    printf("=== idcu-common 原子操作示例 ===\n\n");

    volatile int32_t counter32 = 0;
    volatile int64_t counter64 = 0;

    printf("1. int32_t 原子操作演示\n");
    printf("-------------------------\n");

    printf("初始�? %d\n", (int)counter32);

    int32_t old_val = idcu_atomic_fetch_add_int32(&counter32, 5);
    printf("fetch_add(5): 旧�?%d, 新�?%d\n", (int)old_val, (int)counter32);

    old_val = idcu_atomic_fetch_sub_int32(&counter32, 3);
    printf("fetch_sub(3): 旧�?%d, 新�?%d\n", (int)old_val, (int)counter32);

    old_val = idcu_atomic_inc_int32(&counter32);
    printf("inc: 旧�?%d, 新�?%d\n", (int)old_val, (int)counter32);

    old_val = idcu_atomic_dec_int32(&counter32);
    printf("dec: 旧�?%d, 新�?%d\n", (int)old_val, (int)counter32);

    int32_t expected = 2;
    int success = idcu_atomic_compare_exchange_int32(&counter32, &expected, 100);
    printf("compare_exchange(2 -> 100): ");
    if (success) {
        printf("成功，新�?%d\n", (int)counter32);
    } else {
        printf("失败，期望�?%d，实际�?%d\n", (int)expected, (int)counter32);
    }

    int32_t val = idcu_atomic_load_int32(&counter32);
    printf("load: %d\n", (int)val);

    idcu_atomic_store_int32(&counter32, 0);
    printf("store(0): 新�?%d\n", (int)counter32);

    printf("\n2. int64_t 原子操作演示\n");
    printf("-------------------------\n");

    printf("初始�? %lld\n", (long long)counter64);

    int64_t old_val64 = idcu_atomic_fetch_add_int64(&counter64, 1000000000LL);
    printf("fetch_add(1e9): 旧�?%lld, 新�?%lld\n", (long long)old_val64, (long long)counter64);

    old_val64 = idcu_atomic_exchange_int64(&counter64, 5000000000LL);
    printf("exchange(5e9): 旧�?%lld, 新�?%lld\n", (long long)old_val64, (long long)counter64);

    val = (int32_t)idcu_atomic_load_int64(&counter64);
    printf("load: %lld\n", (long long)val);

    idcu_atomic_store_int64(&counter64, 0);
    printf("store(0): 新�?%lld\n", (long long)counter64);

    printf("\n=== 示例完成 ===\n");

    return 0;
}
