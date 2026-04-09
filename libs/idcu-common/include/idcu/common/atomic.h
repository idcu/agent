#ifndef IDCU_COMMON_ATOMIC_H
#define IDCU_COMMON_ATOMIC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef volatile int32_t idcu_AtomicInt32;
typedef volatile int64_t idcu_AtomicInt64;
typedef volatile uint32_t idcu_AtomicUInt32;
typedef volatile uint64_t idcu_AtomicUInt64;

int32_t idcu_atomic_load_int32(idcu_AtomicInt32* ptr);
void    idcu_atomic_store_int32(idcu_AtomicInt32* ptr, int32_t value);
int32_t idcu_atomic_fetch_add_int32(idcu_AtomicInt32* ptr, int32_t value);
int32_t idcu_atomic_fetch_sub_int32(idcu_AtomicInt32* ptr, int32_t value);
int32_t idcu_atomic_inc_int32(idcu_AtomicInt32* ptr);
int32_t idcu_atomic_dec_int32(idcu_AtomicInt32* ptr);
int     idcu_atomic_cas_int32(idcu_AtomicInt32* ptr, int32_t expected, int32_t desired);

int64_t idcu_atomic_load_int64(idcu_AtomicInt64* ptr);
void    idcu_atomic_store_int64(idcu_AtomicInt64* ptr, int64_t value);
int64_t idcu_atomic_fetch_add_int64(idcu_AtomicInt64* ptr, int64_t value);
int64_t idcu_atomic_fetch_sub_int64(idcu_AtomicInt64* ptr, int64_t value);
int64_t idcu_atomic_inc_int64(idcu_AtomicInt64* ptr);
int64_t idcu_atomic_dec_int64(idcu_AtomicInt64* ptr);
int     idcu_atomic_cas_int64(idcu_AtomicInt64* ptr, int64_t expected, int64_t desired);

#ifdef __cplusplus
}
#endif

#endif
