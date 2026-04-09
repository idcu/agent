#include "idcu/common/atomic.h"

#ifdef _WIN32
#include <windows.h>

int32_t idcu_atomic_load_int32(idcu_AtomicInt32* ptr)
{
    return InterlockedCompareExchange((LONG volatile*)ptr, 0, 0);
}

void idcu_atomic_store_int32(idcu_AtomicInt32* ptr, int32_t value)
{
    InterlockedExchange((LONG volatile*)ptr, value);
}

int32_t idcu_atomic_fetch_add_int32(idcu_AtomicInt32* ptr, int32_t value)
{
    return InterlockedExchangeAdd((LONG volatile*)ptr, value);
}

int32_t idcu_atomic_fetch_sub_int32(idcu_AtomicInt32* ptr, int32_t value)
{
    return InterlockedExchangeAdd((LONG volatile*)ptr, -value);
}

int32_t idcu_atomic_inc_int32(idcu_AtomicInt32* ptr)
{
    return InterlockedIncrement((LONG volatile*)ptr) - 1;
}

int32_t idcu_atomic_dec_int32(idcu_AtomicInt32* ptr)
{
    return InterlockedDecrement((LONG volatile*)ptr) + 1;
}

int idcu_atomic_cas_int32(idcu_AtomicInt32* ptr, int32_t expected, int32_t desired)
{
    return InterlockedCompareExchange((LONG volatile*)ptr, desired, expected) == expected ? 1 : 0;
}

int64_t idcu_atomic_load_int64(idcu_AtomicInt64* ptr)
{
    return InterlockedCompareExchange64((LONGLONG volatile*)ptr, 0, 0);
}

void idcu_atomic_store_int64(idcu_AtomicInt64* ptr, int64_t value)
{
    InterlockedExchange64((LONGLONG volatile*)ptr, value);
}

int64_t idcu_atomic_fetch_add_int64(idcu_AtomicInt64* ptr, int64_t value)
{
    return InterlockedExchangeAdd64((LONGLONG volatile*)ptr, value);
}

int64_t idcu_atomic_fetch_sub_int64(idcu_AtomicInt64* ptr, int64_t value)
{
    return InterlockedExchangeAdd64((LONGLONG volatile*)ptr, -value);
}

int64_t idcu_atomic_inc_int64(idcu_AtomicInt64* ptr)
{
    return InterlockedIncrement64((LONGLONG volatile*)ptr) - 1;
}

int64_t idcu_atomic_dec_int64(idcu_AtomicInt64* ptr)
{
    return InterlockedDecrement64((LONGLONG volatile*)ptr) + 1;
}

int idcu_atomic_cas_int64(idcu_AtomicInt64* ptr, int64_t expected, int64_t desired)
{
    return InterlockedCompareExchange64((LONGLONG volatile*)ptr, desired, expected) == expected ? 1 : 0;
}

#else

#include <stdatomic.h>

int32_t idcu_atomic_load_int32(idcu_AtomicInt32* ptr)
{
    return atomic_load((atomic_int*)ptr);
}

void idcu_atomic_store_int32(idcu_AtomicInt32* ptr, int32_t value)
{
    atomic_store((atomic_int*)ptr, value);
}

int32_t idcu_atomic_fetch_add_int32(idcu_AtomicInt32* ptr, int32_t value)
{
    return atomic_fetch_add((atomic_int*)ptr, value);
}

int32_t idcu_atomic_fetch_sub_int32(idcu_AtomicInt32* ptr, int32_t value)
{
    return atomic_fetch_sub((atomic_int*)ptr, value);
}

int32_t idcu_atomic_inc_int32(idcu_AtomicInt32* ptr)
{
    return atomic_fetch_add((atomic_int*)ptr, 1);
}

int32_t idcu_atomic_dec_int32(idcu_AtomicInt32* ptr)
{
    return atomic_fetch_sub((atomic_int*)ptr, 1);
}

int idcu_atomic_cas_int32(idcu_AtomicInt32* ptr, int32_t expected, int32_t desired)
{
    return atomic_compare_exchange_strong((atomic_int*)ptr, &expected, desired) ? 1 : 0;
}

int64_t idcu_atomic_load_int64(idcu_AtomicInt64* ptr)
{
    return atomic_load((atomic_llong*)ptr);
}

void idcu_atomic_store_int64(idcu_AtomicInt64* ptr, int64_t value)
{
    atomic_store((atomic_llong*)ptr, value);
}

int64_t idcu_atomic_fetch_add_int64(idcu_AtomicInt64* ptr, int64_t value)
{
    return atomic_fetch_add((atomic_llong*)ptr, value);
}

int64_t idcu_atomic_fetch_sub_int64(idcu_AtomicInt64* ptr, int64_t value)
{
    return atomic_fetch_sub((atomic_llong*)ptr, value);
}

int64_t idcu_atomic_inc_int64(idcu_AtomicInt64* ptr)
{
    return atomic_fetch_add((atomic_llong*)ptr, 1);
}

int64_t idcu_atomic_dec_int64(idcu_AtomicInt64* ptr)
{
    return atomic_fetch_sub((atomic_llong*)ptr, 1);
}

int idcu_atomic_cas_int64(idcu_AtomicInt64* ptr, int64_t expected, int64_t desired)
{
    return atomic_compare_exchange_strong((atomic_llong*)ptr, &expected, desired) ? 1 : 0;
}

#endif
