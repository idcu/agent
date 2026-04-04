#include "atomic.h"

#ifdef _WIN32
#include <windows.h>

int32_t idcu_atomic_fetch_add_int32(volatile int32_t* ptr, int32_t value)
{
    return InterlockedExchangeAdd((volatile LONG*)ptr, value);
}

int32_t idcu_atomic_fetch_sub_int32(volatile int32_t* ptr, int32_t value)
{
    return InterlockedExchangeAdd((volatile LONG*)ptr, -value);
}

int32_t idcu_atomic_fetch_or_int32(volatile int32_t* ptr, int32_t value)
{
    int32_t old;
    do {
        old = *ptr;
    } while (InterlockedCompareExchange((volatile LONG*)ptr, old | value, old) != old);
    return old;
}

int32_t idcu_atomic_fetch_and_int32(volatile int32_t* ptr, int32_t value)
{
    int32_t old;
    do {
        old = *ptr;
    } while (InterlockedCompareExchange((volatile LONG*)ptr, old & value, old) != old);
    return old;
}

int32_t idcu_atomic_fetch_xor_int32(volatile int32_t* ptr, int32_t value)
{
    int32_t old;
    do {
        old = *ptr;
    } while (InterlockedCompareExchange((volatile LONG*)ptr, old ^ value, old) != old);
    return old;
}

int32_t idcu_atomic_exchange_int32(volatile int32_t* ptr, int32_t value)
{
    return InterlockedExchange((volatile LONG*)ptr, value);
}

int idcu_atomic_compare_exchange_int32(volatile int32_t* ptr, int32_t* expected, int32_t desired)
{
    int32_t old = *expected;
    int32_t actual = InterlockedCompareExchange((volatile LONG*)ptr, desired, old);
    if (actual != old) {
        *expected = actual;
        return 0;
    }
    return 1;
}

int32_t idcu_atomic_load_int32(volatile int32_t* ptr)
{
    return InterlockedCompareExchange((volatile LONG*)ptr, 0, 0);
}

void idcu_atomic_store_int32(volatile int32_t* ptr, int32_t value)
{
    InterlockedExchange((volatile LONG*)ptr, value);
}

int32_t idcu_atomic_inc_int32(volatile int32_t* ptr)
{
    return InterlockedIncrement((volatile LONG*)ptr);
}

int32_t idcu_atomic_dec_int32(volatile int32_t* ptr)
{
    return InterlockedDecrement((volatile LONG*)ptr);
}

int64_t idcu_atomic_fetch_add_int64(volatile int64_t* ptr, int64_t value)
{
    return InterlockedExchangeAdd64((volatile LONGLONG*)ptr, value);
}

int64_t idcu_atomic_fetch_sub_int64(volatile int64_t* ptr, int64_t value)
{
    return InterlockedExchangeAdd64((volatile LONGLONG*)ptr, -value);
}

int64_t idcu_atomic_exchange_int64(volatile int64_t* ptr, int64_t value)
{
    return InterlockedExchange64((volatile LONGLONG*)ptr, value);
}

int idcu_atomic_compare_exchange_int64(volatile int64_t* ptr, int64_t* expected, int64_t desired)
{
    int64_t old = *expected;
    int64_t actual = InterlockedCompareExchange64((volatile LONGLONG*)ptr, desired, old);
    if (actual != old) {
        *expected = actual;
        return 0;
    }
    return 1;
}

int64_t idcu_atomic_load_int64(volatile int64_t* ptr)
{
    return InterlockedCompareExchange64((volatile LONGLONG*)ptr, 0, 0);
}

void idcu_atomic_store_int64(volatile int64_t* ptr, int64_t value)
{
    InterlockedExchange64((volatile LONGLONG*)ptr, value);
}

#else

int32_t idcu_atomic_fetch_add_int32(volatile int32_t* ptr, int32_t value)
{
    return __sync_fetch_and_add(ptr, value);
}

int32_t idcu_atomic_fetch_sub_int32(volatile int32_t* ptr, int32_t value)
{
    return __sync_fetch_and_sub(ptr, value);
}

int32_t idcu_atomic_fetch_or_int32(volatile int32_t* ptr, int32_t value)
{
    return __sync_fetch_and_or(ptr, value);
}

int32_t idcu_atomic_fetch_and_int32(volatile int32_t* ptr, int32_t value)
{
    return __sync_fetch_and_and(ptr, value);
}

int32_t idcu_atomic_fetch_xor_int32(volatile int32_t* ptr, int32_t value)
{
    return __sync_fetch_and_xor(ptr, value);
}

int32_t idcu_atomic_exchange_int32(volatile int32_t* ptr, int32_t value)
{
    return __sync_lock_test_and_set(ptr, value);
}

int idcu_atomic_compare_exchange_int32(volatile int32_t* ptr, int32_t* expected, int32_t desired)
{
    return __sync_bool_compare_and_swap(ptr, *expected, desired) ? 1 : 0;
}

int32_t idcu_atomic_load_int32(volatile int32_t* ptr)
{
    return __sync_fetch_and_add(ptr, 0);
}

void idcu_atomic_store_int32(volatile int32_t* ptr, int32_t value)
{
    __sync_lock_test_and_set(ptr, value);
}

int32_t idcu_atomic_inc_int32(volatile int32_t* ptr)
{
    return __sync_add_and_fetch(ptr, 1);
}

int32_t idcu_atomic_dec_int32(volatile int32_t* ptr)
{
    return __sync_sub_and_fetch(ptr, 1);
}

int64_t idcu_atomic_fetch_add_int64(volatile int64_t* ptr, int64_t value)
{
    return __sync_fetch_and_add(ptr, value);
}

int64_t idcu_atomic_fetch_sub_int64(volatile int64_t* ptr, int64_t value)
{
    return __sync_fetch_and_sub(ptr, value);
}

int64_t idcu_atomic_exchange_int64(volatile int64_t* ptr, int64_t value)
{
    return __sync_lock_test_and_set(ptr, value);
}

int idcu_atomic_compare_exchange_int64(volatile int64_t* ptr, int64_t* expected, int64_t desired)
{
    return __sync_bool_compare_and_swap(ptr, *expected, desired) ? 1 : 0;
}

int64_t idcu_atomic_load_int64(volatile int64_t* ptr)
{
    return __sync_fetch_and_add(ptr, 0);
}

void idcu_atomic_store_int64(volatile int64_t* ptr, int64_t value)
{
    __sync_lock_test_and_set(ptr, value);
}

#endif
