#ifndef IDCU_COMMON_ATOMIC_H
#define IDCU_COMMON_ATOMIC_H

#include "idcu/common/error_code.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // IDCU_COMMON_ATOMIC_H

int32_t idcu_atomic_fetch_add_int32(volatile int32_t* ptr, int32_t value);
int32_t idcu_atomic_fetch_sub_int32(volatile int32_t* ptr, int32_t value);
int32_t idcu_atomic_fetch_or_int32(volatile int32_t* ptr, int32_t value);
int32_t idcu_atomic_fetch_and_int32(volatile int32_t* ptr, int32_t value);
int32_t idcu_atomic_fetch_xor_int32(volatile int32_t* ptr, int32_t value);
int32_t idcu_atomic_exchange_int32(volatile int32_t* ptr, int32_t value);
int idcu_atomic_compare_exchange_int32(volatile int32_t* ptr, int32_t* expected, int32_t desired);
int32_t idcu_atomic_load_int32(volatile int32_t* ptr);
void idcu_atomic_store_int32(volatile int32_t* ptr, int32_t value);
int32_t idcu_atomic_inc_int32(volatile int32_t* ptr);
int32_t idcu_atomic_dec_int32(volatile int32_t* ptr);

int64_t idcu_atomic_fetch_add_int64(volatile int64_t* ptr, int64_t value);
int64_t idcu_atomic_fetch_sub_int64(volatile int64_t* ptr, int64_t value);
int64_t idcu_atomic_exchange_int64(volatile int64_t* ptr, int64_t value);
int idcu_atomic_compare_exchange_int64(volatile int64_t* ptr, int64_t* expected, int64_t desired);
int64_t idcu_atomic_load_int64(volatile int64_t* ptr);
void idcu_atomic_store_int64(volatile int64_t* ptr, int64_t value);

#ifdef __cplusplus
}
#endif // IDCU_COMMON_ATOMIC_H

#endif // IDCU_COMMON_ATOMIC_H
